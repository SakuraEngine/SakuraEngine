#include "render-scene.h"
#include "gamert.h"
#include "utils/hash.h"

static dual_type_index_t gfx_shader_set_type;
static dual_type_index_t processor_shader_set_type;
static dual_type_index_t gfx_material_type;
static dual_type_index_t processor_material_type;

typedef struct gfx_shader_set_query_exist_t {
    gfx_shader_set_id_t exist;
    size_t hash;
    dual_storage_t* storage;
} gfx_shader_set_query_exist_t;
void gfx_shader_set_query_exist_callback(void* u, dual_chunk_view_t* view)
{
    gfx_shader_set_query_exist_t* query = (gfx_shader_set_query_exist_t*)u;
    gfx_shader_set_t* sets = (gfx_shader_set_t*)dualV_get_owned_rw(view, gfx_shader_set_type);
    const dual_entity_t* ents = dualV_get_entities(view);
    for(uint32_t i = 0; i < view->count; i++)
    {
        const size_t hash = skr_hash(&sets[i], sizeof(gfx_shader_set_t), (size_t)query->storage);
        if(query->hash == hash)
        {
            query->exist = ents[i];
            return;
        }
    }
}
typedef struct gfx_shader_set_construct_t {
    const gfx_shader_set_t* value;
    gfx_shader_set_id_t ret;
} gfx_shader_set_construct_t;
void gfx_shader_set_construct_callback(void* u, dual_chunk_view_t* view)
{
    gfx_shader_set_construct_t* construct = u;
    *(gfx_shader_set_t*)dualV_get_owned_rw(view, gfx_shader_set_type) = *construct->value;
    construct->ret = dualV_get_entities(view)[0];
}
gfx_shader_set_id_t ecsr_register_gfx_shader_set(const gfx_shader_set_t* set) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_filter_t filter = {
        .all.data = &gfx_shader_set_type,
        .all.length = 1
    };
    dual_meta_filter_t meta = {0};
    gfx_shader_set_query_exist_t query = {
        .storage = storage,
        .exist = UINT32_MAX,
        .hash = skr_hash(set, sizeof(gfx_shader_set_t), (size_t)storage)
    };
    dualS_query(storage, &filter, &meta, &gfx_shader_set_query_exist_callback, &query);
    if(query.exist != UINT32_MAX) 
        return query.exist;
    {
        dual_entity_type_t mat_type = {
            .type.data = &gfx_shader_set_type,
            .type.length = 1,
            .meta.data = NULL,
            .meta.length = 0
        };
        gfx_shader_set_construct_t construct = {
            .ret = UINT32_MAX,
            .value = set
        };
        dualS_allocate_type(storage, &mat_type, 1, &gfx_shader_set_construct_callback, &construct);
        return construct.ret;
    }
}
gfx_shader_set_t* ecsr_query_gfx_shader_set(gfx_shader_set_id_t id) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_chunk_view_t view = {0};
    dualS_access(storage, id, &view);
    return (gfx_shader_set_t*)dualV_get_owned_rw(&view, gfx_shader_set_type);
}

gfx_material_id_t ecsr_register_gfx_material(const gfx_material_t* mat) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    (void)storage;
    return 0;
}

void ecsr_register_types()
{
    {
        dual_type_description_t desc = {
            .name = "gfx_shader_set",
            .size = sizeof(gfx_shader_set_t),
            .guid = gfx_shader_set_guid,
            .alignment = _Alignof(gfx_shader_set_t)
        };
        gfx_shader_set_type = dualT_register_type(&desc);
    }
    {
        dual_type_description_t desc = {
            .name = "processor_shader_set",
            .size = sizeof(processor_shader_set_t),
            .guid = processor_shader_set_guid,
            .alignment = _Alignof(processor_shader_set_t)
        };
        processor_shader_set_type = dualT_register_type(&desc);
    }
    {
        dual_type_description_t desc = {
            .name = "gfx_material",
            .size = sizeof(gfx_material_t),
            .guid = gfx_material_guid,
            .alignment = _Alignof(gfx_material_t)
        };
        gfx_material_type = dualT_register_type(&desc);
    }
    {
        dual_type_description_t desc = {
            .name = "processor_material",
            .size = sizeof(processor_material_t),
            .guid = cmpt_material_guid,
            .alignment = _Alignof(processor_material_t)
        };
        processor_material_type = dualT_register_type(&desc);
    }
}