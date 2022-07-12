#include "../../cgpu/common/utils.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.h"
#include "cgpu/api.h"
#include "platform/memory.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "ecs/callback.hpp"
#include "ecs/dual.h"
#include "ecs/type_builder.hpp"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "skr_scene/scene.h"
#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "gamert.h"
#include "cube.hpp"

SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();
const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT;

skr_render_pass_name_t forward_pass_name = "ForwardPass";
struct RenderPassForward : public IPrimitiveRenderPass {
    void on_register(ISkrRenderer* renderer) override
    {
    }

    void on_unregister(ISkrRenderer* renderer) override
    {
    }

    ECGPUShadingRate shading_rate = CGPU_SHADING_RATE_FULL;
    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto depth = renderGraph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name("depth")
                    .extent(900, 900)
                    .format(depth_format)
                    .owns_memory()
                    .allow_depth_stencil();
            });
        // IMGUI control shading rate
        {
            const char* shadingRateNames[] = {
                "1x1", "2x2", "4x4", "1x2", "2x1", "2x4", "4x2"
            };
            ImGui::Begin(u8"ShadingRate");
            if (ImGui::Button(fmt::format("SwitchShadingRate-{}", shadingRateNames[shading_rate]).c_str()))
            {
                if (shading_rate != CGPU_SHADING_RATE_COUNT - 1)
                    shading_rate = (ECGPUShadingRate)(shading_rate + 1);
                else
                    shading_rate = CGPU_SHADING_RATE_FULL;
            }
            ImGui::End();
        }
        renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                const auto out_color = renderGraph->get_texture("backbuffer");
                const auto depth_buffer = renderGraph->get_texture("depth");
                builder.set_name("forward_pass")
                    // we know that the drawcalls always have a same pipeline
                    .set_pipeline(drawcalls.drawcalls->pipeline)
                    .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                    .set_depth_stencil(depth_buffer);
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)900, (float)900,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, 900, 900);
                for (uint32_t i = 0; i < drawcalls.count; i++)
                {
                    auto&& dc = drawcalls.drawcalls[i];
                    cgpu_render_encoder_bind_index_buffer(stack.encoder, dc.index_buffer.buffer, 
                        dc.index_buffer.stride, dc.index_buffer.offset);
                    CGPUBufferId vertex_buffers[3] = {
                        dc.vertex_buffers[0].buffer, dc.vertex_buffers[1].buffer, dc.vertex_buffers[2].buffer
                    };
                    const uint32_t strides[3] = {
                        dc.vertex_buffers[0].stride, dc.vertex_buffers[1].stride, dc.vertex_buffers[2].stride
                    };
                    const uint32_t offsets[3] = {
                        dc.vertex_buffers[0].offset, dc.vertex_buffers[1].offset, dc.vertex_buffers[2].offset
                    };
                    cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 3, vertex_buffers, strides, offsets);
                    cgpu_render_encoder_push_constants(stack.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                    cgpu_render_encoder_set_shading_rate(stack.encoder, shading_rate, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH);
                    cgpu_render_encoder_draw_indexed_instanced(stack.encoder, 36, 0, 1, 0, 0);
                }
            });
    }

    skr_render_pass_name_t identity() const override
    {
        return forward_pass_name;
    }
};
RenderPassForward* forward_pass = new RenderPassForward();

typedef struct forward_effect_identity_t {
    dual_entity_t game_entity;
} forward_effect_identity_t;
skr_render_effect_name_t forward_effect_name = "ForwardEffect";
struct RenderEffectForward : public IRenderEffectProcessor {
    ~RenderEffectForward() = default;

    void on_register(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "fwdIdentity";
            desc.size = sizeof(forward_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(forward_effect_identity_t);
            identity_type = dualT_register_type(&desc);
        }
        type_builder.with(identity_type);
        effect_query = dualQ_from_literal(storage, "[in]fwdIdentity");
        // prepare render resources
        prepare_pipeline(renderer);
        prepare_geometry_resources(renderer);
    }

    void on_unregister(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        free_pipeline(renderer);
        free_geometry_resources(renderer);
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        *set = type_builder.build();
    }

    dual_type_index_t get_identity_type() override
    {
        return identity_type;
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {
        auto game_ents = dualV_get_entities(game_cv);
        auto identities = (forward_effect_identity_t*)dualV_get_owned_ro(render_cv, identity_type);
        for (uint32_t i = 0u; i < game_cv->count; ++i)
        {
            identities[i].game_entity = game_ents[i];
        }
    }

    uint32_t produce_drawcall(IPrimitiveRenderPass* pass, dual_storage_t* storage) override
    {
        // query from identity component
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
            uint32_t c = 0;
            auto counterF = [&](dual_chunk_view_t* cv) {
                c += cv->count;
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(counterF));
            push_constants.clear();
            push_constants.resize(c);
            return c;
        }
        return 0;
    }

    void peek_drawcall(IPrimitiveRenderPass* pass, skr_primitive_draw_list_view_t* drawcalls) override
    {
        // SKR_LOG_FMT_INFO("Pass {} asked Feature {} to peek drawcall", pass->identity(), forward_effect_name);
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
            auto storage = skr_runtime_get_dual_storage();
            auto r_effect_callback = [&](dual_chunk_view_t* r_cv) {
                auto identities = (forward_effect_identity_t*)dualV_get_owned_rw(r_cv, identity_type);
                auto unbatched_g_ents = (dual_entity_t*)identities;
                auto r_ents = dualV_get_entities(r_cv);
                if (unbatched_g_ents)
                {
                    uint32_t idx = 0;
                    auto g_batch_callback = [&](dual_chunk_view_t* g_cv) {
                        auto g_ents = (dual_entity_t*)dualV_get_entities(g_cv);
                        auto translations = (skr_translation_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_translation_t>::get());
                        auto rotations = (skr_rotation_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_rotation_t>::get());
                        auto scales = (skr_scale_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_scale_t>::get());
                        for (uint32_t i = 0; i < g_cv->count; i++)
                        {
                            auto g_ent = g_ents[i];
                            auto r_ent = r_ents[idx];
                            (void)g_ent;
                            (void)r_ent;
                            (void)rotations;
                            push_constants[idx].world = skr::math::make_transform(
                            translations[idx].value,
                            scales[idx].value,
                            skr::math::Quaternion::identity());
                            auto view = skr::math::look_at_matrix({ 0.f, 0.f, 12.5f } /*eye*/, { 0.f, 0.f, 0.f } /*at*/);
                            auto proj = skr::math::perspective_fov(3.1415926f / 2.f, (float)900 / (float)900, 1.f, 1000.f);
                            push_constants[idx].view_proj = skr::math::multiply(view, proj);
                            // drawcall
                            auto& drawcall = drawcalls->drawcalls[idx];
                            drawcall.push_const_name = push_constants_name;
                            drawcall.push_const = (const uint8_t*)(push_constants.data() + idx);
                            drawcall.index_buffer = ibv;
                            drawcall.vertex_buffers = vbvs;
                            drawcall.vertex_buffer_count = 3;
                            drawcall.pipeline = pipeline;
                            idx++;
                        }
                    };
                    dualS_batch(storage, unbatched_g_ents, r_cv->count, DUAL_LAMBDA(g_batch_callback));
                }
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(r_effect_callback));
        }
    }

protected:
    // TODO: move these anywhere else
    void prepare_geometry_resources(ISkrRenderer* renderer);
    void free_geometry_resources(ISkrRenderer* renderer);
    void prepare_pipeline(ISkrRenderer* renderer);
    void free_pipeline(ISkrRenderer* renderer);
    // render resources
    skr_vertex_buffer_view_t vbvs[3];
    skr_index_buffer_view_t ibv;
    CGPUBufferId vertex_buffer;
    CGPUBufferId index_buffer;
    CGPURenderPipelineId pipeline;
    // effect processor data
    const char* push_constants_name = "push_constants";
    dual_query_t* effect_query = nullptr;
    dual_type_index_t identity_type = {};
    dual::type_builder_t type_builder;
    struct PushConstants {
        skr::math::float4x4 world;
        skr::math::float4x4 view_proj;
    };
    eastl::vector<PushConstants> push_constants;
};
RenderEffectForward* forward_effect = new RenderEffectForward();

void RenderEffectForward::prepare_geometry_resources(ISkrRenderer* renderer)
{
    const auto device = renderer->get_cgpu_device();
    const auto gfx_queue = renderer->get_gfx_queue();
    // upload
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "UploadBuffer";
    upload_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGPUBufferDescriptor vb_desc = {};
    vb_desc.name = "VertexBuffer";
    vb_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(CubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGPUBufferDescriptor ib_desc = {};
    ib_desc.name = "IndexBuffer";
    ib_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    ib_desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(CubeGeometry::g_Indices);
    index_buffer = cgpu_create_buffer(device, &ib_desc);
    auto pool_desc = CGPUCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc = CGPUCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = CubeGeometry();
        memcpy(upload_buffer->cpu_mapped_address, &geom, sizeof(CubeGeometry));
    }
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToBufferTransfer vb_cpy = {};
    vb_cpy.dst = vertex_buffer;
    vb_cpy.dst_offset = 0;
    vb_cpy.src = upload_buffer;
    vb_cpy.src_offset = 0;
    vb_cpy.size = sizeof(CubeGeometry);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &vb_cpy);
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(CubeGeometry),
        CubeGeometry::g_Indices, sizeof(CubeGeometry::g_Indices));
    }
    CGPUBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst = index_buffer;
    ib_cpy.dst_offset = 0;
    ib_cpy.src = upload_buffer;
    ib_cpy.src_offset = sizeof(CubeGeometry);
    ib_cpy.size = sizeof(CubeGeometry::g_Indices);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &ib_cpy);
    // barriers
    CGPUBufferBarrier barriers[2] = {};
    CGPUBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = vertex_buffer;
    vb_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGPUBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = index_buffer;
    ib_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = CGPU_RESOURCE_STATE_INDEX_BUFFER;
    CGPUResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 2;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
    cgpu_cmd_end(cpy_cmd);
    CGPUQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cmd_pool);
    // init vbvs & ibvs
    vbvs[0].buffer = vertex_buffer;
    vbvs[0].stride = sizeof(skr::math::Vector3f);
    vbvs[0].offset = offsetof(CubeGeometry, g_Positions);
    vbvs[1].buffer = vertex_buffer;
    vbvs[1].stride = sizeof(skr::math::Vector2f);
    vbvs[1].offset = offsetof(CubeGeometry, g_TexCoords);
    vbvs[2].buffer = vertex_buffer;
    vbvs[2].stride = sizeof(uint32_t);
    vbvs[2].offset = offsetof(CubeGeometry, g_Normals);
    ibv.buffer = index_buffer;
    ibv.offset = 0;
    ibv.stride = sizeof(uint32_t);
}

void RenderEffectForward::free_geometry_resources(ISkrRenderer* renderer)
{
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
}

void RenderEffectForward::prepare_pipeline(ISkrRenderer* renderer)
{
    auto moduleManager = skr_get_module_manager();
    const auto device = renderer->get_cgpu_device();
    const auto backend = device->adapter->instance->backend;

    // read shaders
    eastl::string vsname = u8"shaders/Game/gbuffer_vs";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto gamert = (SGameRTModule*)moduleManager->get_module("GameRT");
    auto vsfile = skr_vfs_fopen(gamert->resource_vfs, vsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    uint32_t* _vs_bytes = (uint32_t*)sakura_malloc(_vs_length);
    skr_vfs_fread(vsfile, _vs_bytes, 0, _vs_length);
    skr_vfs_fclose(vsfile);

    eastl::string fsname = u8"shaders/Game/gbuffer_fs";
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto fsfile = skr_vfs_fopen(gamert->resource_vfs, fsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(fsfile);
    uint32_t* _fs_bytes = (uint32_t*)sakura_malloc(_fs_length);
    skr_vfs_fread(fsfile, _fs_bytes, 0, _fs_length);
    skr_vfs_fclose(fsfile);

    // create default deferred material
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "gbuffer_vertex_buffer";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = _vs_bytes;
    vs_desc.code_size = _vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "gbuffer_fragment_buffer";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = _fs_bytes;
    fs_desc.code_size = _fs_length;
    CGPUShaderLibraryId _vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId _fs = cgpu_create_shader_library(device, &fs_desc);
    sakura_free(_vs_bytes);
    sakura_free(_fs_bytes);

    CGPUPipelineShaderDescriptor ppl_shaders[2];
    CGPUPipelineShaderDescriptor& vs = ppl_shaders[0];
    vs.library = _vs;
    vs.stage = CGPU_SHADER_STAGE_VERT;
    vs.entry = "main";
    CGPUPipelineShaderDescriptor& ps = ppl_shaders[1];
    ps.library = _fs;
    ps.stage = CGPU_SHADER_STAGE_FRAG;
    ps.entry = "main";

    auto rs_desc = make_zeroed<CGPURootSignatureDescriptor>();
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constants_name;
    rs_desc.shader_count = 2;
    rs_desc.shaders = ppl_shaders;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, CGPU_FORMAT_R8G8B8A8_SNORM, 2, 0, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 3;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &vs;
    rp_desc.fragment_shader = &ps;
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
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);

    cgpu_free_shader_library(_vs);
    cgpu_free_shader_library(_fs);
}

void RenderEffectForward::free_pipeline(ISkrRenderer* renderer)
{
    auto sig_to_free = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void initialize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_register_render_pass(renderer, forward_pass_name, forward_pass);
    skr_renderer_register_render_effect(renderer, forward_effect_name, forward_effect);
}

void finalize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_remove_render_pass(renderer, forward_pass_name);
    skr_renderer_remove_render_effect(renderer, forward_effect_name);
    delete forward_effect;
    delete forward_pass;
}