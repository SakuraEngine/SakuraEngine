#include "SkrGui/render_objects/render_color_picker.hpp"
#include "SkrGui/dev/gdi/color.hpp" // TODO. move to color
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"

namespace skr::gui
{
void RenderColorPicker::perform_layout() SKR_NOEXCEPT
{
    set_size(constraints().biggest());
}

void RenderColorPicker::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    Rectf       paint_rect     = Rectf::OffsetSize(offset, size());
    float       current_degree = 0;
    const float kPi            = 3.1415926535897932384626433832795f;

    // draw
    auto canvas = context->canvas();
    {
        auto _ = canvas->paint_scope();

        Offsetf center       = paint_rect.center();
        float   outer_radius = paint_rect.size().shortest_side() / 2.f - 5.f;
        float   inner_radius = outer_radius - 20.f;
        float   radius       = (outer_radius + inner_radius) / 2.f;

        // draw hue ring
        {
            // center
            canvas->path_begin();
            canvas->path_circle(center, radius);
            canvas->path_end(StrokePen().width(outer_radius - inner_radius), ColorBrush().custom(+[](PaintVertex& v) {
                // adjust the rotate to CW[H:0~360]
                v.texcoord.x = 1.f - v.texcoord.x;
                // adjust H:0 to 0 degree
                float h = fmod((180.f + v.texcoord.x * 360.f), 360.f);
                v.color = hsv_to_abgr(h, 1.0, 1.0);
            }));

            // border
            canvas->path_begin();
            canvas->path_circle(center, inner_radius - 0.5f);
            canvas->path_circle(center, outer_radius + 0.5f);
            canvas->path_end(StrokePen().width(1.0f), ColorBrush({ 0, 0, 0, 0.5f }));
        }

        // draw selector
        {
            // state
            auto _ = canvas->state_scope();
            canvas->state_translate(center);
            canvas->state_rotate(current_degree);

            // marker on
            canvas->draw_rect({ inner_radius - 1, -3, outer_radius + 1, 3 }, StrokePen().width(2.0f), ColorBrush({ 1, 1, 1, 0.7529411765f }));

            // center triangle
            canvas->path_begin();
            canvas->path_move_to(Offsetf::Radians(0, radius));
            canvas->path_line_to(Offsetf::Radians(120.f / kPi, radius));
            canvas->path_line_to(Offsetf::Radians(-120.f / kPi, radius));
            canvas->path_close();
            canvas->path_end(FillPen(), ColorBrush().custom(
                                        [current_degree](PaintVertex& v) {
                                            const auto   index = static_cast<uint32_t>(v.texcoord.x / 0.3333f);
                                            const double S[]   = { 0.0, 0.0, 1.0 };
                                            const double V[]   = { 0.0, 1.0, 1.0 };
                                            if (0 <= index && index < 3)
                                            {
                                                v.color = hsv_to_abgr(current_degree, S[index], V[index]);
                                            }
                                        }));

            // select circle on triangle
            canvas->draw_circle(center, 5, StrokePen().width(2), ColorBrush({ 1, 1, 1, 0.7529411765f }));
        }
    }

    Super::paint(context, offset);
}

// hit test
bool RenderColorPicker::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    return size().contains(local_position);
}

} // namespace skr::gui