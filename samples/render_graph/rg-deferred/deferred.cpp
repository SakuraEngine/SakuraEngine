#include "cube_geometry.h"
#include "common/utils.h"
#include "gbuffer_pipeline.h"
#include "lighting_pipeline.h"
#include "blit_pipeline.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"
#include "platform/window.h"
#include "tracy/Tracy.hpp"
#include "pass_profiler.h"
#include "platform/thread.h"
#include "rtm/qvvf.h"

thread_local SWindowHandle window;
thread_local CGPUSurfaceId surface;
thread_local CGPUSwapChainId swapchain;
thread_local uint32_t backbuffer_index;
thread_local CGPUFenceId present_fence;

CubeGeometry::InstanceData CubeGeometry::instance_data;

#if _WIN32
thread_local ECGPUBackend backend = CGPU_BACKEND_D3D12;
#else
thread_local ECGPUBackend backend = CGPU_BACKEND_VULKAN;
#endif

thread_local CGPUInstanceId instance;
thread_local CGPUAdapterId adapter;
thread_local CGPUDeviceId device;
thread_local CGPUQueueId gfx_queue;
thread_local CGPUSamplerId static_sampler;

thread_local CGPUBufferId index_buffer;
thread_local CGPUBufferId vertex_buffer;
thread_local CGPUBufferId instance_buffer;

thread_local CGPURenderPipelineId blit_pipeline;
thread_local CGPURenderPipelineId gbuffer_pipeline;
thread_local CGPURenderPipelineId lighting_pipeline;
thread_local CGPUComputePipelineId lighting_cs_pipeline;

void create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name = true;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count = 1;
    CGPUDeviceDescriptor device_desc = {};
    device_desc.queue_groups = &queue_group_desc;
    device_desc.queue_group_count = 1;
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);
    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    static_sampler = cgpu_create_sampler(device, &sampler_desc);

    // Create swapchain
    surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(window));
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    chain_desc.enable_vsync = false;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
}

void create_resources()
{
    // upload
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = u8"UploadBuffer";
    upload_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices) + sizeof(CubeGeometry::InstanceData);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGPUBufferDescriptor vb_desc = {};
    vb_desc.name = u8"VertexBuffer";
    vb_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(CubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGPUBufferDescriptor ib_desc = {};
    ib_desc.name = u8"IndexBuffer";
    ib_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    ib_desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(CubeGeometry::g_Indices);
    index_buffer = cgpu_create_buffer(device, &ib_desc);
    CGPUBufferDescriptor inb_desc = {};
    inb_desc.name = u8"InstanceBuffer";
    inb_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    inb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    inb_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    inb_desc.size = sizeof(CubeGeometry::InstanceData);
    instance_buffer = cgpu_create_buffer(device, &inb_desc);
    auto pool_desc = CGPUCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc = CGPUCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = CubeGeometry();
        memcpy(upload_buffer->cpu_mapped_address, &geom, upload_buffer_desc.size);
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
    // wvp
    const auto quat = rtm::quat_from_euler_rh(
        rtm::scalar_deg_to_rad(0.f),
        rtm::scalar_deg_to_rad(0.f),
        rtm::scalar_deg_to_rad(0.f));
    const rtm::vector4f translation = rtm::vector_set(0.f, 0.f, 0.f, 0.f);
    const rtm::vector4f scale = rtm::vector_set(2.f, 2.f, 2.f, 0.f);
    const rtm::qvvf transform = rtm::qvv_set(quat, translation, scale);
    const rtm::matrix4x4f matrix = rtm::matrix_cast(rtm::matrix_from_qvv(transform));
    CubeGeometry::instance_data.world = *(skr_float4x4_t*)&matrix;
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices),
        &CubeGeometry::instance_data, sizeof(CubeGeometry::InstanceData));
    }
    CGPUBufferToBufferTransfer istb_cpy = {};
    istb_cpy.dst = instance_buffer;
    istb_cpy.dst_offset = 0;
    istb_cpy.src = upload_buffer;
    istb_cpy.src_offset = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    istb_cpy.size = sizeof(CubeGeometry::instance_data);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &istb_cpy);
    CGPUBufferBarrier barriers[3] = {};
    CGPUBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = vertex_buffer;
    vb_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGPUBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = index_buffer;
    ib_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = CGPU_RESOURCE_STATE_INDEX_BUFFER;
    CGPUBufferBarrier& ist_barrier = barriers[2];
    ist_barrier.buffer = instance_buffer;
    ist_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    ist_barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGPUResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 3;
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
}

void create_render_pipeline()
{
    gbuffer_pipeline = create_gbuffer_render_pipeline(device);
    lighting_pipeline = create_lighting_render_pipeline(device,
    static_sampler, (ECGPUFormat)swapchain->back_buffers[0]->format);
    lighting_cs_pipeline = create_lighting_compute_pipeline(device);
    blit_pipeline = create_blit_render_pipeline(device, static_sampler,
    (ECGPUFormat)swapchain->back_buffers[0]->format);
}

void finalize()
{
    // Free cgpu objects
    cgpu_wait_fences(&present_fence, 1);
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
    skr_float2_t viewportSize = { BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT };
    skr_float2_t viewportOrigin = { 0, 0 };
};
static LightingCSPushConstants lighting_cs_data = {};
bool fragmentLightingPass = true;
bool lockFPS = true;
bool DPIAware = false;

#include "runtime_module.h"

int main(int argc, char* argv[])
{
    DPIAware = skr_runtime_is_dpi_aware();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return -1;
    SWindowDescroptor window_desc = {};
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;
    window_desc.height = BACK_BUFFER_HEIGHT;
    window_desc.width = BACK_BUFFER_WIDTH;
    window = skr_create_window(gCGPUBackendNames[backend], &window_desc);
    create_api_objects();
    create_resources();
    create_render_pipeline();
    // initialize
    namespace render_graph = skr::render_graph;
    auto graph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
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
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        const char8_t* font_path = u8"./../resources/font/SourceSansPro-Regular.ttf";
        uint32_t *font_bytes, font_length;
        read_bytes(font_path, &font_bytes, &font_length);
        float dpi_scaling = 1.f;
        if (!DPIAware)
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(1.f / dpi_scaling);
            ImGui::GetIO().FontGlobalScale = 1.f / dpi_scaling;
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
    }
    uint32_t *im_vs_bytes, im_vs_length;
    read_shader_bytes(u8"imgui_vertex", &im_vs_bytes, &im_vs_length,
    device->adapter->instance->backend);
    uint32_t *im_fs_bytes, im_fs_length;
    read_shader_bytes(u8"imgui_fragment", &im_fs_bytes, &im_fs_length,
    device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = u8"imgui_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = im_vs_bytes;
    vs_desc.code_size = im_vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = u8"imgui_fragment_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = im_fs_bytes;
    fs_desc.code_size = im_fs_length;
    CGPUShaderLibraryId imgui_vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId imgui_fs = cgpu_create_shader_library(device, &fs_desc);
    free(im_vs_bytes);
    free(im_fs_bytes);
    RenderGraphImGuiDescriptor imgui_graph_desc = {};
    imgui_graph_desc.render_graph = graph;
    imgui_graph_desc.backbuffer_format = (ECGPUFormat)swapchain->back_buffers[backbuffer_index]->format;
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = u8"main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = u8"main";
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

            if (event.type == SDL_WINDOWEVENT)
            {
                Uint8 window_event = event.window.event;
                if (window_event == SDL_WINDOWEVENT_CLOSE || window_event == SDL_WINDOWEVENT_MOVED || window_event == SDL_WINDOWEVENT_RESIZED)
                if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(event.window.windowID)))
                {
                    if (window_event == SDL_WINDOWEVENT_CLOSE)
                        viewport->PlatformRequestClose = true;
                    if (window_event == SDL_WINDOWEVENT_MOVED)
                        viewport->PlatformRequestMove = true;
                    if (window_event == SDL_WINDOWEVENT_RESIZED)
                        viewport->PlatformRequestResize = true;
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
                auto profiler_index = (frame_index - 1) % RG_MAX_FRAME_IN_FLIGHT;
                auto&& profiler = profilers[profiler_index];
                if (profiler.times_ms.size() == profiler.query_names.size())
                {
                    // text
                    ImGui::Text("frame: %d(%d frames before)",
                    profiler.frame_index,
                    frame_index - profiler.frame_index);
                    float total_ms = 0.f;
                    for (uint32_t i = 1; i < profiler.times_ms.size(); i++)
                    {
                        auto text = profiler.query_names[i];
                        text = text.append(u8": %.4f ms");
                        ImGui::Text(text.c_str(), profiler.times_ms[i]);
                        total_ms += profiler.times_ms[i];
                    }
                    ImGui::Text("GPU Time: %f(ms)", total_ms);
                    // plot
                    auto max_scale = eastl::max_element(profiler.times_ms.begin(), profiler.times_ms.end());
                    auto min_scale = eastl::max_element(profiler.times_ms.begin(), profiler.times_ms.end());
                    (void)min_scale;
                    ImVec2 size = { 200, 200 };
                    ImGui::PlotHistogram("##ms",
                    profiler.times_ms.data() + 1,
                    (int)profiler.times_ms.size() - 1,
                    0, NULL,
                    0.0001f, *max_scale * 1.1f, size);
                }
            }
            ImGui::End();
        }
        {
            // acquire frame
            ZoneScopedN("AcquireFrame");
            cgpu_wait_fences(&present_fence, 1);
            CGPUAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence = present_fence;
            backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        {
            ZoneScopedN("GraphSetup");
            // render graph setup & compile & exec
            auto back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"backbuffer")
                .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
                .allow_render_target();
            });
            render_graph::TextureHandle composite_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"composite_buffer")
                .extent(native_backbuffer->width, native_backbuffer->height)
                .format((ECGPUFormat)native_backbuffer->format)
                .owns_memory()
                .allow_render_target();
            });
            auto gbuffer_color = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"gbuffer_color")
                .extent(native_backbuffer->width, native_backbuffer->height)
                .format(gbuffer_formats[0])
                .owns_memory()
                .allow_render_target();
            });
            auto gbuffer_depth = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"gbuffer_depth")
                .extent(native_backbuffer->width, native_backbuffer->height)
                .format(gbuffer_depth_format)
                .owns_memory()
                .allow_depth_stencil();
            });
            auto gbuffer_normal = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"gbuffer_normal")
                .extent(native_backbuffer->width, native_backbuffer->height)
                .format(gbuffer_formats[1])
                .owns_memory()
                .allow_render_target();
            });
            auto lighting_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"lighting_buffer")
                .extent(native_backbuffer->width, native_backbuffer->height)
                .format(lighting_buffer_format)
                .owns_memory()
                .allow_readwrite();
            });
            // camera
            auto view = rtm::look_at_matrix(
                { 0.f, 2.5f, 2.5f } /*eye*/,
                { 0.f, 0.f, 0.f } /*at*/,
                { 0.f, 1.f, 0.f } /*up*/);
            auto proj = rtm::perspective_fov(
                3.1415926f / 2.f,
                (float)BACK_BUFFER_WIDTH / (float)BACK_BUFFER_HEIGHT,
                1.f, 1000.f);
            auto view_proj = rtm::matrix_mul(view, proj);
            graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name(u8"gbuffer_pass")
                .set_pipeline(gbuffer_pipeline)
                .write(0, gbuffer_color, CGPU_LOAD_ACTION_CLEAR)
                .write(1, gbuffer_normal, CGPU_LOAD_ACTION_CLEAR)
                .set_depth_stencil(gbuffer_depth.clear_depth(1.f));
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                0.0f, 0.0f,
                (float)native_backbuffer->width, (float)native_backbuffer->height,
                0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->width, native_backbuffer->height);
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
            });
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
                    cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)native_backbuffer->width, (float)native_backbuffer->height,
                    0.f, 1.f);
                    cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, native_backbuffer->width, native_backbuffer->height);
                    cgpu_render_encoder_push_constants(stack.encoder, lighting_pipeline->root_signature, u8"push_constants", &lighting_data);
                    cgpu_render_encoder_draw(stack.encoder, 6, 0);
                });
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
                        cgpu_compute_encoder_push_constants(stack.encoder,
                            lighting_cs_pipeline->root_signature, u8"push_constants", &lighting_cs_data);
                        cgpu_compute_encoder_dispatch(stack.encoder,
                            (uint32_t)ceil(BACK_BUFFER_WIDTH / (float)16),
                            (uint32_t)ceil(BACK_BUFFER_HEIGHT / (float)16),
                            1);
                    });
                graph->add_render_pass(
                    [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                        builder.set_name(u8"lighting_buffer_blit")
                            .set_pipeline(blit_pipeline)
                            .read(u8"input_color", lighting_buffer)
                            .write(0, composite_buffer, CGPU_LOAD_ACTION_CLEAR);
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
            render_graph_imgui_add_render_pass(graph, composite_buffer, CGPU_LOAD_ACTION_LOAD);
            graph->add_render_pass(
                [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                    builder.set_name(u8"final_blit")
                        .set_pipeline(blit_pipeline)
                        .read(u8"input_color", composite_buffer)
                        .write(0, back_buffer, CGPU_LOAD_ACTION_CLEAR);
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
                    builder.set_name(u8"present_pass")
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
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
        }
        {
            ZoneScopedN("Present");
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            render_graph_imgui_present_sub_viewports();

            if (lockFPS) skr_thread_sleep(16);
        }
    }
    cgpu_wait_queue_idle(gfx_queue);
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