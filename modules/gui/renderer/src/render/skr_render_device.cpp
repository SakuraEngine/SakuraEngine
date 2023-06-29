#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "misc/make_zeroed.hpp"
#include "platform/memory.h"

// constants
namespace skr::gui
{
#if _WIN32
static const ECGPUBackend kSkr_GUI_DEFAULT_RENDER_BACKEND = CGPU_BACKEND_D3D12;
#else
static const ECGPUBackend kSkr_GUI_DEFAULT_RENDER_BACKEND = CGPU_BACKEND_VULKAN;
#endif
} // namespace skr::gui

namespace skr::gui
{
// init & shutdown
void SkrRenderDevice::init()
{
    // create instance
    auto instance_desc = make_zeroed<CGPUInstanceDescriptor>();
    instance_desc.backend = kSkr_GUI_DEFAULT_RENDER_BACKEND;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    _cgpu_instance = cgpu_create_instance(&instance_desc);

    // filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(_cgpu_instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(_cgpu_instance, adapters, &adapters_count);
    _cgpu_adapter = adapters[0];

    // create device & queue
    auto queue_group_desc = make_zeroed<CGPUQueueGroupDescriptor>();
    queue_group_desc.queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count = 1;
    auto device_desc = make_zeroed<CGPUDeviceDescriptor>();
    device_desc.queue_groups = &queue_group_desc;
    device_desc.queue_group_count = 1;
    _cgpu_device = cgpu_create_device(_cgpu_adapter, &device_desc);
    _cgpu_queue = cgpu_get_queue(_cgpu_device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

    // create rg
    _render_graph = RenderGraph::create(
    [=](render_graph::RenderGraphBuilder& b) {
        b.with_device(_cgpu_device)
        .with_gfx_queue(_cgpu_queue);
    });
}
void SkrRenderDevice::shutdown()
{
    // destroy rg
    if (_render_graph) RenderGraph::destroy(_render_graph);

    // destroy cgpu
    if (_cgpu_queue) cgpu_free_queue(_cgpu_queue);
    if (_cgpu_device) cgpu_free_device(_cgpu_device);
    if (_cgpu_instance) cgpu_free_instance(_cgpu_instance);
}

// create view
SkrRenderWindow* SkrRenderDevice::create_window(SWindowHandle window)
{
    return SkrNew<SkrRenderWindow>(this, window);
}
void SkrRenderDevice::destroy_window(SkrRenderWindow* view)
{
    SkrDelete(view);
}
} // namespace skr::gui