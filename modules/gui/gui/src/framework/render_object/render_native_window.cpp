#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{
RenderNativeWindow::RenderNativeWindow(INativeWindow* native_window)
    : RenderWindow(native_window)
{
}

void RenderNativeWindow::prepare_initial_frame() SKR_NOEXCEPT
{
    // TODO. schedule layout & paint & root layer
}
} // namespace skr::gui