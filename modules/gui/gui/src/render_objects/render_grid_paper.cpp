#include "SkrGui/render_objects/render_grid_paper.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"

namespace skr::gui
{
void RenderGridPaper::perform_layout() SKR_NOEXCEPT
{
    set_size(constraints().biggest());
}

void RenderGridPaper::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    // paint
    ICanvas* canvas           = context->canvas();
    Rectf    paint_rect       = Rectf::OffsetSize(offset, size());
    Color    background_color = { 235.f / 255.f, 235.f / 255.f, 235.f / 255.f, 235.f / 255.f };
    Sizef    grid_size        = { 100, 100 };
    Sizef    sub_grid_size    = { 10, 10 };
    Color    grid_color       = { 125.f / 255.f, 125.f / 255.f, 255.f / 255.f, 200.f / 255.f };
    Color    sub_grid_color   = { 88.f / 255.f, 88.f / 255.f, 222.f / 255.f, 180.f / 255.f };
    {
        auto _ = canvas->paint_scope();

        // background
        canvas->draw_rect(paint_rect, FillPen(), ColorBrush(background_color));
        // draw grid
        canvas->path_begin();
        for (float x = std::ceil((paint_rect.left - 1) / grid_size.width) * grid_size.width; x < paint_rect.right + 1; x += grid_size.width)
        {
            canvas->path_move_to({ x, paint_rect.top });
            canvas->path_line_to({ x, paint_rect.bottom });
        }
        for (float y = std::ceil((paint_rect.top - 1) / grid_size.height) * grid_size.height; y < paint_rect.bottom + 1; y += grid_size.height)
        {
            canvas->path_move_to({ paint_rect.left, y });
            canvas->path_line_to({ paint_rect.right, y });
        }
        canvas->path_end(StrokePen().width(2), ColorBrush(grid_color));

        // draw sub grid
        canvas->path_begin();
        for (float x = std::ceil((paint_rect.left - 1) / sub_grid_size.width) * sub_grid_size.width; x < paint_rect.right + 1; x += sub_grid_size.width)
        {
            canvas->path_move_to({ x, paint_rect.top });
            canvas->path_line_to({ x, paint_rect.bottom });
        }
        for (float y = std::ceil((paint_rect.top - 1) / sub_grid_size.height) * sub_grid_size.height; y < paint_rect.bottom + 1; y += sub_grid_size.height)
        {
            canvas->path_move_to({ paint_rect.left, y });
            canvas->path_line_to({ paint_rect.right, y });
        }
        canvas->path_end(StrokePen().width(1), ColorBrush(sub_grid_color));
    }

    Super::paint(context, offset);
}

// hit test
bool RenderGridPaper::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    return size().contains(local_position);
}

} // namespace skr::gui