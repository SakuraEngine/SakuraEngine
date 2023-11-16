#include "SkrGui/framework/render_object/render_window.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/layer/window_layer.hpp"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{
RenderWindow::RenderWindow(INativeWindow* window)
    : _window(window)
{
}

void RenderWindow::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    if (child())
    {
        context->paint_child(child(), offset);
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
bool RenderWindow::is_repaint_boundary() const SKR_NOEXCEPT
{
    return true;
}

NotNull<OffsetLayer*> RenderWindow::update_layer(OffsetLayer* old_layer)
{
    return old_layer ? old_layer : SkrNew<WindowLayer>(_window);
}
} // namespace skr::gui