#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGuiRenderer/device/skr_native_window.hpp"

namespace skr::gui
{

// other view
NotNull<IWindow*> SkrNativeDevice::create_window(const WindowDesc& desc)
{
    return make_not_null<IWindow*>(nullptr);
}
void SkrNativeDevice::destroy_window(NotNull<IWindow*> view)
{
}

// view ops
void SkrNativeDevice::update_window(NotNull<IWindow*> view)
{
}
void SkrNativeDevice::draw_window(NotNull<IWindow*> view)
{
}
} // namespace skr::gui