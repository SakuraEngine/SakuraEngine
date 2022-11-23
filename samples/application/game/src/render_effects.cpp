#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.hpp"
#include "cgpu/api.h"
#include "platform/memory.h"
#include "ecs/callback.hpp"
#include "ecs/dual.h"
#include "ecs/type_builder.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"
#include "SkrScene/scene.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderer/render_effect.h"
#include "cube.hpp"
#include "platform/vfs.h"
#include <platform/filesystem.hpp>

#include "resource/resource_system.h"

#include "tracy/Tracy.hpp"

const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT_S8_UINT;

skr_render_pass_name_t forward_pass_name = "ForwardPass";
struct RenderPassForward : public IPrimitiveRenderPass {
    void on_update(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph) override
    {

    }

    void post_update(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph) override
    {

    }

    struct DrawCallListData
    {
        skr::math::float4x4 view_projection;
        uint32_t viewport_width;
        uint32_t viewport_height;
    };

    ECGPUShadingRate shading_rate = CGPU_SHADING_RATE_FULL;
    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {
        const auto list_data = *(DrawCallListData*)drawcalls.user_data;
        auto depth = renderGraph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name("depth")
                    .extent(list_data.viewport_width, list_data.viewport_height)
                    .format(depth_format)
                    .owns_memory()
                    .allow_depth_stencil();
            });(void)depth;
        if (drawcalls.count)
        {
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
            auto cbuffer = renderGraph->create_buffer(
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::BufferBuilder& builder) {
                    builder.set_name("forward_cbuffer")
                        .size(sizeof(skr_float4x4_t))
                        .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                        .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
                        .with_tags(kRenderGraphDynamicResourceTag)
                        .prefer_on_device()
                        .as_uniform_buffer();
                });
            renderGraph->add_render_pass(
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                    const auto out_color = renderGraph->get_texture("backbuffer");
                    const auto depth_buffer = renderGraph->get_texture("depth");
                    builder.set_name("forward_pass")
                        // we know that the drawcalls always have a same pipeline
                        .set_pipeline(drawcalls.drawcalls->pipeline)
                        .read("ForwardRenderConstants", cbuffer.range(0, sizeof(skr_float4x4_t)))
                        .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                        .set_depth_stencil(depth_buffer);
                },
                [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                    auto cb = stack.resolve(cbuffer);
                    SKR_ASSERT(cb && "cbuffer not found");
                    ::memcpy(cb->cpu_mapped_address, &list_data.view_projection, sizeof(list_data.view_projection));
                    cgpu_render_encoder_set_viewport(stack.encoder,
                        0.0f, 0.0f,
                        (float)list_data.viewport_width, (float)list_data.viewport_height,
                        0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder,
                        0, 0, 
                        list_data.viewport_width, list_data.viewport_height);
                    
                    for (uint32_t i = 0; i < drawcalls.count; i++)
                    {
                        ZoneScopedN("DrawCall");

                        auto&& dc = drawcalls.drawcalls[i];
                        {
                            ZoneScopedN("BindGeometry");
                            cgpu_render_encoder_bind_index_buffer(stack.encoder, 
                                dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
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
                        }
                        {
                            ZoneScopedN("PushConstants");
                            cgpu_render_encoder_push_constants(stack.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                        }
                        cgpu_render_encoder_set_shading_rate(stack.encoder, shading_rate, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH);
                        {
                            ZoneScopedN("DrawIndexed");
                            cgpu_render_encoder_draw_indexed_instanced(stack.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                        }
                    }
                });
        }
    }

    skr_render_pass_name_t identity() const override
    {
        return forward_pass_name;
    }
};
RenderPassForward* forward_pass = nullptr;

typedef struct forward_effect_identity_t {
    dual_entity_t game_entity;
} forward_effect_identity_t;
skr_render_effect_name_t forward_effect_name = "ForwardEffect";
struct RenderEffectForward : public IRenderEffectProcessor {
    RenderEffectForward(skr_vfs_t* resource_vfs)
        :resource_vfs(resource_vfs) {}
    ~RenderEffectForward() = default;
    dual::type_builder_t type_builder;
    dual_type_set_t typeset;
    skr_vfs_t* resource_vfs;

    void on_register(SRendererId renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "forward_render_identity";
            desc.size = sizeof(forward_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(forward_effect_identity_t);
            identity_type = dualT_register_type(&desc);
            type_builder.with(identity_type);
            type_builder.with<skr_render_mesh_comp_t>();
            typeset = type_builder.build();
        }
        // initialize queries
        effect_query = dualQ_from_literal(storage, "[in]forward_render_identity");
        camera_query = dualQ_from_literal(storage, "[in]skr_camera_t");
        // prepare render resources
        prepare_pipeline(renderer);
        prepare_geometry_resources(renderer);
    }

    void on_unregister(SRendererId renderer, dual_storage_t* storage) override
    {
        auto sweepFunction = [&](dual_chunk_view_t* r_cv) {
            auto resource_system = skr::resource::GetResourceSystem();
            auto meshes = (skr_render_mesh_comp_t*)dualV_get_owned_ro(r_cv, dual_id_of<skr_render_mesh_comp_t>::get());
            for (uint32_t i = 0; i < r_cv->count; i++)
            {
                auto status = meshes[i].mesh_resource.get_status();
                if (status == ESkrLoadingStatus::SKR_LOADING_STATUS_INSTALLED)
                {
                    auto mesh_resource = (skr_mesh_resource_id)meshes[i].mesh_resource.get_ptr();
                    SKR_LOG_TRACE("Mesh Loaded: name - %s, bin0 - %s", mesh_resource->name.c_str(), mesh_resource->bins[0].uri.c_str());
                    resource_system->UnloadResource(meshes[i].mesh_resource);
                    resource_system->Update();
                    while (meshes[i].mesh_resource.get_status() != SKR_LOADING_STATUS_UNLOADED)
                    {
                        resource_system->Update();
                    }
                }
            }
        };
        dualQ_get_views(effect_query, DUAL_LAMBDA(sweepFunction));
        free_pipeline(renderer);
        free_geometry_resources(renderer);
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        *set = typeset;
    }

    dual_type_index_t get_identity_type() override
    {
        return identity_type;
    }

    void initialize_data(SRendererId renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {
        auto game_ents = dualV_get_entities(game_cv);
        auto identities = (forward_effect_identity_t*)dualV_get_owned_ro(render_cv, identity_type);
        for (uint32_t i = 0u; i < game_cv->count; ++i)
        {
            identities[i].game_entity = game_ents[i];
        }
    }

    eastl::vector<skr_primitive_draw_t> mesh_drawcalls;
    skr_primitive_draw_list_view_t mesh_draw_list;
    skr_primitive_draw_packet_t produce_draw_packets(IPrimitiveRenderPass* pass, dual_storage_t* storage) override
    {
        skr_primitive_draw_packet_t packet = {};
        // query from identity component
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
            uint32_t c = 0;
            auto counterF = [&](dual_chunk_view_t* r_cv) {
                auto meshes = (skr_render_mesh_comp_t*)dualV_get_owned_ro(r_cv, dual_id_of<skr_render_mesh_comp_t>::get());
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    auto status = meshes[i].mesh_resource.get_status();
                    if (status == SKR_LOADING_STATUS_INSTALLED)
                    {
                        auto resourcePtr = (skr_mesh_resource_t*)meshes[i].mesh_resource.get_ptr();
                        auto renderMesh = resourcePtr->render_mesh;
                        c += (uint32_t)renderMesh->primitive_commands.size();
                    }
                    else
                    {
                        c++;
                    }
                }
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(counterF));
            push_constants.clear();
            mesh_drawcalls.clear();
            push_constants.reserve(c);
            mesh_drawcalls.reserve(c);
            auto view = skr::math::look_at_matrix(
                { 0.f, -135.f, 55.f } /*eye*/, 
                { 0.f, 0.f, 50.f } /*at*/,
                { 0.f, 0.f, 1.f } /*up*/
            );
            auto cameraSetup = [&](dual_chunk_view_t* g_cv) {
                auto cameras = (skr_camera_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_camera_t>::get());
                auto camera_transforms = (skr_translation_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_translation_t>::get());
                auto camera_forward = skr::math::Vector3f(0.f, 1.f, 0.f);
                SKR_ASSERT(g_cv->count <= 1);
                if (cameras)
                {
                    forward_pass_data.viewport_width = cameras->viewport_width;
                    forward_pass_data.viewport_height = cameras->viewport_height;

                    view = skr::math::look_at_matrix(
                        camera_transforms->value /*eye*/, 
                        camera_forward + camera_transforms->value /*at*/,
                        { 0.f, 0.f, 1.f } /*up*/
                    );
                    auto proj = skr::math::perspective_fov(
                        3.1415926f / 2.f, 
                        (float)forward_pass_data.viewport_width / (float)forward_pass_data.viewport_height, 
                        1.f, 1000.f);
                    forward_pass_data.view_projection = skr::math::multiply(view, proj);
                }
            };
            dualQ_get_views(camera_query, DUAL_LAMBDA(cameraSetup));

            auto r_effect_callback = [&](dual_chunk_view_t* r_cv) {
                uint32_t r_idx = 0;
                uint32_t dc_idx = 0;

                auto identities = (forward_effect_identity_t*)dualV_get_owned_rw(r_cv, identity_type);
                auto unbatched_g_ents = (dual_entity_t*)identities;
                auto meshes = (skr_render_mesh_comp_t*)dualV_get_owned_ro(r_cv, dual_id_of<skr_render_mesh_comp_t>::get());
                if (unbatched_g_ents)
                {
                    auto g_batch_callback = [&](dual_chunk_view_t* g_cv) {
                        //SKR_LOG_DEBUG("batch: %d -> %d", g_cv->start, g_cv->count);
                        auto translations = (skr_translation_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_translation_t>::get());
                        auto rotations = (skr_rotation_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_rotation_t>::get());(void)rotations;
                        auto scales = (skr_scale_t*)dualV_get_owned_ro(g_cv, dual_id_of<skr_scale_t>::get());
                        for (uint32_t g_idx = 0; g_idx < g_cv->count; g_idx++, r_idx++)
                        {
                            const auto quaternion = skr::math::quaternion_from_euler(
                                rotations[g_idx].euler.pitch, rotations[g_idx].euler.yaw, rotations[g_idx].euler.roll);
                            auto world = skr::math::make_transform(
                                translations[g_idx].value,
                                scales[g_idx].value,
                                quaternion);
                            //SKR_LOG_DEBUG("primitives: %d (%f, %f, %f)", dc_idx, translations[g_idx].value.x, translations[g_idx].value.y, translations[g_idx].value.z);
                            //SKR_LOG_DEBUG("primitives: %d (%f, %f, %f)", dc_idx, quaternion.x, quaternion.y, quaternion.z);
                            //SKR_LOG_DEBUG("primitives: %d (%f, %f, %f)", dc_idx, scales[g_idx].value.x, scales[g_idx].value.y, scales[g_idx].value.z);
                            // drawcall
                            auto status = meshes[r_idx].mesh_resource.get_status();
                            if (status == SKR_LOADING_STATUS_INSTALLED)
                            {
                                auto resourcePtr = (skr_mesh_resource_t*)meshes[r_idx].mesh_resource.get_ptr();
                                auto renderMesh = resourcePtr->render_mesh;
                    
                                const auto& cmds = renderMesh->primitive_commands;
                                for (auto&& cmd : cmds)
                                {
                                    // resources may be ready after produce_drawcall, so we need to check it here
                                    if (push_constants.capacity() <= dc_idx) return;

                                    auto& push_const = push_constants.emplace_back();
                                    push_const.world = world;
                                    auto& drawcall = mesh_drawcalls.emplace_back();
                                    drawcall.pipeline = pipeline;
                                    drawcall.push_const_name = push_constants_name;
                                    drawcall.push_const = (const uint8_t*)(&push_const);
                                    drawcall.index_buffer = *cmd.ibv;
                                    drawcall.vertex_buffers = cmd.vbvs.data();
                                    drawcall.vertex_buffer_count = (uint32_t)cmd.vbvs.size();
                                    dc_idx++;
                                }
                            }
                            else
                            {
                                // resources may be ready after produce_drawcall, so we need to check it here
                                if (push_constants.capacity() <= dc_idx) return;

                                auto& push_const = push_constants.emplace_back();
                                push_const.world = world;
                                auto& drawcall = mesh_drawcalls.emplace_back();
                                drawcall.pipeline = pipeline;
                                drawcall.push_const_name = push_constants_name;
                                drawcall.push_const = (const uint8_t*)(&push_const);
                                drawcall.index_buffer = ibv;
                                drawcall.vertex_buffers = vbvs;
                                drawcall.vertex_buffer_count = 4;
                                dc_idx++;
                            }
                        }
                    };
                    dualS_batch(storage, unbatched_g_ents, r_cv->count, DUAL_LAMBDA(g_batch_callback));
                }
            };
            dualQ_get_views(effect_query, DUAL_LAMBDA(r_effect_callback));
            mesh_draw_list.drawcalls = mesh_drawcalls.data();
            mesh_draw_list.count = (uint32_t)mesh_drawcalls.size();
            mesh_draw_list.user_data = &forward_pass_data;
            packet.count = 1;
            packet.lists = &mesh_draw_list;
        }
        return packet;
    }

protected:
    // TODO: move these anywhere else
    void prepare_geometry_resources(SRendererId renderer);
    void free_geometry_resources(SRendererId renderer);
    void prepare_pipeline(SRendererId renderer);
    void free_pipeline(SRendererId renderer);
    // render resources
    skr_vertex_buffer_view_t vbvs[4];
    skr_index_buffer_view_t ibv;
    CGPUBufferId vertex_buffer;
    CGPUBufferId index_buffer;
    CGPURenderPipelineId pipeline;
    // effect processor data
    const char* push_constants_name = "push_constants";
    dual_query_t* effect_query = nullptr;
    dual_query_t* camera_query = nullptr;
    dual_type_index_t identity_type = {};
    struct PushConstants {
        skr::math::float4x4 world;
    };
    eastl::vector<PushConstants> push_constants;
    RenderPassForward::DrawCallListData forward_pass_data;
};
RenderEffectForward* forward_effect = nullptr;

void RenderEffectForward::prepare_geometry_resources(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto device = render_device->get_cgpu_device();
    const auto gfx_queue = render_device->get_gfx_queue();
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
    vbvs[2].stride = sizeof(skr::math::Vector3f);
    vbvs[2].offset = offsetof(CubeGeometry, g_Normals);
    vbvs[3].buffer = vertex_buffer;
    vbvs[3].stride = sizeof(skr::math::Vector4f);
    vbvs[3].offset = offsetof(CubeGeometry, g_Tangents);
    ibv.buffer = index_buffer;
    ibv.index_count = 36;
    ibv.first_index = 0;
    ibv.offset = 0;
    ibv.stride = sizeof(uint32_t);
}

void RenderEffectForward::free_geometry_resources(SRendererId renderer)
{
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
}

void RenderEffectForward::prepare_pipeline(SRendererId renderer)
{
    const auto render_device = renderer->get_render_device();
    const auto device = render_device->get_cgpu_device();
    const auto backend = device->adapter->instance->backend;

    // read shaders
    skr::string vsname = u8"shaders/Game/gbuffer_vs";
    vsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto vsfile = skr_vfs_fopen(resource_vfs, vsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _vs_length = (uint32_t)skr_vfs_fsize(vsfile);
    uint32_t* _vs_bytes = (uint32_t*)sakura_malloc(_vs_length);
    skr_vfs_fread(vsfile, _vs_bytes, 0, _vs_length);
    skr_vfs_fclose(vsfile);

    skr::string fsname = u8"shaders/Game/gbuffer_fs";
    fsname.append(backend == ::CGPU_BACKEND_D3D12 ? ".dxil" : ".spv");
    auto fsfile = skr_vfs_fopen(resource_vfs, fsname.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    uint32_t _fs_length = (uint32_t)skr_vfs_fsize(fsfile);
    uint32_t* _fs_bytes = (uint32_t*)sakura_malloc(_fs_length);
    skr_vfs_fread(fsfile, _fs_bytes, 0, _fs_length);
    skr_vfs_fclose(fsfile);

    // create default deferred material
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "gbuffer_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = _vs_bytes;
    vs_desc.code_size = _vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "gbuffer_pixel_shader";
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
    rs_desc.pool = render_device->get_root_signature_pool();
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);

    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 2, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "TANGENT", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 3, 0, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 3;

    auto rp_desc = make_zeroed<CGPURenderPipelineDescriptor>();
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &vs;
    rp_desc.fragment_shader = &ps;
    rp_desc.render_target_count = 1;
    const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
    rp_desc.color_formats = &fmt;
    rp_desc.depth_stencil_format = depth_format;

    auto raster_desc = make_zeroed<CGPURasterizerStateDescriptor>();
    raster_desc.cull_mode = CGPU_CULL_MODE_BACK;
    raster_desc.depth_bias = 0;
    raster_desc.fill_mode = CGPU_FILL_MODE_SOLID;
    raster_desc.front_face = CGPU_FRONT_FACE_CCW;

    auto ds_desc = make_zeroed<CGPUDepthStateDescriptor>();
    ds_desc.depth_func = CGPU_CMP_LEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;

    rp_desc.rasterizer_state = &raster_desc;
    rp_desc.depth_state = &ds_desc;
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);

    cgpu_free_shader_library(_vs);
    cgpu_free_shader_library(_fs);
}

void RenderEffectForward::free_pipeline(SRendererId renderer)
{
    auto sig_to_free = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(sig_to_free);
}

void game_initialize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph, skr_vfs_t* resource_vfs)
{
    forward_effect = new RenderEffectForward(resource_vfs);
    forward_pass = new RenderPassForward();
    skr_renderer_register_render_effect(renderer, forward_effect_name, forward_effect);
}

void game_register_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph)
{
    skr_renderer_register_render_pass(renderer, forward_pass_name, forward_pass);
}

void game_finalize_render_effects(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph)
{
    skr_renderer_remove_render_pass(renderer, forward_pass_name);
    skr_renderer_remove_render_effect(renderer, forward_effect_name);
    delete forward_effect;
    delete forward_pass;
}