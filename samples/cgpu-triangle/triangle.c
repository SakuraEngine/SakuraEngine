#include "window_helper.h"
#include "cgpu/cgpu_config.h"
#include "math.h"
#include "cgpu/api.h"

_Thread_local SDL_Window* sdl_window;
_Thread_local CGpuSurfaceId surface;
_Thread_local CGpuSwapChainId swapchain;
_Thread_local uint32_t backbuffer_index;
_Thread_local CGpuInstanceId instance;
_Thread_local CGpuAdapterId adapter;
_Thread_local CGpuDeviceId device;
_Thread_local CGpuFenceId present_fence;
_Thread_local CGpuQueueId gfx_queue;
_Thread_local CGpuCommandPoolId pool;
_Thread_local CGpuCommandBufferId cmd;

void initialize(void* usrdata)
{
    // Create window
    SDL_SysWMinfo wmInfo;
    sdl_window = SDL_CreateWindow("title",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

    ECGpuBackend backend = *(ECGpuBackend*)usrdata;
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
        .queueType = ECGpuQueueType_Graphics,
        .queueCount = 1
    };
    CGpuDeviceDescriptor device_desc = {
        .queueGroups = &G,
        .queueGroupCount = 1
    };
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
    present_fence = cgpu_create_fence(device);

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(nswin);
    surface = cgpu_surface_from_ns_view(device, wmInfo.info.cocoa.window);
#endif
    DECLARE_ZERO(CGpuSwapChainDescriptor, descriptor)
    descriptor.presentQueues = &gfx_queue;
    descriptor.presentQueuesCount = 1;
    descriptor.surface = surface;
    descriptor.imageCount = 3;
    descriptor.format = PF_R8G8B8A8_UNORM;
    descriptor.enableVsync = true;
    swapchain = cgpu_create_swapchain(device, &descriptor);
}

void raster_redraw()
{
    // sync & reset
    cgpu_wait_fences(&present_fence, 1);
    CGpuAcquireNextDescriptor acquire_desc = {
        .fence = present_fence
    };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    cgpu_reset_command_pool(pool);
    // record
    cgpu_cmd_begin(cmd);
    /*
    CGpuClearValue fast_clear = { 0 };
    CGpuColorAttachment screen_attachment = {
        .view = swapchain->views[backbuffer_index],
        .load_action = LA_CLEAR,
        .store_action = SA_Store,
        .clear_color = fast_clear
    };
    CGpuRenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count = SC_1,
        .color_attachments = &screen_attachment,
        .depth_stencil = CGPU_NULLPTR
    };
    CGpuRenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);

    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    */
    CGpuTextureBarrier texture_barrier = {
        .texture = swapchain->views[backbuffer_index],
        .src_state = RS_UNDEFINED,
        .dst_state = RS_PRESENT
    };
    CGpuResourceBarrierDescriptor barrier_desc = { .texture_barriers = &texture_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc);
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
            // olog::Info(u"event type: {}  windowID: {}"_o, (int)event.type, (int)event.window.windowID);
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
    SDL_Quit();
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_fence(present_fence);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_command_buffer(cmd);
    cgpu_free_command_pool(pool);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

int main()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;

    ECGpuBackend backend = ECGpuBackend_VULKAN;
    initialize(&backend);
    raster_program();
    finalize();

    return 0;
}