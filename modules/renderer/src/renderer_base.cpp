#include "skr_renderer/skr_renderer.h"
#include "cgpu/api.h"
#include "module/module_manager.hpp"
#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include <string.h>

#define BACK_BUFFER_HEIGHT 900
#define BACK_BUFFER_WIDTH 900

void skr::Renderer::initialize()
{
    auto mm = skr_get_module_manager();
    create_api_objects();
}

void skr::Renderer::finalize()
{
    for (auto& swapchain : swapchains)
    {
        if (swapchain.second) cgpu_free_swapchain(swapchain.second);
    }
    swapchains.clear();
    for (auto& surface : surfaces)
    {
        if (surface.second) cgpu_free_surface(device, surface.second);
    }
    surfaces.clear();
    cgpu_free_sampler(linear_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

void skr::Renderer::create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
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
    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    linear_sampler = cgpu_create_sampler(device, &sampler_desc);
}

CGPUSwapChainId skr::Renderer::register_window(SWindowHandle window)
{
    // find registered
    {
        auto _ = swapchains.find(window);
        if (_ != swapchains.end()) return _->second;
    }
    CGPUSurfaceId surface = nullptr;
    // find registered
    {
        auto _ = surfaces.find(window);
        if (_ != surfaces.end())
            surface = _->second;
        else
        {
            surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(window));
            surfaces[window] = surface;
        }
    }
    // Create swapchain
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = CGPU_FORMAT_B8G8R8A8_UNORM;
    chain_desc.enable_vsync = false;
    auto swapchain = cgpu_create_swapchain(device, &chain_desc);
    swapchains[window] = swapchain;
    return swapchain;
}