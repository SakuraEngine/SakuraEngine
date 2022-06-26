#include "render-scene.h"
#include "cgpu/api.h"
#include "cgpu/flags.h"
#include "gamert.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/unordered_map.h>
#include <EASTL/vector_map.h>
#include <EASTL/string.h>
#include "ecs/callback.hpp"
#include "ecs/array.hpp"
#include "math/vectormath.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"

static struct 
{
    struct ECSRShaderResource
    {
        ECGPUResourceType type;
        uint32_t set;
        uint32_t binding;
        dual_type_index_t type_index;
    };
    eastl::unordered_map<gfx_material_id_t, 
        eastl::vector_map<eastl::string, ECSRShaderResource>> bindingTypeDB;
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
            return type->second.type_index;
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
                type_desc.alignment = alignof(char8_t);
                dual_make_guid(&type_desc.guid);
                db[res->name].type_index = dualT_register_type(&type_desc);
                db[res->name].type = res->type;
                db[res->name].set = res->set;
                db[res->name].binding = res->binding;
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
            type_desc.alignment = alignof(char8_t);
            dual_make_guid(&type_desc.guid);
            db[res->name].type_index = dualT_register_type(&type_desc);
            db[res->name].type = CGPU_RESOURCE_TYPE_PUSH_CONSTANT;
            db[res->name].set = res->set;
            db[res->name].binding = res->binding;
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
            ctypes[3] = dual_id_of<ecsr_vertex_buffer_t>::get();
        }
        const uint32_t bcount = static_cast<uint32_t>(matDB.bindingTypeDB[desc->material].size());
        if (ctypes)
        {
            for(uint32_t i = 0; i < bcount; i++)
            {
                ctypes[*ctype_count + i] = matDB.bindingTypeDB[desc->material].at(i).second.type_index;
            }
        }
        *ctype_count += bcount;
        return true;
    }
    return false;
}


// TODO: REFACTOR THIS
const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT;
CGPURenderPipelineId ecsr_create_gbuffer_render_pipeline(
    CGPUDeviceId device, CGPURootSignatureId root_sig,
    const CGPUPipelineShaderDescriptor* vs,
    const CGPUPipelineShaderDescriptor* ps)
{
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, CGPU_FORMAT_R8G8B8A8_SNORM, 2, 0, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 3;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = vs;
    rp_desc.fragment_shader = ps;
    rp_desc.render_target_count = 1;
    const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
    rp_desc.color_formats = &fmt;
    rp_desc.depth_stencil_format = depth_format;
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
#include <iostream>

void ecsr_draw_scene(struct skr_render_graph_t* graph, ECGPUShadingRate shading_rate) SKR_NOEXCEPT
{
    auto renderGraph = (skr::render_graph::RenderGraph*)graph;
    // TODO: Culling
    // iterate materials
    dual_storage_t* storage = gamert_get_ecs_world();
    {
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            builder.set_name("depth")
            .extent(900, 900)
            .format(depth_format)
            .owns_memory()
            .allow_depth_stencil();
        });
    }
    auto filter = make_zeroed<dual_filter_t>();
    filter.all.data = &gfx_material_type;
    filter.all.length = 1;
    auto meta = make_zeroed<dual_meta_filter_t>();
    // foreach materials
    auto callback = [&](dual_chunk_view_t* inView) {
        auto mats = (gfx_material_t*)dualV_get_owned_ro(inView, gfx_material_type);
        auto ents = dualV_get_entities(inView);
        // per material
        for(uint32_t i = 0; i < inView->count; i++)
        {
            auto matId = ents[i];
            auto& mat = mats[i];
            {
                const dual_type_index_t draw_filter_all[] = { 
                    transform_type, gfx_material_inst_type, index_buffer_type, 
                    dual_id_of<ecsr_vertex_buffer_t>::get()
                };
                auto draw_filter = make_zeroed<dual_filter_t>();
                draw_filter.all.data = draw_filter_all;
                draw_filter.all.length = sizeof(draw_filter_all) / sizeof(dual_type_index_t);
                auto draw_meta = make_zeroed<dual_meta_filter_t>();
                draw_meta.all_meta.data = &matId;
                draw_meta.all_meta.length = 1;
                
                auto ppl_callback = [&](dual_chunk_view_t* inView) {
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
                        matDB.pipelinePool[matId] = ecsr_create_gbuffer_render_pipeline(mat.device, RS, &vs, &ps);
                    }
                };
                dualS_query(storage, &draw_filter, &draw_meta, DUAL_LAMBDA(ppl_callback));
            }

            const auto gbuffer_pipeline = matDB.pipelinePool[matId];
            renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                const auto out_color = renderGraph->get_texture("backbuffer");
                const auto depth_buffer = renderGraph->get_texture("depth");
                builder.set_name("forward_pass")
                .set_pipeline(gbuffer_pipeline)
                .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                .set_depth_stencil(depth_buffer);
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                // this is a batch
                auto draw_callback = [&](dual_chunk_view_t* inView) {
                    auto&& bindingType = matDB.bindingTypeDB[matId];
                    // TODO: update binding data if is not push_const

                    // draw
                    using vertex_buffers_t = dual::array_component_T<ecsr_vertex_buffer_t, 8>;
                    auto index_buffers = (CGPUBufferId*)dualV_get_owned_ro(inView, index_buffer_type);
                    auto vertex_buffers = (vertex_buffers_t*)dualV_get_owned_ro(inView, dual_id_of<ecsr_vertex_buffer_t>::get());
                    auto transforms = (transform_t*)dualV_get_owned_ro(inView, transform_type);
                    for(uint32_t j = 0; j < inView->count; j++)
                    {
                        struct PushConstants {
                            skr::math::float4x4 world;
                            skr::math::float4x4 view_proj;
                        } data;
                        auto world = skr::math::make_transform(
                            transforms[j].location,
                            transforms[j].scale,
                            skr::math::Quaternion::identity()
                        );
                        auto view = skr::math::look_at_matrix(
                            { 0.f, 0.f, 12.5f } /*eye*/, 
                            { 0.f, 0.f, 0.f } /*at*/);
                        auto proj = skr::math::perspective_fov(
                            3.1415926f / 2.f,
                            (float)900 / (float)900,
                            1.f, 1000.f);
                        data.world = world;
                        data.view_proj = skr::math::multiply(view, proj);
                        const auto vertex_buffer = vertex_buffers[j];
                        const auto index_buffer = index_buffers[j];

                        cgpu_render_encoder_set_viewport(stack.encoder,
                        0.0f, 0.0f,
                        (float)900, (float)900,
                        0.f, 1.f);
                        cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, 900, 900);
                        CGPUBufferId vertex_buffers[3] = {
                            vertex_buffer[0].buffer, vertex_buffer[1].buffer, vertex_buffer[2].buffer
                        };
                        const uint32_t strides[3] = {
                            vertex_buffer[0].stride, vertex_buffer[1].stride, vertex_buffer[2].stride
                        };
                        const uint32_t offsets[3] = {
                            vertex_buffer[0].offset, vertex_buffer[1].offset, vertex_buffer[2].offset
                        };
                        cgpu_render_encoder_bind_index_buffer(stack.encoder, index_buffer, sizeof(uint32_t), 0);
                        cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 3, vertex_buffers, strides, offsets);
                        cgpu_render_encoder_push_constants(stack.encoder, gbuffer_pipeline->root_signature, "push_constants", &data);
                        cgpu_render_encoder_set_shading_rate(stack.encoder, CGPU_SHADING_RATE_QUARTER, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH);
                        cgpu_render_encoder_draw_indexed_instanced(stack.encoder, 36, 0, 1, 0, 0);
                        cgpu_render_encoder_set_shading_rate(stack.encoder, CGPU_SHADING_RATE_FULL, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH);
                    }
                };
                const dual_type_index_t draw_filter_all[] = { 
                    transform_type, gfx_material_inst_type, index_buffer_type, 
                    dual_id_of<ecsr_vertex_buffer_t>::get()
                };
                auto draw_filter = make_zeroed<dual_filter_t>();
                draw_filter.all.data = draw_filter_all;
                draw_filter.all.length = sizeof(draw_filter_all) / sizeof(dual_type_index_t);
                auto draw_meta = make_zeroed<dual_meta_filter_t>();
                draw_meta.all_meta.data = &matId;
                draw_meta.all_meta.length = 1;
                dualS_query(storage, &draw_filter, &draw_meta, DUAL_LAMBDA(draw_callback));
            });
        }
    };
    dualS_query(storage, &filter, &meta, DUAL_LAMBDA(callback));
}

// END TODO: REFACTOR THIS