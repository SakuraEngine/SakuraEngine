#include "cube_geometry.h"
#include "common/utils.h"
#include "gbuffer_pipeline.h"
#include "lighting_pipeline.h"
#include "blit_pipeline.h"

#include "SkrProfile/profile.h"
#include "SkrOS/thread.h"
#include "SkrOS/filesystem.hpp"
#include "SkrCore/module/module_manager.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/skr_renderer.h"
#include "SkrImGui/imgui_render_backend.hpp"
#include "SkrImGui/imgui_backend.hpp"
#include "pass_profiler.h"

CubeGeometry::InstanceData CubeGeometry::instance_data;

#if _WIN32
thread_local ECGPUBackend backend = CGPU_BACKEND_D3D12;
#else
thread_local ECGPUBackend backend = CGPU_BACKEND_VULKAN;
#endif

thread_local SRenderDeviceId render_device;
thread_local CGPUSamplerId static_sampler;

thread_local CGPUBufferId index_buffer;
thread_local CGPUBufferId vertex_buffer;
thread_local CGPUBufferId instance_buffer;

thread_local CGPURenderPipelineId  blit_pipeline;
thread_local CGPURenderPipelineId  gbuffer_pipeline;
thread_local CGPURenderPipelineId  lighting_pipeline;
thread_local CGPUComputePipelineId lighting_cs_pipeline;

void create_render_pipeline()
{
    render_device = skr_get_default_render_device();
    auto device = render_device->get_cgpu_device();
    auto gfx_queue = render_device->get_gfx_queue();

    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode           = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter            = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter            = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func          = CGPU_CMP_NEVER;
    static_sampler                     = cgpu_create_sampler(device, &sampler_desc);

    // upload
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name                 = u8"UploadBuffer";
    upload_buffer_desc.flags                = CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors          = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage         = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size                 = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices) + sizeof(CubeGeometry::InstanceData);
    auto                 upload_buffer      = cgpu_create_buffer(device, &upload_buffer_desc);
    CGPUBufferDescriptor vb_desc            = {};
    vb_desc.name                            = u8"VertexBuffer";
    vb_desc.flags                           = CGPU_BCF_NONE;
    vb_desc.descriptors                     = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage                    = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size                            = sizeof(CubeGeometry);
    vertex_buffer                           = cgpu_create_buffer(device, &vb_desc);
    CGPUBufferDescriptor ib_desc            = {};
    ib_desc.name                            = u8"IndexBuffer";
    ib_desc.flags                           = CGPU_BCF_NONE;
    ib_desc.descriptors                     = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    ib_desc.memory_usage                    = CGPU_MEM_USAGE_GPU_ONLY;
    ib_desc.size                            = sizeof(CubeGeometry::g_Indices);
    index_buffer                            = cgpu_create_buffer(device, &ib_desc);
    CGPUBufferDescriptor inb_desc           = {};
    inb_desc.name                           = u8"InstanceBuffer";
    inb_desc.flags                          = CGPU_BCF_NONE;
    inb_desc.descriptors                    = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    inb_desc.memory_usage                   = CGPU_MEM_USAGE_GPU_ONLY;
    inb_desc.size                           = sizeof(CubeGeometry::InstanceData);
    instance_buffer                         = cgpu_create_buffer(device, &inb_desc);
    auto pool_desc                          = CGPUCommandPoolDescriptor();
    auto cmd_pool                           = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc                           = CGPUCommandBufferDescriptor();
    auto cpy_cmd                            = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = CubeGeometry();
        memcpy(upload_buffer->info->cpu_mapped_address, &geom, upload_buffer_desc.size);
    }
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToBufferTransfer vb_cpy = {};
    vb_cpy.dst                        = vertex_buffer;
    vb_cpy.dst_offset                 = 0;
    vb_cpy.src                        = upload_buffer;
    vb_cpy.src_offset                 = 0;
    vb_cpy.size                       = sizeof(CubeGeometry);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &vb_cpy);
    {
        memcpy((char8_t*)upload_buffer->info->cpu_mapped_address + sizeof(CubeGeometry), CubeGeometry::g_Indices, sizeof(CubeGeometry::g_Indices));
    }
    CGPUBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst                        = index_buffer;
    ib_cpy.dst_offset                 = 0;
    ib_cpy.src                        = upload_buffer;
    ib_cpy.src_offset                 = sizeof(CubeGeometry);
    ib_cpy.size                       = sizeof(CubeGeometry::g_Indices);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &ib_cpy);
    // wvp
    const auto transform = skr::TransformF(skr::QuatF(skr::RotatorF()), skr::float3(0), skr::float3(2));
    const auto matrix = skr::transpose(transform.to_matrix());
    CubeGeometry::instance_data.world = *(skr_float4x4_t*)&matrix;
    {
        memcpy((char8_t*)upload_buffer->info->cpu_mapped_address + sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices), &CubeGeometry::instance_data, sizeof(CubeGeometry::InstanceData));
    }
    CGPUBufferToBufferTransfer istb_cpy = {};
    istb_cpy.dst                        = instance_buffer;
    istb_cpy.dst_offset                 = 0;
    istb_cpy.src                        = upload_buffer;
    istb_cpy.src_offset                 = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    istb_cpy.size                       = sizeof(CubeGeometry::instance_data);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &istb_cpy);
    CGPUBufferBarrier  barriers[3]             = {};
    CGPUBufferBarrier& vb_barrier              = barriers[0];
    vb_barrier.buffer                          = vertex_buffer;
    vb_barrier.src_state                       = CGPU_RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state                       = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGPUBufferBarrier& ib_barrier              = barriers[1];
    ib_barrier.buffer                          = index_buffer;
    ib_barrier.src_state                       = CGPU_RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state                       = CGPU_RESOURCE_STATE_INDEX_BUFFER;
    CGPUBufferBarrier& ist_barrier             = barriers[2];
    ist_barrier.buffer                         = instance_buffer;
    ist_barrier.src_state                      = CGPU_RESOURCE_STATE_COPY_DEST;
    ist_barrier.dst_state                      = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGPUResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers               = barriers;
    barrier_desc.buffer_barriers_count         = 3;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
    cgpu_cmd_end(cpy_cmd);
    CGPUQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds                      = &cpy_cmd;
    cpy_submit.cmds_count                = 1;
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cmd_pool);

    gbuffer_pipeline     = create_gbuffer_render_pipeline(device);
    lighting_pipeline    = create_lighting_render_pipeline(device, static_sampler, CGPU_FORMAT_R8G8B8A8_UNORM);
    lighting_cs_pipeline = create_lighting_compute_pipeline(device);
    blit_pipeline        = create_blit_render_pipeline(device, static_sampler, CGPU_FORMAT_R8G8B8A8_UNORM);
}

void finalize()
{
    // Free cgpu objects
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
    cgpu_free_buffer(instance_buffer);
    free_pipeline_and_signature(gbuffer_pipeline);
    free_pipeline_and_signature(lighting_pipeline);
    free_pipeline_and_signature(lighting_cs_pipeline);
    free_pipeline_and_signature(blit_pipeline);
    cgpu_free_sampler(static_sampler);
}

struct LightingPushConstants {
    int bFlipUVX = 0;
    int bFlipUVY = 0;
};
static LightingPushConstants lighting_data = {};
struct LightingCSPushConstants {
    skr_float2_t viewportSize   = { BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT };
    skr_float2_t viewportOrigin = { 0, 0 };
};
static LightingCSPushConstants lighting_cs_data     = {};
bool                           fragmentLightingPass = false;
bool                           lockFPS              = true;
bool                           DPIAware             = false;

#include "SkrRT/runtime_module.h"

struct RenderGraphDeferredModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;
};
IMPLEMENT_DYNAMIC_MODULE(RenderGraphDeferredModule, RenderGraphDeferred);

void RenderGraphDeferredModule::on_load(int argc, char8_t** argv)
{
    DPIAware = skr_runtime_is_dpi_aware();
    create_render_pipeline();
}

int RenderGraphDeferredModule::main_module_exec(int argc, char8_t** argv)
{
    // init rendering
    namespace render_graph = skr::render_graph;
    render_graph::RenderGraph*   graph;
    PassProfiler                 profilers[RG_MAX_FRAME_IN_FLIGHT];
    skr::UPtr<skr::ImGuiApp> imgui_app = nullptr;
    skr::ImGuiRendererBackendRG* imgui_render = nullptr;
    {
        // init render graph
        auto device = render_device->get_cgpu_device();
        auto gfx_queue = render_device->get_gfx_queue();
        graph = render_graph::RenderGraph::create(
            [=](skr::render_graph::RenderGraphBuilder& builder) {
                builder.with_device(device)
                    .with_gfx_queue(gfx_queue)
                    .enable_memory_aliasing();
            }
        );

        // init pass profiler
        for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
        {
            profilers[i].initialize(device);
        }

        // init imgui backend
        {
            using namespace skr;

            // init imgui backend
            auto render_backend     = RCUnique<ImGuiRendererBackendRG>::New();
            imgui_render = render_backend.get();
            ImGuiRendererBackendRGConfig config{};
            config.render_graph   = graph;
            config.queue          = gfx_queue;
            config.static_sampler = static_sampler;
            render_backend->init(config);

            skr::SystemWindowCreateInfo window_config = {
                .size = { BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT },
                .is_resizable = false
            };
            imgui_app = UPtr<ImGuiApp>::New(window_config, std::move(render_backend));
            imgui_app->initialize();
            imgui_app->enable_docking();

            // load font
            const char8_t* font_path = u8"./../resources/font/SourceSansPro-Regular.ttf";
            uint32_t *     font_bytes, font_length;
            read_bytes(font_path, &font_bytes, &font_length);
            ImFontConfig cfg = {};
            cfg.SizePixels   = 16.f;
            cfg.OversampleH = cfg.OversampleV = 1;
            cfg.PixelSnapH                    = true;
            ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
                font_bytes,
                font_length,
                cfg.SizePixels,
                &cfg
            );
            ImGui::GetIO().Fonts->Build();
        }
    }

    // loop
    while (!imgui_app->want_exit().comsume())
    {
        imgui_app->pump_message();

        // draw imgui
        SkrZoneScopedN("FrameTime");
        static uint64_t frame_index = 0;
        {
            SkrZoneScopedN("ImGui");
            imgui_app->begin_frame();
            ImGui::Begin("RenderGraphProfile");
            if (ImGui::Button(fragmentLightingPass ? "SwitchToComputeLightingPass" : "SwitchToFragmentLightingPass"))
            {
                fragmentLightingPass = !fragmentLightingPass;
            }
            if (ImGui::Button(lockFPS ? "UnlockFPS" : "LockFPS"))
            {
                lockFPS = !lockFPS;
            }
            if (frame_index > RG_MAX_FRAME_IN_FLIGHT)
            {
                auto   profiler_index = (frame_index - 1) % RG_MAX_FRAME_IN_FLIGHT;
                auto&& profiler       = profilers[profiler_index];
                if (profiler.times_ms.size() == profiler.query_names.size())
                {
                    // text
                    ImGui::Text("frame: %llu(%llu frames before)", profiler.frame_index, frame_index - profiler.frame_index);
                    float total_ms = 0.f;
                    for (uint32_t i = 1; i < profiler.times_ms.size(); i++)
                    {
                        auto text = profiler.query_names[i];
                        text.append(u8": %.4f ms");
                        ImGui::Text(text.c_str_raw(), profiler.times_ms[i]);
                        total_ms += profiler.times_ms[i];
                    }
                    ImGui::Text("GPU Time: %f(ms)", total_ms);
                    // plot
                    auto max_scale = std::max_element(profiler.times_ms.begin(), profiler.times_ms.end());
                    auto min_scale = std::max_element(profiler.times_ms.begin(), profiler.times_ms.end());
                    (void)min_scale;
                    ImVec2 size = { 200, 200 };
                    ImGui::PlotHistogram("##ms", profiler.times_ms.data() + 1, (int)profiler.times_ms.size() - 1, 0, NULL, 0.0001f, *max_scale * 1.1f, size);
                }
            }
            ImGui::End();
            imgui_app->end_frame();
        }

        // rendering
        auto native_backbuffer = imgui_render->get_backbuffer(ImGui::GetMainViewport());
        imgui_render->set_load_action(ImGui::GetMainViewport(), CGPU_LOAD_ACTION_LOAD);
        {
            SkrZoneScopedN("GraphSetup");
            // render graph setup & compile & exec
            auto back_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(u8"backbuffer")
                        .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
                        .allow_render_target();
                }
            );
            render_graph::TextureHandle composite_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(u8"composite_buffer")
                        .extent(native_backbuffer->info->width, native_backbuffer->info->height)
                        .format(CGPU_FORMAT_R8G8B8A8_UNORM)
                        .allocate_dedicated()
                        .allow_render_target();
                }
            );
            auto gbuffer_color = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(u8"gbuffer_color")
                        .extent(native_backbuffer->info->width, native_backbuffer->info->height)
                        .format(gbuffer_formats[0])
                        .allocate_dedicated()
                        .allow_render_target();
                }
            );
            auto gbuffer_depth = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(u8"gbuffer_depth")
                        .extent(native_backbuffer->info->width, native_backbuffer->info->height)
                        .format(gbuffer_depth_format)
                        .allocate_dedicated()
                        .allow_depth_stencil();
                }
            );
            auto gbuffer_normal = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(u8"gbuffer_normal")
                        .extent(native_backbuffer->info->width, native_backbuffer->info->height)
                        .format(gbuffer_formats[1])
                        .allocate_dedicated()
                        .allow_render_target();
                }
            );
            auto lighting_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(u8"lighting_buffer")
                        .extent(native_backbuffer->info->width, native_backbuffer->info->height)
                        .format(lighting_buffer_format)
                        .allocate_dedicated()
                        .allow_readwrite();
                }
            );
            // camera
            auto eye = skr::float4(0.f, 2.1f, -2.1f, 0.0f) /*eye*/;
            auto view = skr::float4x4::view_at(eye, skr::float4(0.f), skr::float4(skr::float3::up(), 0.f));
            const auto aspect_ratio = (float)BACK_BUFFER_WIDTH / (float)BACK_BUFFER_HEIGHT;
            auto proj = skr::float4x4::perspective_fov(
                skr::camera_fov_y_from_x(3.1415926f / 2.f, aspect_ratio),
                aspect_ratio,
                1.f, 1000.f
            );
            auto _view_proj = skr::mul(view, proj);
            auto view_proj = skr::transpose(_view_proj);
            graph->add_render_pass(
                [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                    builder.set_name(u8"gbuffer_pass")
                        .set_pipeline(gbuffer_pipeline)
                        .write(0, gbuffer_color, CGPU_LOAD_ACTION_CLEAR)
                        .write(1, gbuffer_normal, CGPU_LOAD_ACTION_CLEAR)
                        .set_depth_stencil(gbuffer_depth.clear_depth(1.f));
                },
                [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                    cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)native_backbuffer->info->width, (float)native_backbuffer->info->height, 0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->info->width, native_backbuffer->info->height);
                    CGPUBufferId vertex_buffers[5] = {
                        vertex_buffer, vertex_buffer, vertex_buffer,
                        vertex_buffer, instance_buffer
                    };
                    const uint32_t strides[5] = {
                        sizeof(skr_float3_t), sizeof(skr_float2_t),
                        sizeof(uint32_t), sizeof(uint32_t),
                        sizeof(CubeGeometry::InstanceData::world)
                    };
                    const uint32_t offsets[5] = {
                        offsetof(CubeGeometry, g_Positions), offsetof(CubeGeometry, g_TexCoords),
                        offsetof(CubeGeometry, g_Normals), offsetof(CubeGeometry, g_Tangents),
                        offsetof(CubeGeometry::InstanceData, world)
                    };
                    cgpu_render_encoder_bind_index_buffer(stack.encoder, index_buffer, sizeof(uint32_t), 0);
                    cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 5, vertex_buffers, strides, offsets);
                    cgpu_render_encoder_push_constants(stack.encoder, gbuffer_pipeline->root_signature, u8"push_constants", &view_proj);
                    cgpu_render_encoder_draw_indexed_instanced(stack.encoder, 36, 0, 1, 0, 0);
                }
            );
            if (fragmentLightingPass)
            {
                graph->add_render_pass(
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                        builder.set_name(u8"light_pass_fs")
                            .set_pipeline(lighting_pipeline)
                            .read(u8"gbuffer_color", gbuffer_color.read_mip(0, 1))
                            .read(u8"gbuffer_normal", gbuffer_normal)
                            .read(u8"gbuffer_depth", gbuffer_depth)
                            .write(0, composite_buffer, CGPU_LOAD_ACTION_CLEAR);
                    },
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                        cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)native_backbuffer->info->width, (float)native_backbuffer->info->height, 0.f, 1.f);
                        cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->info->width, native_backbuffer->info->height);
                        cgpu_render_encoder_push_constants(stack.encoder, lighting_pipeline->root_signature, u8"push_constants", &lighting_data);
                        cgpu_render_encoder_draw(stack.encoder, 3, 0);
                    }
                );
            }
            else
            {
                graph->add_compute_pass(
                    [=](render_graph::RenderGraph& g, render_graph::ComputePassBuilder& builder) {
                        builder.set_name(u8"light_pass_cs")
                            .set_pipeline(lighting_cs_pipeline)
                            .read(u8"gbuffer_color", gbuffer_color)
                            .read(u8"gbuffer_normal", gbuffer_normal)
                            .read(u8"gbuffer_depth", gbuffer_depth)
                            .readwrite(u8"lighting_output", lighting_buffer);
                    },
                    [=](render_graph::RenderGraph& g, render_graph::ComputePassContext& stack) {
                        cgpu_compute_encoder_push_constants(stack.encoder, lighting_cs_pipeline->root_signature, u8"push_constants", &lighting_cs_data);
                        cgpu_compute_encoder_dispatch(stack.encoder, (uint32_t)ceil(BACK_BUFFER_WIDTH / (float)16), (uint32_t)ceil(BACK_BUFFER_HEIGHT / (float)16), 1);
                    }
                );
                graph->add_render_pass(
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                        builder.set_name(u8"lighting_buffer_blit")
                            .set_pipeline(blit_pipeline)
                            .read(u8"input_color", lighting_buffer)
                            .write(0, composite_buffer, CGPU_LOAD_ACTION_CLEAR);
                    },
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                        cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)native_backbuffer->info->width, (float)native_backbuffer->info->height, 0.f, 1.f);
                        cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->info->width, native_backbuffer->info->height);
                        cgpu_render_encoder_draw(stack.encoder, 3, 0);
                    }
                );
            }
            graph->add_render_pass(
                [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                    builder.set_name(u8"final_blit")
                        .set_pipeline(blit_pipeline)
                        .read(u8"input_color", composite_buffer)
                        .write(0, back_buffer, CGPU_LOAD_ACTION_CLEAR);
                },
                [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                    cgpu_render_encoder_set_viewport(stack.encoder, 0.0f, 0.0f, (float)native_backbuffer->info->width, (float)native_backbuffer->info->height, 0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->info->width, native_backbuffer->info->height);
                    cgpu_render_encoder_draw(stack.encoder, 3, 0);
                }
            );
            imgui_app->render();
        }

        // compile and draw rg
        {
            SkrZoneScopedN("GraphCompile");
            if (frame_index == 0)
                render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_deferred_cs.gv");
            if (frame_index == 6)
                render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_deferred.gv");
        }
        {
            SkrZoneScopedN("GraphExecute");
            auto profiler_index = frame_index % RG_MAX_FRAME_IN_FLIGHT;
            frame_index         = graph->execute(profilers + profiler_index);
        }
        {
            SkrZoneScopedN("CollectGarbage");
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
        }

        // present
        {
            SkrZoneScopedN("Present");
            imgui_render->present_all();
            if (lockFPS) skr_thread_sleep(16);
        }
    }

    // wait rendering done
    cgpu_wait_queue_idle(render_device->get_gfx_queue());

    // clean up
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        profilers[i].finalize();
    }
    render_graph::RenderGraph::destroy(graph);
    imgui_app->shutdown();
    return 0;
}

void RenderGraphDeferredModule::on_unload()
{
    finalize();
}

int main(int argc, char* argv[])
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    {
        FrameMark;
        SkrZoneScopedN("Initialize");
        moduleManager->mount(root.u8string().c_str());
        moduleManager->make_module_graph(u8"RenderGraphDeferred", true);
        moduleManager->init_module_graph(argc, argv);
        moduleManager->destroy_module_graph();
    }
    return 0;
}