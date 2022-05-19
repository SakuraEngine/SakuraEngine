#include "render-scene.h"
#include "gamert.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/unordered_map.h>
#include <EASTL/vector_map.h>
#include <EASTL/string.h>
#include "ecs/callback.hpp"

static struct 
{
    eastl::unordered_map<gfx_material_id_t, 
        eastl::vector_map<eastl::string, dual_type_index_t>> bindingTypeDB;
    // TODO: add a real pipeline pool with hashset
    eastl::unordered_map<gfx_material_id_t, CGPURenderPipelineId> pipelinePool;
} matDB;

dual_type_index_t ecsr_query_material_parameter_type(gfx_material_id_t mat_id, const char8_t* name)
{
    auto&& db = matDB.bindingTypeDB[mat_id];
    if (name != nullptr)
    {
        auto type = db.find(name);
        if(type != db.end())
        {
            return type->second;
        }
        return UINT32_MAX;
    }
    // allocate all
    dual_storage_t* storage = gamert_get_ecs_world();
    auto view = make_zeroed<dual_chunk_view_t>();
    dualS_access(storage, mat_id, &view);
    CGPURootSignatureId RS = ((CGPURootSignatureId*)dualV_get_owned_rw(&view, gfx_root_sig_type))[0];
    // make or check the register table of this material
    for (uint32_t i = 0; i < RS->table_count; i++)
    {
        for (uint32_t j = 0; j < RS->tables[i].resources_count; j++)
        {
            CGPUShaderResource* res = &RS->tables[i].resources[j];
            if (ecsr_query_material_parameter_type(mat_id, res->name) == UINT32_MAX)
            {
                auto type_desc = make_zeroed<dual_type_description_t>();
                eastl::string unique_string = eastl::to_string(mat_id).append(res->name);
                type_desc.name = unique_string.c_str();
                type_desc.size = res->size;
                dual_make_guid(&type_desc.guid);
                db[res->name] = dualT_register_type(&type_desc);
            }
        }
    }
    for (uint32_t i = 0; i < RS->push_constant_count; i++)
    {
        CGPUShaderResource* res = &RS->push_constants[i];
        if (ecsr_query_material_parameter_type(mat_id, res->name) == UINT32_MAX)
        {
            auto type_desc = make_zeroed<dual_type_description_t>();
            eastl::string unique_string = eastl::to_string(mat_id).append("-").append(res->name);
            type_desc.name = unique_string.c_str();
            type_desc.size = res->size;
            dual_make_guid(&type_desc.guid);
            db[res->name] = dualT_register_type(&type_desc);
        }
    }
    return 0;
}

bool ecsr_renderable_primitive_type(const skr_scene_primitive_desc_t* desc, 
dual_type_index_t* ctypes, uint32_t* ctype_count, dual_entity_t* emetas, uint32_t* meta_count) SKR_NOEXCEPT
{
    *meta_count = 0;
    *ctype_count = 0;
    if (desc->material)
    {
        *meta_count = 1;
        if(emetas != NULL) emetas[0] = desc->material;
        *ctype_count += 4;
        if(ctypes)
        {
            ctypes[0] = transform_type;
            ctypes[1] = gfx_material_inst_type;
            ctypes[2] = index_buffer_type;
            ctypes[3] = vertex_buffer_type;
        }
        const uint32_t bcount = matDB.bindingTypeDB[desc->material].size();
        if (ctypes)
        {
            for(uint32_t i = 0; i < bcount; i++)
            {
                ctypes[*ctype_count + i] = matDB.bindingTypeDB[desc->material].at(i).second;
            }
        }
        *ctype_count += bcount;
        return true;
    }
    return false;
}


// TODO: REFACTOR THIS
const ECGPUFormat gbuffer_formats[] = {
    CGPU_FORMAT_R8G8B8A8_UNORM, CGPU_FORMAT_R16G16B16A16_SNORM
};
const ECGPUFormat gbuffer_depth_format = CGPU_FORMAT_D32_SFLOAT;
CGPURenderPipelineId ecsr_create_gbuffer_render_pipeline(
    CGPUDeviceId device, CGPURootSignatureId root_sig,
    const CGPUPipelineShaderDescriptor* vs,
    const CGPUPipelineShaderDescriptor* ps)
{
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, CGPU_FORMAT_R8G8B8A8_SNORM, 2, 0, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "TANGENT", 1, CGPU_FORMAT_R8G8B8A8_SNORM, 3, 0, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "MODEL", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 4, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 5;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = vs;
    rp_desc.fragment_shader = ps;
    rp_desc.render_target_count = sizeof(gbuffer_formats) / sizeof(ECGPUFormat);
    rp_desc.color_formats = gbuffer_formats;
    rp_desc.depth_stencil_format = gbuffer_depth_format;
    CGPURasterizerStateDescriptor raster_desc = {};
    raster_desc.cull_mode = CGPU_CULL_MODE_BACK;
    raster_desc.depth_bias = 0;
    raster_desc.fill_mode = CGPU_FILL_MODE_SOLID;
    raster_desc.front_face = CGPU_FRONT_FACE_CCW;
    rp_desc.rasterizer_state = &raster_desc;
    CGPUDepthStateDescriptor ds_desc = {};
    ds_desc.depth_func = CGPU_CMP_LEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;
    rp_desc.depth_state = &ds_desc;
    return cgpu_create_render_pipeline(device, &rp_desc);
}

void ecsr_draw_scene(struct skr_render_graph_t* graph) SKR_NOEXCEPT
{
    const dual_type_index_t draw_filter_all[] = { 
        transform_type, gfx_material_inst_type, index_buffer_type, vertex_buffer_type 
    };
    auto rg = (skr::render_graph::RenderGraph*)graph;
    // TODO: Culling
    // iterate materials
    dual_storage_t* storage = gamert_get_ecs_world();
    auto filter = make_zeroed<dual_filter_t>();
    filter.all.data = &gfx_material_type;
    filter.all.length = 1;
    auto meta = make_zeroed<dual_meta_filter_t>();
    auto callback = [&](dual_chunk_view_t* inView) {
        auto mats = (gfx_material_t*)dualV_get_owned_ro(inView, gfx_material_type);
        auto ents = dualV_get_entities(inView);
        for(uint32_t i = 0; i < inView->count; i++)
        {
            auto matId = ents[i];
            auto& mat = mats[i];
            auto draw_filter = make_zeroed<dual_filter_t>();
            draw_filter.all.data = draw_filter_all;
            draw_filter.all.length = sizeof(draw_filter_all) / sizeof(dual_type_index_t);
            auto draw_meta = make_zeroed<dual_meta_filter_t>();
            draw_meta.all_meta.data = &matId;
            draw_meta.all_meta.length = 1;
            // this is a batch
            auto draw_callback = [&](dual_chunk_view_t* inView) {
                // find proper render pipeline
                auto iter = matDB.pipelinePool.find(matId);
                if (iter == matDB.pipelinePool.end()) // create a new one
                {
                    auto view = make_zeroed<dual_chunk_view_t>();
                    dualS_access(storage, matId, &view);
                    CGPURootSignatureId RS = ((CGPURootSignatureId*)dualV_get_owned_rw(&view, gfx_root_sig_type))[0];
                    auto shader_set = ecsr_query_gfx_shader_set(mat.m_gfx);
                    CGPUPipelineShaderDescriptor vs = {};
                    vs.library = shader_set->vs;
                    vs.stage = CGPU_SHADER_STAGE_VERT;
                    vs.entry = "main";
                    CGPUPipelineShaderDescriptor ps = {};
                    ps.library = shader_set->ps;
                    ps.stage = CGPU_SHADER_STAGE_FRAG;
                    ps.entry = "main";
                    matDB.pipelinePool[matId] = 
                    ecsr_create_gbuffer_render_pipeline(mat.device, RS, &vs, &ps);
                }
                auto&& bindingType = matDB.bindingTypeDB[matId];
            };
            dualS_query(storage, &draw_filter, &draw_meta, DUAL_LAMBDA(draw_callback));
        }
    };
    dualS_query(storage, &filter, &meta, DUAL_LAMBDA(callback));
    
}

// END TODO: REFACTOR THIS