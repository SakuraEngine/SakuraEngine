#include "SkrGui/render_objects/render_color_picker.hpp"
#include "SkrGui/dev/gdi/gdi.hpp"
#include "SkrGui/dev/gdi/color.hpp"
#include "SkrGui/framework/painting_context.hpp"
#include "SkrGui/backend/render/canvas.hpp"

namespace skr::gui
{
void RenderColorPicker::perform_layout() SKR_NOEXCEPT
{
    set_size(constraints().biggest());
}

void RenderColorPicker::paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT
{
    Rect        paint_rect = Rect::OffsetSize(offset, size());
    float       current_degree = 0;
    const float kPi = 3.1415926535897932384626433832795f;

    // draw
    auto canvas = context->canvas();
    {
        auto _ = canvas->paint_scope();

        Offset center = paint_rect.center();
        float  outer_radius = paint_rect.size().shortest_side() / 2.f - 5.f;
        float  inner_radius = outer_radius - 20.f;
        float  radius = (outer_radius + inner_radius) / 2.f;

        // draw hue ring
        {
            // center
            canvas->path_begin();
            canvas->path_circle(center, radius);
            canvas->state_stroke_width(outer_radius - inner_radius);
            canvas->path_stroke(CustomBrush(+[](GDIVertex& v) {
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
            canvas->state_stroke_width(1.0f);
            canvas->path_stroke(ColorBrush({ 0, 0, 0, 0.5f }));
        }

        // draw selector
        {
            // state
            auto _ = canvas->state_scope();
            canvas->state_translate(center);
            canvas->state_rotate(current_degree);

            // marker on
            canvas->path_begin();
            canvas->path_rect({ inner_radius - 1, -3, outer_radius + 1, 3 });
            canvas->state_stroke_width(2.f);
            canvas->path_stroke(ColorBrush({ 1, 1, 1, 0.7529411765f }));

            // center triangle
            canvas->path_begin();
            canvas->path_move_to(Offset::Radians(0, radius));
            canvas->path_line_to(Offset::Radians(120.f / kPi, radius));
            canvas->path_line_to(Offset::Radians(-120.f / kPi, radius));
            canvas->path_close();
            canvas->path_fill(CustomBrush([current_degree](GDIVertex& v) {
                const auto   index = static_cast<uint32_t>(v.texcoord.x / 0.3333f);
                const double S[] = { 0.0, 0.0, 1.0 };
                const double V[] = { 0.0, 1.0, 1.0 };
                if (0 <= index && index < 3)
                {
                    v.color = hsv_to_abgr(current_degree, S[index], V[index]);
                }
            }));

            // select circle on triangle
            canvas->path_begin();
            canvas->path_circle(center, 5);
            canvas->state_stroke_width(2.f);
            canvas->path_stroke(ColorBrush({ 1, 1, 1, 0.7529411765f }));
        }
    }

    Super::paint(context, offset);
}

} // namespace skr::gui