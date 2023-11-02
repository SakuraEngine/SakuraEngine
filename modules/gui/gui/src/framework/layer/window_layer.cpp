#include "SkrGui/framework/layer/window_layer.hpp"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{
WindowLayer::WindowLayer(INativeWindow* window)
    : _window(window)
{
}

void WindowLayer::update_window()
{
    if (needs_composite())
    {
        _window->update_content(this);
        cancel_needs_composite();
    }
}

} // namespace skr::gui