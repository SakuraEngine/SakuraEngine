#include "cube_geometry.h"
#include "../common/utils.h"
#include "gbuffer_pipeline.h"
#include "lighting_pipeline.h"
#include "blit_pipeline.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "platform/window.h"
#include "tracy/Tracy.hpp"
#include "pass_profiler.h"

thread_local SWindowHandle window;
thread_local CGpuSurfaceId surface;
thread_local CGpuSwapChainId swapchain;
thread_local uint32_t backbuffer_index;
thread_local CGpuFenceId present_fence;

CubeGeometry::InstanceData CubeGeometry::instance_data;

#if _WIN32
thread_local ECGpuBackend backend = CGPU_BACKEND_VULKAN;
#else
thread_local ECGpuBackend backend = CGPU_BACKEND_VULKAN;
#endif

thread_local CGpuInstanceId instance;
thread_local CGpuAdapterId adapter;
thread_local CGpuDeviceId device;
thread_local CGpuQueueId gfx_queue;
thread_local CGpuSamplerId static_sampler;

thread_local CGpuBufferId index_buffer;
thread_local CGpuBufferId vertex_buffer;
thread_local CGpuBufferId instance_buffer;

thread_local CGpuRenderPipelineId blit_pipeline;
thread_local CGpuRenderPipelineId gbuffer_pipeline;
thread_local CGpuRenderPipelineId lighting_pipeline;
thread_local CGpuComputePipelineId lighting_cs_pipeline;

void create_api_objects()
{
    // Create instance
    CGpuInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGpuAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGpuQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queueType = QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queueCount = 1;
    CGpuDeviceDescriptor device_desc = {};
    device_desc.queueGroups = &queue_group_desc;
    device_desc.queueGroupCount = 1;
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);
    // Sampler
    CGpuSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CMP_NEVER;
    static_sampler = cgpu_create_sampler(device, &sampler_desc);

    // Create swapchain
    surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(window));
    CGpuSwapChainDescriptor chain_desc = {};
    chain_desc.presentQueues = &gfx_queue;
    chain_desc.presentQueuesCount = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = PF_R8G8B8A8_UNORM;
    chain_desc.enableVsync = false;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
}

void create_resources()
{
    // upload
    CGpuBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "UploadBuffer";
    upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = RT_NONE;
    upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices) + sizeof(CubeGeometry::InstanceData);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGpuBufferDescriptor vb_desc = {};
    vb_desc.name = "VertexBuffer";
    vb_desc.flags = BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = RT_VERTEX_BUFFER;
    vb_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(CubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGpuBufferDescriptor ib_desc = {};
    ib_desc.name = "IndexBuffer";
    ib_desc.flags = BCF_OWN_MEMORY_BIT;
    ib_desc.descriptors = RT_INDEX_BUFFER;
    ib_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(CubeGeometry::g_Indices);
    index_buffer = cgpu_create_buffer(device, &ib_desc);
    CGpuBufferDescriptor inb_desc = {};
    inb_desc.name = "InstanceBuffer";
    inb_desc.flags = BCF_OWN_MEMORY_BIT;
    inb_desc.descriptors = RT_VERTEX_BUFFER;
    inb_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    inb_desc.size = sizeof(CubeGeometry::InstanceData);
    instance_buffer = cgpu_create_buffer(device, &inb_desc);
    auto pool_desc = CGpuCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc = CGpuCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = CubeGeometry();
        memcpy(upload_buffer->cpu_mapped_address, &geom, upload_buffer_desc.size);
    }
    cgpu_cmd_begin(cpy_cmd);
    CGpuBufferToBufferTransfer vb_cpy = {};
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
    CGpuBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst = index_buffer;
    ib_cpy.dst_offset = 0;
    ib_cpy.src = upload_buffer;
    ib_cpy.src_offset = sizeof(CubeGeometry);
    ib_cpy.size = sizeof(CubeGeometry::g_Indices);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &ib_cpy);
    // wvp
    auto world = smath::make_transform(
        { 0.f, 0.f, 0.f },                        // translation
        2 * sakura::math::Vector3f::vector_one(), // scale
        sakura::math::Quaternion::identity()      // quat
    );
    CubeGeometry::instance_data.world = world;
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices),
            &CubeGeometry::instance_data, sizeof(CubeGeometry::InstanceData));
    }
    CGpuBufferToBufferTransfer istb_cpy = {};
    istb_cpy.dst = instance_buffer;
    istb_cpy.dst_offset = 0;
    istb_cpy.src = upload_buffer;
    istb_cpy.src_offset = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    istb_cpy.size = sizeof(CubeGeometry::instance_data);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &istb_cpy);
    CGpuBufferBarrier barriers[3] = {};
    CGpuBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = vertex_buffer;
    vb_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGpuBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = index_buffer;
    ib_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = RESOURCE_STATE_INDEX_BUFFER;
    CGpuBufferBarrier& ist_barrier = barriers[2];
    ist_barrier.buffer = instance_buffer;
    ist_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    ist_barrier.dst_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGpuResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 3;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
    cgpu_cmd_end(cpy_cmd);
    CGpuQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cmd_pool);
}

void create_render_pipeline()
{
    gbuffer_pipeline = create_gbuffer_render_pipeline(device);
    lighting_pipeline = create_lighting_render_pipeline(device,
        static_sampler, (ECGpuFormat)swapchain->back_buffers[0]->format);
    lighting_cs_pipeline = create_lighting_compute_pipeline(device);
    blit_pipeline = create_blit_render_pipeline(device, static_sampler,
        (ECGpuFormat)swapchain->back_buffers[0]->format);
}

void finalize()
{
    // Free cgpu objects
    cgpu_free_fence(present_fence);
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
    cgpu_free_buffer(instance_buffer);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    free_pipeline_and_signature(gbuffer_pipeline);
    free_pipeline_and_signature(lighting_pipeline);
    free_pipeline_and_signature(lighting_cs_pipeline);
    free_pipeline_and_signature(blit_pipeline);
    cgpu_free_sampler(static_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

struct LightingPushConstants {
    int bFlipUVX = 0;
    int bFlipUVY = 0;
};
static LightingPushConstants lighting_data = {};
struct LightingCSPushConstants {
    smath::Vector2f viewportSize = { BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT };
    smath::Vector2f viewportOrigin = { 0, 0 };
};
static LightingCSPushConstants lighting_cs_data = {};
const bool fragmentLightingPass = false;
bool DPIAware = false;

#ifdef SAKURA_RUNTIME_OS_WINDOWS
    #include <shellscalingapi.h>
#endif

int main(int argc, char* argv[])
{
#ifdef SAKURA_RUNTIME_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    DPIAware = true;
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    SWindowDescroptor window_desc = {};
    window_desc.centered = true;
    window_desc.resizable = true;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window(gCGpuBackendNames[backend], &window_desc);
    create_api_objects();
    create_resources();
    create_render_pipeline();
    // initialize
    namespace render_graph = sakura::render_graph;
    auto graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(device)
                .with_gfx_queue(gfx_queue)
                .enable_memory_aliasing();
        });
    PassProfiler profilers[RG_MAX_FRAME_IN_FLIGHT];
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        profilers[i].initialize(device);
    }
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        const char* font_path = "./../resources/font/SourceSansPro-Regular.ttf";
        uint32_t *font_bytes, font_length;
        read_bytes(font_path, (char**)&font_bytes, &font_length);
        float dpi_scaling = 1.f;
        if (!DPIAware)
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(1.f / dpi_scaling);
            ImGui::GetIO().FontGlobalScale = 0.5f;
        }
        else
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(dpi_scaling);
        }
        ImFontConfig cfg = {};
        cfg.SizePixels = 16.f * dpi_scaling;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.PixelSnapH = true;
        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_bytes,
            font_length, cfg.SizePixels, &cfg);
        ImGui::GetIO().Fonts->Build();
        free(font_bytes);
        style.AntiAliasedFill = true;
        style.AntiAliasedLines = true;
        style.AntiAliasedLinesUseTex = true;
    }
    uint32_t *im_vs_bytes, im_vs_length;
    read_shader_bytes("imgui_vertex", &im_vs_bytes, &im_vs_length,
        device->adapter->instance->backend);
    uint32_t *im_fs_bytes, im_fs_length;
    read_shader_bytes("imgui_fragment", &im_fs_bytes, &im_fs_length,
        device->adapter->instance->backend);
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "imgui_vertex_shader";
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.code = im_vs_bytes;
    vs_desc.code_size = im_vs_length;
    CGpuShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "imgui_fragment_shader";
    fs_desc.stage = SHADER_STAGE_FRAG;
    fs_desc.code = im_fs_bytes;
    fs_desc.code_size = im_fs_length;
    CGpuShaderLibraryId imgui_vs = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId imgui_fs = cgpu_create_shader_library(device, &fs_desc);
    free(im_vs_bytes);
    free(im_fs_bytes);
    RenderGraphImGuiDescriptor imgui_graph_desc = {};
    imgui_graph_desc.render_graph = graph;
    imgui_graph_desc.backbuffer_format = (ECGpuFormat)swapchain->back_buffers[backbuffer_index]->format;
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = "main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = "main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = static_sampler;
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);
    // loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        auto sdl_window = (SDL_Window*)window;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }
            }
        }
        ZoneScopedN("FrameTime");
        static uint64_t frame_index = 0;
        {
            ZoneScopedN("ImGui");
            auto& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(
                (float)swapchain->back_buffers[0]->width,
                (float)swapchain->back_buffers[0]->height);
            skr_imgui_new_frame(window, 1.f / 60.f);
            ImGui::Begin(u8"RenderGraphProfile");
            if (frame_index > RG_MAX_FRAME_IN_FLIGHT)
            {
                auto profiler_index = (frame_index - 1) % RG_MAX_FRAME_IN_FLIGHT;
                auto&& profiler = profilers[profiler_index];
                // text
                ImGui::Text("frame: %d(%d frames before)",
                    profiler.frame_index,
                    frame_index - profiler.frame_index);
                for (uint32_t i = 1; i < profiler.times_ms.size(); i++)
                {
                    auto text = profiler.query_names[i];
                    text = text.append(": %.4f ms");
                    ImGui::Text(text.c_str(), profiler.times_ms[i]);
                }
                // plot
                auto max_scale = eastl::max_element(profiler.times_ms.begin(), profiler.times_ms.end());
                auto min_scale = eastl::max_element(profiler.times_ms.begin(), profiler.times_ms.end());
                ImVec2 size = { 200, 200 };
                ImGui::PlotHistogram("##ms",
                    profiler.times_ms.data() + 1,
                    (int)profiler.times_ms.size() - 1,
                    0, NULL,
                    0.0001f, *max_scale * 1.1f, size);
            }
            ImGui::End();
        }
        {
            // acquire frame
            ZoneScopedN("AcquireFrame");
            cgpu_wait_fences(&present_fence, 1);
            CGpuAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence = present_fence;
            backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        CGpuTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        {
            ZoneScopedN("GraphSetup");
            // render graph setup & compile & exec
            auto back_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("backbuffer")
                        .import(native_backbuffer, RESOURCE_STATE_UNDEFINED)
                        .allow_render_target();
                });
            render_graph::TextureHandle composite_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("composite_buffer")
                        .extent(native_backbuffer->width, native_backbuffer->height)
                        .format((ECGpuFormat)native_backbuffer->format)
                        .owns_memory()
                        .allow_render_target();
                });
            auto gbuffer_color = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("gbuffer_color")
                        .extent(native_backbuffer->width, native_backbuffer->height)
                        .format(gbuffer_formats[0])
                        .owns_memory()
                        .allow_render_target();
                });
            auto gbuffer_depth = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("gbuffer_depth")
                        .extent(native_backbuffer->width, native_backbuffer->height)
                        .format(gbuffer_depth_format)
                        .owns_memory()
                        .allow_depth_stencil();
                });
            auto gbuffer_normal = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("gbuffer_normal")
                        .extent(native_backbuffer->width, native_backbuffer->height)
                        .format(gbuffer_formats[1])
                        .owns_memory()
                        .allow_render_target();
                });
            auto lighting_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("lighting_buffer")
                        .extent(native_backbuffer->width, native_backbuffer->height)
                        .format(lighting_buffer_format)
                        .owns_memory()
                        .allow_readwrite();
                });
            // camera
            auto view = smath::look_at_matrix(
                { 0.f, 2.5f, 2.5f } /*eye*/,
                { 0.f, 0.f, 0.f } /*at*/);
            auto proj = smath::perspective_fov(
                3.1415926f / 2.f,
                (float)BACK_BUFFER_HEIGHT / (float)BACK_BUFFER_WIDTH,
                1.f, 1000.f);
            auto view_proj = smath::multiply(view, proj);
            graph->add_render_pass(
                [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                    builder.set_name("gbuffer_pass")
                        .set_pipeline(gbuffer_pipeline)
                        .write(0, gbuffer_color, LOAD_ACTION_CLEAR)
                        .write(1, gbuffer_normal, LOAD_ACTION_CLEAR)
                        .set_depth_stencil(gbuffer_depth);
                },
                [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                    cgpu_render_encoder_set_viewport(stack.encoder,
                        0.0f, 0.0f,
                        (float)native_backbuffer->width, (float)native_backbuffer->height,
                        0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->width, native_backbuffer->height);
                    CGpuBufferId vertex_buffers[5] = {
                        vertex_buffer, vertex_buffer, vertex_buffer,
                        vertex_buffer, instance_buffer
                    };
                    const uint32_t strides[5] = {
                        sizeof(sakura::math::Vector3f), sizeof(sakura::math::Vector2f),
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
                    cgpu_render_encoder_push_constants(stack.encoder, gbuffer_pipeline->root_signature, "root_constants", &view_proj);
                    cgpu_render_encoder_draw_indexed_instanced(stack.encoder, 36, 0, 1, 0, 0);
                });
            if (fragmentLightingPass)
            {
                graph->add_render_pass(
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                        builder.set_name("light_pass_fs")
                            .set_pipeline(lighting_pipeline)
                            .read("gbuffer_color", gbuffer_color.read_mip(0, 1))
                            .read("gbuffer_normal", gbuffer_normal)
                            .read("gbuffer_depth", gbuffer_depth)
                            .write(0, composite_buffer, LOAD_ACTION_CLEAR);
                    },
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                        cgpu_render_encoder_set_viewport(stack.encoder,
                            0.0f, 0.0f,
                            (float)native_backbuffer->width, (float)native_backbuffer->height,
                            0.f, 1.f);
                        cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->width, native_backbuffer->height);
                        cgpu_render_encoder_push_constants(stack.encoder, lighting_pipeline->root_signature, "root_constants", &lighting_data);
                        cgpu_render_encoder_draw(stack.encoder, 6, 0);
                    });
            }
            else
            {
                graph->add_compute_pass(
                    [=](render_graph::RenderGraph& g, render_graph::ComputePassBuilder& builder) {
                        builder.set_name("light_pass_cs")
                            .set_pipeline(lighting_cs_pipeline)
                            .read("gbuffer_color", gbuffer_color)
                            .read("gbuffer_normal", gbuffer_normal)
                            .read("gbuffer_depth", gbuffer_depth)
                            .readwrite("lighting_output", lighting_buffer);
                    },
                    [=](render_graph::RenderGraph& g, render_graph::ComputePassContext& stack) {
                        cgpu_compute_encoder_push_constants(stack.encoder,
                            lighting_cs_pipeline->root_signature, "root_constants", &lighting_cs_data);
                        cgpu_compute_encoder_dispatch(stack.encoder,
                            (uint32_t)ceil(BACK_BUFFER_WIDTH / (float)16),
                            (uint32_t)ceil(BACK_BUFFER_HEIGHT / (float)16),
                            1);
                    });
                graph->add_render_pass(
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                        builder.set_name("lighting_buffer_blit")
                            .set_pipeline(blit_pipeline)
                            .read("input_color", lighting_buffer)
                            .write(0, composite_buffer, LOAD_ACTION_CLEAR);
                    },
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                        cgpu_render_encoder_set_viewport(stack.encoder,
                            0.0f, 0.0f,
                            (float)native_backbuffer->width, (float)native_backbuffer->height,
                            0.f, 1.f);
                        cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->width, native_backbuffer->height);
                        cgpu_render_encoder_draw(stack.encoder, 6, 0);
                    });
            }
            render_graph_imgui_add_render_pass(graph,
                composite_buffer, LOAD_ACTION_LOAD);
            graph->add_render_pass(
                [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                    builder.set_name("final_blit")
                        .set_pipeline(blit_pipeline)
                        .read("input_color", composite_buffer)
                        .write(0, back_buffer, LOAD_ACTION_CLEAR);
                },
                [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                    cgpu_render_encoder_set_viewport(stack.encoder,
                        0.0f, 0.0f,
                        (float)native_backbuffer->width, (float)native_backbuffer->height,
                        0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->width, native_backbuffer->height);
                    cgpu_render_encoder_draw(stack.encoder, 6, 0);
                });
            graph->add_present_pass(
                [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                    builder.set_name("present_pass")
                        .swapchain(swapchain, backbuffer_index)
                        .texture(back_buffer, true);
                });
        }
        {
            ZoneScopedN("GraphCompile");
            graph->compile();
            if (frame_index == 0)
                render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_deferred_cs.gv");
            if (frame_index == 6)
                render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_deferred.gv");
        }
        {
            ZoneScopedN("GraphExecute");
            auto profiler_index = frame_index % RG_MAX_FRAME_IN_FLIGHT;
            frame_index = graph->execute(profilers + profiler_index);
        }
        // present
        {
            ZoneScopedN("CollectGarbage");
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT);
        }
        {
            ZoneScopedN("Present");
            CGpuQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
        }
    }
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        profilers[i].finalize();
    }
    render_graph::RenderGraph::destroy(graph);
    render_graph_imgui_finalize();
    // clean up
    finalize();
    skr_free_window(window);
    SDL_Quit();
    return 0;
}