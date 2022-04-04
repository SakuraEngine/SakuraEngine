#include "../common/utils.h"
#include "render_graph/frontend/render_graph.hpp"

thread_local SDL_Window* sdl_window;
thread_local SDL_SysWMinfo wmInfo;
thread_local CGpuSurfaceId surface;
thread_local CGpuSwapChainId swapchain;
thread_local uint32_t backbuffer_index;
thread_local CGpuFenceId present_fence;

#if _WINDOWS
thread_local ECGpuBackend backend = CGPU_BACKEND_D3D12;
#else
thread_local ECGpuBackend backend = CGPU_BACKEND_VULKAN;
#endif

thread_local CGpuInstanceId instance;
thread_local CGpuAdapterId adapter;
thread_local CGpuDeviceId device;
thread_local CGpuQueueId gfx_queue;

thread_local CGpuRootSignatureId root_sig;
thread_local CGpuRenderPipelineId pipeline;

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

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGpuSwapChainDescriptor chain_desc = {};
    chain_desc.presentQueues = &gfx_queue;
    chain_desc.presentQueuesCount = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 3;
    chain_desc.format = PF_R8G8B8A8_UNORM;
    chain_desc.enableVsync = true;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
}

void create_render_pipeline()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-triangle/vertex_shader", &vs_bytes, &vs_length, backend);
    read_shader_bytes("rg-triangle/fragment_shader", &fs_bytes, &fs_length, backend);
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGpuShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "FragmentShaderLibrary";
    ps_desc.stage = SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGpuShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    auto backend_format = (ECGpuFormat)swapchain->back_buffers[0]->format;
    rp_desc.color_formats = &backend_format;
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

void finalize()
{
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(root_sig);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    sdl_window = SDL_CreateWindow(gCGpuBackendNames[backend],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);
    create_api_objects();
    create_render_pipeline();
    // initialize
    namespace render_graph = sakura::render_graph;
    auto graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(device)
                .with_gfx_queue(gfx_queue);
        });
    // loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
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
        // acquire frame
        cgpu_wait_fences(&present_fence, 1);
        CGpuAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = present_fence;
        backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        // render graph setup & compile & exec
        CGpuTextureId to_import = swapchain->back_buffers[backbuffer_index];
        auto back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name("backbuffer")
                    .import(to_import, RESOURCE_STATE_PRESENT)
                    .allow_render_target();
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name("color_pass")
                    .set_pipeline(pipeline)
                    .write(0, back_buffer, LOAD_ACTION_CLEAR);
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassStack& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)to_import->width / 3, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, to_import->width, to_import->height);
                cgpu_render_encoder_draw(stack.encoder, 3, 0);
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name("color_pass2")
                    .set_pipeline(pipeline)
                    .write(0, back_buffer, LOAD_ACTION_LOAD);
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassStack& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    2 * (float)to_import->width / 3, 0.0f,
                    (float)to_import->width / 3, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, to_import->width, to_import->height);
                cgpu_render_encoder_draw(stack.encoder, 3, 0);
            });
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name("present")
                    .swapchain(swapchain, backbuffer_index)
                    .texture(back_buffer, true);
            });
        graph->compile();
        const auto frame_index = graph->execute();
        // present
        cgpu_wait_queue_idle(gfx_queue);
        CGpuQueuePresentDescriptor present_desc = {};
        present_desc.index = backbuffer_index;
        present_desc.swapchain = swapchain;
        cgpu_queue_present(gfx_queue, &present_desc);
        if (frame_index == 0)
            render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_demo.gv");
    }
    render_graph::RenderGraph::destroy(graph);
    // clean up
    finalize();
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}