#include "render-scene.h"
#include "gamert.h"
#include "utils/hash.h"
#include "ecs/dualX.h"

dual_type_index_t gfx_shader_set_type = UINT32_MAX;
dual_type_index_t processor_shader_set_type = UINT32_MAX;
dual_type_index_t gfx_material_type = UINT32_MAX;
dual_type_index_t gfx_root_sig_type = UINT32_MAX;
dual_type_index_t processor_material_type = UINT32_MAX;

dual_type_index_t gfx_material_inst_type = UINT32_MAX;
// index/vertex buffers
dual_type_index_t index_buffer_type = UINT32_MAX;
dual_type_index_t vertex_buffer_type = UINT32_MAX;

static struct
{
    dual_storage_t* storage;
} ecsRenderer;
SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();

void __gfx_shader_set_construct_callback(void* u, dual_chunk_view_t* view)
{
    const gfx_shader_set_t* set = u;
    *(gfx_shader_set_t*)dualV_get_owned_rw(view, gfx_shader_set_type) = *set;
}
gfx_shader_set_id_t ecsr_register_gfx_shader_set(const gfx_shader_set_t* set) SKR_NOEXCEPT
{
    dual_storage_t* storage = skr_runtime_get_dual_storage();
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
    dual_storage_t* storage = skr_runtime_get_dual_storage();
    dual_chunk_view_t view = { 0 };
    dualS_access(storage, id, &view);
    return (gfx_shader_set_t*)dualV_get_owned_rw(&view, gfx_shader_set_type);
}

bool ecsr_unregister_gfx_shader_set(gfx_shader_set_id_t id) SKR_NOEXCEPT
{
    dual_storage_t* storage = skr_runtime_get_dual_storage();
    gfx_shader_set_t* set = ecsr_query_gfx_shader_set(id);
    if (set->vs) cgpu_free_shader_library(set->vs);
    if (set->hs) cgpu_free_shader_library(set->hs);
    if (set->ds) cgpu_free_shader_library(set->ds);
    if (set->gs) cgpu_free_shader_library(set->gs);
    if (set->ps) cgpu_free_shader_library(set->ps);
    dual_chunk_view_t cv = { 0 };
    dualS_access(storage, id, &cv);
    dualS_destroy(storage, &cv);
    return true;
}

const char8_t* entry = "main";
void __gfx_material_construct_callback(void* u, dual_chunk_view_t* view)
{
    const gfx_material_t* mat = u;
    gfx_material_t* new_mat = (gfx_material_t*)dualV_get_owned_rw(view, gfx_material_type);
    *new_mat = *mat;
    CGPURootSignatureId* pRS = dualV_get_owned_rw(view, gfx_root_sig_type);
    gfx_shader_set_t* set = ecsr_query_gfx_shader_set(mat->m_gfx);
    uint32_t sindex = 0;
    CGPUPipelineShaderDescriptor gshaders[5];
    if (set->vs)
    {
        gshaders[sindex].entry = entry;
        gshaders[sindex].stage = CGPU_SHADER_STAGE_VERT;
        gshaders[sindex++].library = set->vs;
    }
    if (set->hs)
    {
        gshaders[sindex].entry = entry;
        gshaders[sindex].stage = CGPU_SHADER_STAGE_HULL;
        gshaders[sindex++].library = set->hs;
    }
    if (set->ds)
    {
        gshaders[sindex].entry = entry;
        gshaders[sindex].stage = CGPU_SHADER_STAGE_DOMAIN;
        gshaders[sindex++].library = set->ds;
    }
    if (set->gs)
    {
        gshaders[sindex].entry = entry;
        gshaders[sindex].stage = CGPU_SHADER_STAGE_GEOM;
        gshaders[sindex++].library = set->gs;
    }
    if (set->ps)
    {
        gshaders[sindex].entry = entry;
        gshaders[sindex].stage = CGPU_SHADER_STAGE_FRAG;
        gshaders[sindex++].library = set->ps;
    }
    CGPURootSignatureDescriptor rs_desc = {
        .shader_count = sindex,
        .shaders = &gshaders[0],
        .pool = NULL,
        .push_constant_names = mat->push_constant_names,
        .push_constant_count = mat->push_constant_count,
        .static_sampler_count = mat->static_sampler_count,
        .static_samplers = mat->static_samplers,
        .static_sampler_names = mat->static_sampler_names
    };
    *pRS = cgpu_create_root_signature(mat->device, &rs_desc);
}
gfx_material_id_t ecsr_register_gfx_material(const gfx_material_t* mat) SKR_NOEXCEPT
{
    dual_storage_t* storage = skr_runtime_get_dual_storage();
    dual_type_index_t types[] = { gfx_material_type, gfx_root_sig_type };
    dual_entity_type_t alloc_type = {
        .type.data = types,
        .type.length = 2,
        //.meta.data = &mat->m_gfx,
        //.meta.length = 1
    };
    dual_type_set_t key_set = {
        .data = &gfx_material_type,
        .length = 1
    };
    gfx_material_id_t material = dualX_hashset_insert(storage, &key_set, &alloc_type, &__gfx_material_construct_callback, (void*)mat);
    // initialize all parameter types
    ecsr_query_material_parameter_type(material, NULL);
    return material;
}

gfx_material_t* ecsr_query_gfx_material(gfx_material_id_t id) SKR_NOEXCEPT
{
    dual_storage_t* storage = skr_runtime_get_dual_storage();
    dual_chunk_view_t view = { 0 };
    dualS_access(storage, id, &view);
    return (gfx_material_t*)dualV_get_owned_rw(&view, gfx_material_type);
}

bool ecsr_unregister_gfx_material(gfx_material_id_t ent) SKR_NOEXCEPT
{
    dual_storage_t* storage = skr_runtime_get_dual_storage();
    dual_chunk_view_t cv = { 0 };
    dualS_access(storage, ent, &cv);
    CGPURootSignatureId* pRS = dualV_get_owned_rw(&cv, gfx_root_sig_type);
    if (pRS) cgpu_free_root_signature(*pRS);
    dualS_destroy(storage, &cv);
    return true;
}

bool ecsr_finalize()
{
    // destroy shaders

    // destroy rootsignatures

    return true;
}

void ecsr_register_types()
{
    // program objects
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
            .name = "gfx_root_signature",
            .size = sizeof(CGPURootSignatureId),
            .guid = { 0xc3d2cf26, 0x5a7e, 0x4bcc, { 0xb5, 0x1e, 0xae, 0xc5, 0x1c, 0xd0, 0x4c, 0x49 } },
            .alignment = _Alignof(CGPURootSignatureId)
        };
        gfx_root_sig_type = dualT_register_type(&desc);
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
    // objects on scene prims
    {
        dual_type_description_t desc = {
            .name = "material_inst",
            .size = sizeof(gfx_material_inst_t),
            .guid = gfx_material_inst_guid,
            .alignment = _Alignof(gfx_material_inst_t)
        };
        gfx_material_inst_type = dualT_register_type(&desc);
    }
    {
        dual_type_description_t desc = {
            .name = "index_buffer",
            .size = sizeof(CGPUBufferId),
            .guid = { 0x925a3900, 0xed0d, 0x4aa9, { 0x8b, 0xbb, 0x05, 0x0a, 0xca, 0x80, 0xfb, 0x4a } },
            .alignment = _Alignof(CGPUBufferId)
        };
        index_buffer_type = dualT_register_type(&desc);
    }
}