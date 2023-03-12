#include "SkrGui/render_elements/render_color_picker.hpp"
#include "SkrGui/gdi/gdi.hpp"
#include "SkrGui/gdi/color.hpp"
#include "SkrGui/framework/window_context.hpp"
#include "SkrGui/interface/window.hpp"

#include "tracy/Tracy.hpp"

namespace skr {
namespace gui {

inline static void draw_color_picker(gdi::GDIElement* element, gdi::GDIPaint* paint, float x, float y, float w, float h, float t)
{
    // non-convex
    return;

    const float kPi = 3.1415926535897932384626433832795f;
    element->begin_frame(1.f);
    float r0, r1, ax, ay, bx, by, cx, cy, aeps, r;
	float hue = sinf(t * 0.12f);
    cx = x + w * 0.5f;
	cy = y + h * 0.5f;
	r1 = (w < h ? w : h) * 0.5f - 5.0f;
	r0 = r1 - 20.0f;
	aeps = 0.5f / r1;	// half a pixel arc length in radians (2pi cancels out).

    const uint32_t n = 6u;
    const float nf = (float)n;
	for (int32_t i = 0; i < n; i++) 
    {
		float a0 = (float)i / nf * kPi * 2.0f - aeps;
		float a1 = (float)(i + 1.0f) / nf * kPi * 2.0f + aeps;
        element->begin_path();
        element->arc(cx, cy, r0, a0, a1, gdi::EGDIWinding::CW);
		element->arc(cx, cy, r1, a1, a0, gdi::EGDIWinding::CCW);
        element->close_path();
		ax = cx + cosf(a0) * (r0+r1) * 0.5f;
		ay = cy + sinf(a0) * (r0+r1) * 0.5f;
		bx = cx + cosf(a1) * (r0+r1) * 0.5f;
		by = cy + sinf(a1) * (r0+r1) * 0.5f;
        SKR_ASSERT(paint && "ColorPicker: paint is null!");
        paint->linear_gradient(ax, ay, bx, by, 
            gdi::hsla_to_rgbaf(a0 / (kPi * 2), 1.0f, 0.55f, 255u), 
            gdi::hsla_to_rgbaf(a1 / (kPi * 2), 1.0f, 0.55f, 255u));
        element->fill_paint(paint);
        element->fill();
	}
    // hue ring
    element->begin_path();
    element->circle(cx, cy, r0 - 0.5f);
    element->circle(cx, cy, r1 + 0.5f);
    element->stroke_color(0u, 0u, 0u, 64u);
	element->stroke_width(1.0f);
    element->stroke();

    // selector
    element->save();
    element->translate(cx, cy);
    element->rotate(hue * kPi * 2);

    // marker on
    element->stroke_width(2.f);
    element->begin_path();
    element->rect(r0 - 1, -3, r1 - r0 + 2, 6);
    element->stroke_color(255u, 255u, 255u, 192u);
    element->stroke();

    paint->box_gradient(r0 - 3, -5, r1 - r0 + 6, 10, 2, 4, 
        {0.f, 0.f, 0.f, 0.5f}, 
        {0.f, 0.f, 0.f, 0.f});
    element->begin_path();
    element->rect(r0 - 2 - 10, -4 - 10, r1 - r0 + 4 + 20, 8 + 20);
    element->stroke_color(r0 - 2, -4, r1 - r0 + 4, 8);
    element->path_winding(gdi::EGDISolidity::Hole);
    element->fill_paint(paint);
    element->fill();

    // center triangle
    r = r0 - 6;
    ax = cosf(120.0f / 180.0f * kPi) * r;
    ay = sinf(120.0f / 180.0f * kPi) * r;
    bx = cosf(-120.0f / 180.0f * kPi) * r;
    by = sinf(-120.0f / 180.0f * kPi) * r;
    element->begin_path();
    element->move_to(r, 0);
    element->line_to(ax, ay);
    element->line_to(bx, by);
    element->close_path();
    paint->linear_gradient(r, 0, ax, ay, 
        gdi::hsla_to_rgbaf(hue, 1.0f, 0.5f, 255u), 
        { 1.f, 1.f, 1.f, 1.f });
    element->fill_paint(paint);
    element->fill();
    paint->linear_gradient((r + ax) * 0.5f, (0 + ay) * 0.5f, bx, by, 
        { 0.0f, 0.0f, 0.0f, 0.f },
        { 0.0f, 0.0f, 0.0f, 1.f });
    element->fill_paint(paint);
    element->fill();
    element->stroke_color(0u, 0u, 0u, 64u);
    element->stroke();

    // select circle on triangle
    ax = cosf(120.0f/180.0f *kPi) * r * 0.3f;
	ay = sinf(120.0f/180.0f * kPi) * r * 0.4f;
    element->stroke_width(2.f);
    element->begin_path();
    element->circle(ax, ay, 5);
    element->stroke_color(255u, 255u, 255u, 192u);
    element->stroke();

    paint->radial_gradient(ax, ay, 7, 9, 
        { 0.0f, 0.0f, 0.0f, 0.5f },
        { 0.0f, 0.0f, 0.0f, 0.0f });
    element->begin_path();
    element->rect(ax - 20, ay - 20, 40, 40);
    element->circle(ax, ay, 7);
    element->path_winding(gdi::EGDISolidity::Hole);
    element->fill_paint(paint);
    element->fill();

    element->restore();
    element->restore();
}

RenderColorPicker::RenderColorPicker(skr_gdi_device_id gdi_device)
    : gdi_device(gdi_device), gdi_element(nullptr)
{
    gdi_element = gdi_device->create_element();
    gdi_paint = gdi_device->create_paint();

    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>("type", "color_picker", "draws color picker")
    );
}

RenderColorPicker::~RenderColorPicker()
{
    gdi_device->free_paint(gdi_paint);
    gdi_device->free_element(gdi_element);
}

void RenderColorPicker::layout(Constraints* constraints, bool needSize)
{

}

void RenderColorPicker::draw(const DrawParams* params)
{
    ZoneScopedN("DrawGridPaper");

    // TODO: virtual size?
    auto platform_window = params->window_context->get_platform_window();
    uint32_t w, h;
    platform_window->get_extent(&w, &h);
    const float window_width = (float)w, window_height = (float)h;
    params->canvas->add_element(gdi_element);
    static float t = 10.f;
    t += .01f;
    draw_color_picker(gdi_element, gdi_paint, 
        window_width / 3, window_height / 3, 
        window_width / 2, window_height / 2, t);

    RenderElement::draw(params);
}

skr_float2_t RenderColorPicker::get_size() const
{
    return skr_float2_t();
}

void RenderColorPicker::set_size(const skr_float2_t& size)
{

}


} }