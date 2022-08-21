#include "skr_renderer/skr_renderer.h"
#include "utils/make_zeroed.hpp"
#include "cgpu/api.h"
#include "cgpu/io.hpp"
#include "module/module_manager.hpp"
#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include <string.h>
#ifdef _WIN32
#include "cgpu/extensions/dstorage_windows.h"
#endif

#define BACK_BUFFER_HEIGHT 900
#define BACK_BUFFER_WIDTH 900

void skr::Renderer::initialize()
{
    auto mm = skr_get_module_manager();
    create_api_objects();

    auto vram_service_desc = make_zeroed<skr_vram_io_service_desc_t>();
    vram_service_desc.lockless = true;
    vram_service_desc.name = "vram_service";
    vram_service_desc.sleep_mode = SKR_IO_SERVICE_SLEEP_MODE_SLEEP;
    vram_service_desc.sleep_time = 1000 / 60;
    vram_service_desc.sort_method = SKR_IO_SERVICE_SORT_METHOD_PARTIAL;
    vram_service = skr::io::VRAMService::create(&vram_service_desc);
}

void skr::Renderer::finalize()
{
    skr::io::VRAMService::destroy(vram_service);

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
    for (auto& cpy_queue : cpy_queues)
    {
        if (cpy_queue) cgpu_free_queue(cpy_queue);
    }
    cpy_queues.clear();

#ifdef _WIN32
    cgpu_win_free_decompress_service(decompress_service);
#endif
    if(file_dstorage_queue) cgpu_free_dstorage_queue(file_dstorage_queue);
    if(memory_dstorage_queue) cgpu_free_dstorage_queue(memory_dstorage_queue);

    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

#define MAX_CPY_QUEUE_COUNT 2
void skr::Renderer::create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = false;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name = false;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    const auto cpy_queue_count_ = cgpu_query_queue_count(adapter, CGPU_QUEUE_TYPE_TRANSFER);
    CGPUQueueGroupDescriptor Gs[2];
    Gs[0].queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
    Gs[0].queue_count = 1;
    Gs[1].queue_type = CGPU_QUEUE_TYPE_TRANSFER;
    Gs[1].queue_count = cgpu_min(cpy_queue_count_, MAX_CPY_QUEUE_COUNT);
    if (Gs[1].queue_count)
    {
        CGPUDeviceDescriptor device_desc = {};
        device_desc.queue_groups = Gs;
        device_desc.queue_group_count = 2;
        device = cgpu_create_device(adapter, &device_desc);
        gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
        cpy_queues.resize(Gs[1].queue_count);
        for (uint32_t i = 0; i < cpy_queues.size(); i++)
        {
            cpy_queues[i] = cgpu_get_queue(device, CGPU_QUEUE_TYPE_TRANSFER, i);
        }
    }
    else
    {
        CGPUDeviceDescriptor device_desc = {};
        device_desc.queue_groups = Gs;
        device_desc.queue_group_count = 1;
        device = cgpu_create_device(adapter, &device_desc);
        gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
        cpy_queues.emplace_back(gfx_queue);
    }

    // dstorage queue
    auto dstorage_cap = cgpu_query_dstorage_availability(device);
    const bool supportDirectStorage = (dstorage_cap != CGPU_DSTORAGE_AVAILABILITY_NONE);
    if (supportDirectStorage)
    {
#ifdef _WIN32
        cgpu_win_dstorage_set_staging_buffer_size(4096 * 4096 * 8);
#endif
        {
            auto queue_desc = make_zeroed<CGPUDStorageQueueDescriptor>();
            queue_desc.name = "DirectStorageFileQueue";
            queue_desc.capacity = CGPU_DSTORAGE_MAX_QUEUE_CAPACITY;
            queue_desc.source = CGPU_DSTORAGE_SOURCE_FILE;
            queue_desc.priority = CGPU_DSTORAGE_PRIORITY_NORMAL;
            file_dstorage_queue = cgpu_create_dstorage_queue(device, &queue_desc);
        }
        {
            auto queue_desc = make_zeroed<CGPUDStorageQueueDescriptor>();
            queue_desc.name = "DirectStorageMemoryQueue";
            queue_desc.capacity = CGPU_DSTORAGE_MAX_QUEUE_CAPACITY;
            queue_desc.source = CGPU_DSTORAGE_SOURCE_MEMORY;
            queue_desc.priority = CGPU_DSTORAGE_PRIORITY_NORMAL;
            memory_dstorage_queue = cgpu_create_dstorage_queue(device, &queue_desc);
        }
    }

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

#ifdef _WIN32
    decompress_service = cgpu_win_create_decompress_service();
#endif
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
    int32_t width, height;
    skr_window_get_extent(window, &width, &height);
    // Create swapchain
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = width;
    chain_desc.height = height;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = CGPU_FORMAT_B8G8R8A8_UNORM;
    chain_desc.enable_vsync = false;
    auto swapchain = cgpu_create_swapchain(device, &chain_desc);
    swapchains[window] = swapchain;
    return swapchain;
}