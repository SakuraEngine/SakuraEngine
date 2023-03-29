#include "SkrGui/render_objects/render_color_picker.hpp"
#include "SkrGui/gdi/gdi.hpp"
#include "SkrGui/gdi/color.hpp"
#include "SkrGui/framework/window_context.hpp"
#include "SkrGui/interface/window.hpp"

#include "tracy/Tracy.hpp"

namespace skr {
namespace gui {


void RenderColorPicker::draw_color_picker(gdi::GDIElement* element, gdi::GDIPaint* paint, float x, float y, float w, float h)
{
    const float current_hue_in_degrees = get_current_hue_by_degree();
    const float current_hue_in_radians = current_hue_in_degrees * 3.1415926535897932384626433832795f / 180.f;

    const float kPi = 3.1415926535897932384626433832795f;
    element->begin_frame(1.f);
    float cx = x + w * 0.5f;
	float cy = y + h * 0.5f;
	// float r = (w < h ? w : h) * 0.5f;
	float r1 = (w < h ? w : h) * 0.5f - 5.0f;
	float r0 = r1 - 20.0f;
    float r = (r0 + r1) * 0.5f;

    // const uint32_t n = 6u;
    // const float nf = (float)n;
	// for (int32_t i = 0; i < n; i++) 
    {
        element->begin_path();
        element->circle(cx, cy, r);
        element->close_path();
        element->stroke_width(r1 - r0);
        SKR_ASSERT(paint && "ColorPicker: paint is null!");
        // element->stroke_color(255u, 255u, 255u, 255u);
        paint->set_pattern(cx, cy, w, h, 0, (gdi::GDITextureId)nullptr, {1.f, 1.f, 1.f, 1.f});
        paint->custom_vertex_color(+[](struct skr_gdi_vertex_t* pVertex, void* usrdata) SKR_NOEXCEPT
        {
            // adjust the rotate to CW[H:0~360]
            pVertex->texcoord.x = 1.f - pVertex->texcoord.x;
            // adjust H:0 to 0 degree
            float h = fmod((180.f + pVertex->texcoord.x * 360.f), 360.f);
            pVertex->color = gdi::hsv_to_abgr(h, 1.0, 1.0);
        }, this);
        element->stroke_paint(paint);
        element->stroke();
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
    element->rotate(current_hue_in_radians);

    // marker on
    element->stroke_width(2.f);
    element->begin_path();
    element->rect(r0 - 1, -3, r1 - r0 + 2, 6);
    element->stroke_color(255u, 255u, 255u, 192u);
    element->stroke();

    /*
    element->begin_path();
    element->rect(r0 - 2 - 10, -4 - 10, r1 - r0 + 4 + 20, 8 + 20);
    element->stroke_color(r0 - 2, -4, r1 - r0 + 4, 8);
    element->path_winding(gdi::EGDISolidity::Hole);
    // element->fill_paint(paint);
    element->stroke_color(255u, 255u, 255u, 192u);
    element->fill();
    */

    // center triangle
    r = r0 - 6;
    float ax = cosf(120.0f / 180.0f * kPi) * r;
    float ay = sinf(120.0f / 180.0f * kPi) * r;
    float bx = cosf(-120.0f / 180.0f * kPi) * r;
    float by = sinf(-120.0f / 180.0f * kPi) * r;
    element->begin_path();
    element->move_to(r, 0);
    element->line_to(ax, ay);
    element->line_to(bx, by);
    element->close_path();
    paint->custom_vertex_color(+[](struct skr_gdi_vertex_t* pVertex, void* usrdata) SKR_NOEXCEPT
    {
        auto _this = (RenderColorPicker*)usrdata;
        const auto index = static_cast<uint32_t>(pVertex->texcoord.x / 0.3333f);
        const double S[] = { 0.0, 0.0, 1.0 };
        const double V[] = { 0.0, 1.0, 1.0 };
        if (0 <= index && index < 3)
        {
            pVertex->color = gdi::hsv_to_abgr(_this->get_current_hue_by_degree(), S[index], V[index]);
        }
    }, this);
    paint->enable_imagespace_coordinate(true);
    element->fill_paint(paint);
    element->fill();

    // select circle on triangle
    ax = cosf(120.0f/180.0f *kPi) * r * 0.3f;
	ay = sinf(120.0f/180.0f * kPi) * r * 0.4f;
    element->stroke_width(2.f);
    element->begin_path();
    element->circle(ax, ay, 5);
    element->stroke_color(255u, 255u, 255u, 192u);
    element->stroke();

    element->restore();
}

RenderColorPicker::RenderColorPicker(skr_gdi_device_id gdi_device)
    : RenderBox(gdi_device), gdi_element(nullptr)
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

void RenderColorPicker::layout(BoxConstraint constraints, bool needSize)
{

}

void RenderColorPicker::draw(const DrawParams* params)
{
    ZoneScopedN("DrawGridPaper");

    // TEST
    auto platform_window = params->window_context->get_platform_window();
    uint32_t w, h;
    platform_window->get_extent(&w, &h);
    const float window_width = (float)w, window_height = (float)h;
    pos.x = window_width / 2;
    pos.y = window_height / 2;
    size.x = window_width / 3;
    size.y = window_height / 3;
    current_degree += .75f;
    // END TEST

    current_degree = fmod(current_degree, 360.f);
    draw_color_picker(gdi_element, gdi_paint, 
        pos.x, pos.y, size.x, size.y);
        
    addElementToCanvas(params, gdi_element);

    RenderBox::draw(params);
}

} }