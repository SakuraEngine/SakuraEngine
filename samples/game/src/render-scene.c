#include "render-scene.h"
#include "gamert.h"
#include "utils/hash.h"
#include "ecs/dualX.h"

static dual_type_index_t gfx_shader_set_type;
static dual_type_index_t processor_shader_set_type;
static dual_type_index_t gfx_material_type;
static dual_type_index_t processor_material_type;

void __gfx_shader_set_construct_callback(void* u, dual_chunk_view_t* view)
{
    const gfx_shader_set_t* set = u;
    *(gfx_shader_set_t*)dualV_get_owned_rw(view, gfx_shader_set_type) = *set;
}
gfx_shader_set_id_t ecsr_register_gfx_shader_set(const gfx_shader_set_t* set) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_entity_type_t alloc_type = {
        .type.data = &gfx_shader_set_type,
        .type.length = 1,
    };
    dual_type_set_t key_set = {
        .data = &gfx_shader_set_type,
        .length = 1
    };
    return dualX_hashset_insert(storage, &key_set, &alloc_type, &__gfx_shader_set_construct_callback, (void*)set);
}

gfx_shader_set_t* ecsr_query_gfx_shader_set(gfx_shader_set_id_t id) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_chunk_view_t view = {0};
    dualS_access(storage, id, &view);
    return (gfx_shader_set_t*)dualV_get_owned_rw(&view, gfx_shader_set_type);
}

bool ecsr_unregister_gfx_shader_set(gfx_shader_set_id_t ent) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_chunk_view_t cv = {0};
    dualS_access(storage, ent, &cv);
    dualS_destroy(storage, &cv);
    return true;
}

void __gfx_material_construct_callback(void* u, dual_chunk_view_t* view)
{
    const gfx_material_t* mat = u;
    *(gfx_material_t*)dualV_get_owned_rw(view, gfx_material_type) = *mat;
}
gfx_material_id_t ecsr_register_gfx_material(const gfx_material_t* mat) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_entity_type_t alloc_type = {
        .type.data = &gfx_material_type,
        .type.length = 1,
        .meta.data = &mat->m_gfx,
        .meta.length = 1
    };
    dual_type_set_t key_set = {
        .data = &gfx_material_type,
        .length = 1
    };
    return dualX_hashset_insert(storage, &key_set, &alloc_type, &__gfx_material_construct_callback, (void*)mat);
}

gfx_material_t* ecsr_query_gfx_material(gfx_material_id_t id) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_chunk_view_t view = {0};
    dualS_access(storage, id, &view);
    return (gfx_material_t*)dualV_get_owned_rw(&view, gfx_material_type);
}

bool ecsr_unregister_gfx_material(gfx_material_id_t ent) SKR_NOEXCEPT
{
    dual_storage_t* storage = gamert_get_ecs_world();
    dual_chunk_view_t cv = {0};
    dualS_access(storage, ent, &cv);
    dualS_destroy(storage, &cv);
    return true;
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