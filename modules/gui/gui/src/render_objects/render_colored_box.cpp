#include "SkrGui/render_objects/render_colored_box.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/backend/render/canvas.hpp"

namespace skr::gui
{
void RenderColoredBox::paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT
{
    if (!size().is_empty())
    {
        auto canvas = context->canvas();

        auto _ = canvas->paint_scope();
        canvas->draw_rect(Rect::OffsetSize(offset, size()), FillPen().anti_alias(false), ColorBrush(color()));
    }

    if (child())
    {
        context->paint_child(make_not_null(child()), offset);
    }
}
} // namespace skr::gui