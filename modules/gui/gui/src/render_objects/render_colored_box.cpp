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
        {
            auto _ = canvas->path_fill_scope(ColorBrush(color()));
            canvas->state_anti_alias(false);
            canvas->path_rect(Rect::OffsetSize(offset, size()));
        }
    }

    if (child())
    {
        context->paint_child(make_not_null(child()), offset);
    }
}
} // namespace skr::gui