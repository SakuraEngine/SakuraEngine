#include "SkrGui/render_objects/render_colored_box.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"

namespace skr::gui
{
void RenderColoredBox::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    if (!size().is_empty())
    {
        auto canvas = context->canvas();

        auto _ = canvas->paint_scope();
        canvas->draw_rect(Rectf::OffsetSize(offset, size()), FillPen().anti_alias(false), ColorBrush(color()));
    }

    if (child())
    {
        context->paint_child(child(), offset);
    }
}
} // namespace skr::gui