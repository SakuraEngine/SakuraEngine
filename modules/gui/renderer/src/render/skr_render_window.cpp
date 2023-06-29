#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"

namespace skr::gui
{
SkrRenderWindow::SkrRenderWindow(SkrRenderDevice* owner, SWindowHandle window)
    : _owner(owner)
    , _window(window)
{
    auto device = _owner->cgpu_device();
    auto queue = _owner->cgpu_queue();

    _cgpu_fence = cgpu_create_fence(_owner->cgpu_device());
    _cgpu_surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(_window));

    int32_t width, height;
    skr_window_get_extent(_window, &width, &height);
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.surface = _cgpu_surface;
    chain_desc.present_queues = &queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = width;
    chain_desc.height = height;
    chain_desc.image_count = 2;
    chain_desc.format = CGPU_FORMAT_B8G8R8A8_UNORM; // TODO: use correct screen buffer format
    chain_desc.enable_vsync = false;
    _cgpu_swapchain = cgpu_create_swapchain(device, &chain_desc);
}
SkrRenderWindow::~SkrRenderWindow()
{
    if (_cgpu_fence)
    {
        cgpu_free_fence(_cgpu_fence);
    }
    if (_cgpu_surface)
    {
        cgpu_free_surface(_owner->cgpu_device(), _cgpu_surface);
    }
    if (_cgpu_swapchain)
    {
        cgpu_free_swapchain(_cgpu_swapchain);
    }
}

void SkrRenderWindow::sync_window_size()
{
    auto device = _owner->cgpu_device();
    auto queue = _owner->cgpu_queue();

    // TODO. fix this hack
    cgpu_wait_queue_idle(queue);

    if (!_cgpu_fence)
    {
        _cgpu_fence = cgpu_create_fence(_owner->cgpu_device());
    }
    if (!_cgpu_surface)
    {
        _cgpu_surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(_window));
    }

    if (_cgpu_fence)
    {
        cgpu_wait_fences(&_cgpu_fence, 1);
    }
    if (_cgpu_swapchain)
    {
        cgpu_free_swapchain(_cgpu_swapchain);
    }

    int32_t width, height;
    skr_window_get_extent(_window, &width, &height);
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.surface = _cgpu_surface;
    chain_desc.present_queues = &queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = width;
    chain_desc.height = height;
    chain_desc.image_count = 2;
    chain_desc.format = CGPU_FORMAT_B8G8R8A8_UNORM; // TODO: use correct screen buffer format
    chain_desc.enable_vsync = false;
    _cgpu_swapchain = cgpu_create_swapchain(device, &chain_desc);
}
} // namespace skr::gui