#include "SkrGui/framework/layer/native_window_layer.hpp"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{
NativeWindowLayer::NativeWindowLayer(INativeWindow* native_window)
    : WindowLayer(native_window)
{
}
} // namespace skr::gui