#include "triangle_module.wa.c"
#include "../common/utils.h"
#include "platform/thread.h"
#include "utils.h"
#include "math.h"

// WA Engine
THREAD_LOCAL void* wa_watcher;

// Render objects
THREAD_LOCAL SDL_Window* sdl_window;
THREAD_LOCAL ECGpuBackend backend;
THREAD_LOCAL CGpuSurfaceId surface;
THREAD_LOCAL CGpuSwapChainId swapchain;
THREAD_LOCAL uint32_t backbuffer_index;
THREAD_LOCAL CGpuInstanceId instance;
THREAD_LOCAL CGpuAdapterId adapter;
THREAD_LOCAL CGpuDeviceId device;
THREAD_LOCAL CGpuFenceId present_fence;
THREAD_LOCAL CGpuQueueId gfx_queue;
THREAD_LOCAL CGpuRootSignatureId root_sig;
THREAD_LOCAL CGpuRenderPipelineId pipeline;
THREAD_LOCAL CGpuCommandPoolId pool;
THREAD_LOCAL CGpuCommandBufferId cmd;
THREAD_LOCAL CGpuTextureViewId views[3];

const uint32_t* get_vertex_shader()
{
    if (backend == CGPU_BACKEND_VULKAN) return (const uint32_t*)vertex_shader_spirv;
    if (backend == CGPU_BACKEND_D3D12) return (const uint32_t*)vertex_shader_dxil;
    return CGPU_NULLPTR;
}
const uint32_t get_vertex_shader_size()
{
    if (backend == CGPU_BACKEND_VULKAN) return sizeof(vertex_shader_spirv);
    if (backend == CGPU_BACKEND_D3D12) return sizeof(vertex_shader_dxil);
    return 0;
}
const uint32_t* get_fragment_shader()
{
    if (backend == CGPU_BACKEND_VULKAN) return (const uint32_t*)fragment_shader_spirv;
    if (backend == CGPU_BACKEND_D3D12) return (const uint32_t*)fragment_shader_dxil;
    return CGPU_NULLPTR;
}
const uint32_t get_fragment_shader_size()
{
    if (backend == CGPU_BACKEND_VULKAN) return sizeof(fragment_shader_spirv);
    if (backend == CGPU_BACKEND_D3D12) return sizeof(fragment_shader_dxil);
    return 0;
}

void create_render_pipeline()
{
    CGpuShaderLibraryDescriptor vs_desc = {
        .stage = SHADER_STAGE_VERT,
        .name = "VertexShaderLibrary",
        .code = get_vertex_shader(),
        .code_size = get_vertex_shader_size()
    };
    CGpuShaderLibraryDescriptor ps_desc = {
        .name = "FragmentShaderLibrary",
        .stage = SHADER_STAGE_FRAG,
        .code = get_fragment_shader(),
        .code_size = get_fragment_shader_size()
    };
    CGpuShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGpuRootSignatureDescriptor rs_desc = {
        .shaders = ppl_shaders,
        .shader_count = 2
    };
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = { .attribute_count = 0 };
    CGpuRenderPipelineDescriptor rp_desc = {
        .root_signature = root_sig,
        .prim_topology = PRIM_TOPO_TRI_LIST,
        .vertex_layout = &vertex_layout,
        .vertex_shader = &ppl_shaders[0],
        .fragment_shader = &ppl_shaders[1],
        .render_target_count = 1,
        .color_formats = &views[0]->info.format
    };
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

void initialize(void* usrdata)
{
    // WASM
    wa_watcher = watch_wasm();
    // Create window
    SDL_SysWMinfo wmInfo;
    backend = *(ECGpuBackend*)usrdata;

    sdl_window = SDL_CreateWindow(gCGpuBackendNames[backend],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

    // Create instance
    CGpuInstanceDescriptor instance_desc = {
        .backend = backend,
        .enable_debug_layer = true,
        .enable_gpu_based_validation = true,
        .enable_set_name = true
    };
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    DECLARE_ZERO_VLA(CGpuAdapterId, adapters, adapters_count);
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGpuQueueGroupDescriptor G = {
        .queueType = QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    CGpuDeviceDescriptor device_desc = {
        .queueGroups = &G,
        .queueGroupCount = 1
    };
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
    CGpuSwapChainDescriptor descriptor = {
        .presentQueues = &gfx_queue,
        .presentQueuesCount = 1,
        .width = BACK_BUFFER_WIDTH,
        .height = BACK_BUFFER_HEIGHT,
        .surface = surface,
        .imageCount = 3,
        .format = PF_R8G8B8A8_UNORM,
        .enableVsync = true
    };
    swapchain = cgpu_create_swapchain(device, &descriptor);
    // Create views
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        CGpuTextureViewDescriptor view_desc = {
            .texture = swapchain->back_buffers[i],
            .aspects = TVA_COLOR,
            .dims = TEX_DIMENSION_2D,
            .format = swapchain->back_buffers[i]->format,
            .usages = TVU_RTV
        };
        views[i] = cgpu_create_texture_view(device, &view_desc);
    }
    create_render_pipeline();
}

void raster_redraw()
{
    // sync & reset
    cgpu_wait_fences(&present_fence, 1);
    CGpuAcquireNextDescriptor acquire_desc = {
        .fence = present_fence
    };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    const CGpuTextureId back_buffer = swapchain->back_buffers[backbuffer_index];
    const CGpuTextureViewId back_buffer_view = views[backbuffer_index];
    cgpu_reset_command_pool(pool);
    // record
    cgpu_cmd_begin(cmd);
    CGpuColorAttachment screen_attachment = {
        .view = back_buffer_view,
        .load_action = LOAD_ACTION_CLEAR,
        .store_action = STORE_ACTION_STORE,
        .clear_color = fastclear_0000
    };
    CGpuRenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count = SAMPLE_COUNT_1,
        .color_attachments = &screen_attachment,
        .depth_stencil = CGPU_NULLPTR
    };
    CGpuTextureBarrier draw_barrier = {
        .texture = back_buffer,
        .src_state = RESOURCE_STATE_UNDEFINED,
        .dst_state = RESOURCE_STATE_RENDER_TARGET
    };
    CGpuResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);
    CGpuRenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    // get hot-reloadable wasm
    SWAModuleId wa_module = get_available_wasm(wa_watcher);
    if (wa_module != NULL)
    {
        SWAValue params[5];
        params[0].I = (int64_t)cmd;
        params[0].type = SWA_VAL_I64;
        params[1].I = (int64_t)pipeline;
        params[1].type = SWA_VAL_I64;
        params[2].I = (int64_t)rp_encoder;
        params[3].i = back_buffer->width;
        params[3].type = SWA_VAL_I32;
        params[4].i = back_buffer->height;
        params[4].type = SWA_VAL_I32;
        SWAExecDescriptor exec_desc = {
            5, params,
            0, NULL
        };
        const char* res = swa_exec(wa_module, "raster_cmd_record", &exec_desc);
        if (res) printf("[fatal]: %s", res);
    }
    else
    {
        raster_cmd_record(cmd, pipeline,
            rp_encoder,
            back_buffer->width, back_buffer->height);
    }
    CGpuTextureBarrier present_barrier = {
        .texture = back_buffer,
        .src_state = RESOURCE_STATE_RENDER_TARGET,
        .dst_state = RESOURCE_STATE_PRESENT
    };
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    CGpuResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
    cgpu_cmd_end(cmd);
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {
        .cmds = &cmd,
        .cmds_count = 1,
    };
    cgpu_submit_queue(gfx_queue, &submit_desc);
    // present
    cgpu_wait_queue_idle(gfx_queue);
    CGpuQueuePresentDescriptor present_desc = {
        .index = backbuffer_index,
        .swapchain = swapchain,
        .wait_semaphore_count = 0,
        .wait_semaphores = CGPU_NULLPTR
    };
    cgpu_queue_present(gfx_queue, &present_desc);
}

void raster_program()
{
    // Create command objects
    pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
    CGpuCommandBufferDescriptor cmd_desc = { .is_secondary = false };
    cmd = cgpu_create_command_buffer(pool, &cmd_desc);

    while (sdl_window)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    sdl_window = CGPU_NULLPTR;
                }
            }
        }
        raster_redraw();
    }
}

void finalize()
{
    SDL_DestroyWindow(sdl_window);
    // Free wasm engine
    unwatch_wasm(wa_watcher);
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        cgpu_free_texture_view(views[i]);
    }
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_command_buffer(cmd);
    cgpu_free_command_pool(pool);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(root_sig);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

void ProgramMain(void* usrdata)
{
    initialize(usrdata);
    raster_program();
    finalize();
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    // When we support more add them here
    ECGpuBackend backends[] = {
        CGPU_BACKEND_VULKAN
#ifdef CGPU_USE_D3D12
        ,
        CGPU_BACKEND_D3D12
#endif
    };
    void* watcher = watch_source();
#if defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(__wasi__)
    ProgramMain(backends);
#else
    const uint32_t TEST_BACKEND_COUNT = sizeof(backends) / sizeof(ECGpuBackend);
    DECLARE_ZERO_VLA(SThreadHandle, hdls, TEST_BACKEND_COUNT)
    DECLARE_ZERO_VLA(SThreadDesc, thread_descs, TEST_BACKEND_COUNT)
    for (uint32_t i = 0; i < TEST_BACKEND_COUNT; i++)
    {
        thread_descs[i].pFunc = &ProgramMain;
        thread_descs[i].pData = &backends[i];
        skr_init_thread(&thread_descs[i], &hdls[i]);
    }
    for (uint32_t i = 0; i < TEST_BACKEND_COUNT; i++)
    {
        skr_join_thread(hdls[i]);
        skr_destroy_thread(hdls[i]);
    }
#endif
    unwatch_source(watcher);
    SDL_Quit();

    return 0;
}