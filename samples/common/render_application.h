#pragma once
#include "./utils.h"
#include "platform/window.h"

#if _WIN32
static const ECGPUBackend platform_default_backend = CGPU_BACKEND_D3D12;
#else
static const ECGPUBackend platform_default_backend = CGPU_BACKEND_VULKAN;
#endif

typedef struct render_application_t 
{
    const char* window_title;
    SWindowHandle window_handle;
    SDL_SysWMinfo wmInfo;
    uint32_t window_width;
    uint32_t window_height;
    ECGPUBackend backend;

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
    CGPUSurfaceId surface;
    CGPUSwapChainId swapchain;
    uint32_t backbuffer_index;
    CGPUQueueId gfx_queue;
    CGPUFenceId present_fence;
} render_application_t;

inline int app_create_gfx_objects(render_application_t* pApp)
{
    // Create instance
    DECLARE_ZERO(CGPUInstanceDescriptor, instance_desc);
    instance_desc.backend = pApp->backend;
    instance_desc.enable_debug_layer = false;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name = true;
    pApp->instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(pApp->instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(pApp->instance, adapters, &adapters_count);
    pApp->adapter = adapters[0];

    // Create device
    DECLARE_ZERO(CGPUQueueGroupDescriptor, queue_group_desc);
    queue_group_desc.queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count = 1;
    DECLARE_ZERO(CGPUDeviceDescriptor, device_desc);
    device_desc.queue_groups = &queue_group_desc;
    device_desc.queue_group_count = 1;
    pApp->device = cgpu_create_device(pApp->adapter, &device_desc);
    pApp->gfx_queue = cgpu_get_queue(pApp->device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    pApp->present_fence = cgpu_create_fence(pApp->device);

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    pApp->surface = cgpu_surface_from_hwnd(pApp->device, pApp->wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGPUNSView* ns_view = (struct CGPUNSView*)nswindow_get_content_view(pApp->wmInfo.info.cocoa.window);
    pApp->surface = cgpu_surface_from_ns_view(pApp->device, ns_view);
#endif
    DECLARE_ZERO(CGPUSwapChainDescriptor, chain_desc);
    chain_desc.present_queues = &pApp->gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = pApp->window_width;
    chain_desc.height = pApp->window_height;
    chain_desc.surface = pApp->surface;
    chain_desc.imageCount = 3;
    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    chain_desc.enable_vsync = true;
    pApp->swapchain = cgpu_create_swapchain(pApp->device, &chain_desc);
    return 0;
}

inline int app_create_window(render_application_t* pApp, uint32_t width, uint32_t height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    const char* window_title = pApp->window_title ? pApp->window_title :gCGPUBackendNames[pApp->backend];
    DECLARE_ZERO(SWindowDescroptor, window_desc);
    window_desc.width = width;
    window_desc.height = height;
    window_desc.flags |= SKR_WINDOW_CENTERED;
    window_desc.flags |= SKR_WINDOW_RESIZABLE;
    pApp->window_handle = skr_create_window(window_title, &window_desc);
    int32_t window_width = 0, window_height = 0;
    skr_window_get_extent(pApp->window_handle, &window_width, &window_height);
    pApp->window_width = window_width;
    pApp->window_height = window_height;
    return 0;
}

inline int app_resize_window(render_application_t* pApp, uint32_t w, uint32_t h)
{
    cgpu_wait_queue_idle(pApp->gfx_queue);
    cgpu_free_swapchain(pApp->swapchain);

    DECLARE_ZERO(CGPUSwapChainDescriptor, chain_desc);
    chain_desc.present_queues = &pApp->gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = w;
    chain_desc.height = h;
    chain_desc.surface = pApp->surface;
    chain_desc.imageCount = 3;
    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    chain_desc.enable_vsync = true;
    pApp->swapchain = cgpu_create_swapchain(pApp->device, &chain_desc);
    pApp->window_width = w;
    pApp->window_height = h;
    return 0;
}

inline int app_wait_gpu_idle(render_application_t* pApp)
{
    if (pApp->gfx_queue) cgpu_wait_queue_idle(pApp->gfx_queue);
    if (pApp->present_fence) cgpu_wait_fences(&pApp->present_fence, 1);
    return 0;
}

inline int app_finalize(render_application_t* pApp)
{
    app_wait_gpu_idle(pApp);

    if (pApp->present_fence) cgpu_free_fence(pApp->present_fence);
    if (pApp->swapchain) cgpu_free_swapchain(pApp->swapchain);
    if (pApp->surface) cgpu_free_surface(pApp->device, pApp->surface);
    if (pApp->gfx_queue) cgpu_free_queue(pApp->gfx_queue);
    if (pApp->device) cgpu_free_device(pApp->device);
    if (pApp->instance) cgpu_free_instance(pApp->instance);

    if (pApp->window_handle) skr_free_window(pApp->window_handle);
    SDL_Quit();
    return 0;
}