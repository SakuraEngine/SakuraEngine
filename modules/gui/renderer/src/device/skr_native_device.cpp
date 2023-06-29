#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/device/skr_native_window.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"

namespace skr::gui
{
void SkrNativeDevice::init()
{
    // init render device
    _render_device = SkrNew<SkrRenderDevice>();
    _render_device->init();

    // get display metrics
    uint32_t       count;
    SMonitorHandle monitors[128];
    skr_get_all_monitors(&count, monitors);
    for (uint32_t i = 0; i < count; ++i)
    {
        SMonitorHandle monitor = monitors[i];
        int32_t        x, y, width, height;
        skr_monitor_get_extent(monitor, &width, &height);
        skr_monitor_get_position(monitor, &x, &y);

        auto out = _display_metrics.monitors.emplace_back();

        // name
        // device_id
        // native_size
        // max_resolution
        // display_area
        out.work_area = Recti::LTWH(x, y, width, height);
        // is_primary
        // features
    }

    // TODO. total data
}
void SkrNativeDevice::shutdown()
{

    // shutdown render device
    _render_device->shutdown();
    SkrDelete(_render_device);
}

// view
NotNull<IWindow*> SkrNativeDevice::create_window()
{
    auto view = SkrNew<SkrNativeWindow>(this);
    _all_views.push_back(view);
    return make_not_null(view);
}
void SkrNativeDevice::destroy_window(NotNull<IWindow*> view)
{
    // erase it
    auto it = std::find(_all_views.begin(), _all_views.end(), view);
    if (it != _all_views.end())
    {
        _all_views.erase(it);
    }

    // delete
    SkrDelete(view.get());
}

// view ops
void SkrNativeDevice::draw_window(NotNull<IWindow*> view)
{
    // TODO. draw window
}

// display info
const DisplayMetrics& SkrNativeDevice::display_metrics() const
{
    return _display_metrics;
}
} // namespace skr::gui