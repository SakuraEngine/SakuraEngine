#include "skr_renderer.h"
#include "utils/log.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"

IMPLEMENT_DYNAMIC_MODULE(SkrRendererModule, SkrRenderer);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "SkrRenderer",
    "prettyname" : "SakuraRenderer",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrRenderGraph", "version":"0.1.0"},
        {"name":"SkrImGui", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
SkrRenderer)

#define BACK_BUFFER_HEIGHT 900
#define BACK_BUFFER_WIDTH 900

void SkrRendererModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("skr renderer loaded!");

    create_api_objects();
}

void create_test_materials();
void free_test_materials();

void SkrRendererModule::on_unload()
{
    SKR_LOG_INFO("skr renderer unloaded!");
    
    if (swapchain) cgpu_free_swapchain(swapchain);
    if (surface) cgpu_free_surface(device, surface);
    cgpu_free_sampler(linear_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

void SkrRendererModule::create_api_objects()
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


CGPUSwapChainId SkrRendererModule::register_window(SWindowHandle window)
{
    // Create swapchain
    surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(window));
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = CGPU_FORMAT_B8G8R8A8_UNORM;
    chain_desc.enable_vsync = false;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
    return swapchain;
}

SkrRendererModule* SkrRendererModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SkrRendererModule*>(mm->get_module("SkrRenderer"));
    return rm;
}

ECGPUFormat SkrRendererModule::get_swapchain_format() const
{
    if (swapchain) return (ECGPUFormat)swapchain->back_buffers[0]->format;
    return CGPU_FORMAT_B8G8R8A8_UNORM;
}    

CGPUSamplerId SkrRendererModule::get_linear_sampler() const
{
    return linear_sampler;
}

CGPUDeviceId SkrRendererModule::get_cgpu_device() const
{
    return device;
}

CGPUQueueId SkrRendererModule::get_gfx_queue() const
{
    return gfx_queue;
}

CGPUSwapChainId skr_renderer_register_window(SWindowHandle window)
{
    return SkrRendererModule::Get()->register_window(window);
}

ECGPUFormat skr_renderer_get_swapchain_format()
{
    return SkrRendererModule::Get()->get_swapchain_format();
}

CGPUSamplerId skr_renderer_get_linear_sampler()
{
    return SkrRendererModule::Get()->get_linear_sampler();
}

CGPUQueueId skr_renderer_get_gfx_queue()
{
    return SkrRendererModule::Get()->get_gfx_queue();
}

CGPUDeviceId skr_renderer_get_cgpu_device()
{
    return SkrRendererModule::Get()->get_cgpu_device();
}