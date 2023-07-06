#include "SkrGui/framework/render_object/render_window.hpp"

namespace skr::gui
{
RenderWindow::RenderWindow(IWindow* window)
    : _window(window)
{
}

} // namespace skr::gui