#include "SkrGui/framework/render_object/render_window.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/layer/window_layer.hpp"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{
RenderWindow::RenderWindow(IWindow* window)
    : _window(window)
{
}

void RenderWindow::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    if (child())
    {
        context->paint_child(make_not_null(child()), offset);
    }
}
void RenderWindow::perform_layout() SKR_NOEXCEPT
{
    if (child())
    {
        child()->set_constraints(BoxConstraints::Tight(_window->to_relative(_window->absolute_size())));
        child()->layout();
    }
}

NotNull<OffsetLayer*> RenderWindow::update_layer(OffsetLayer* old_layer)
{
    return old_layer ? make_not_null(old_layer) : make_not_null(SkrNew<WindowLayer>(_window));
}
} // namespace skr::gui