#include "SkrGui/render_elements/render_canvas.hpp"
#include "SkrGui/gdi/gdi.hpp"
#include "SkrGui/framework/window_context.hpp"
#include "SkrGui/interface/window.hpp"

namespace skr {
namespace gui {

RenderCanvas::RenderCanvas(skr_gdi_device_id gdi_device)
    : gdi_device(gdi_device), gdi_canvas(nullptr)
{
    gdi_canvas = gdi_device->create_canvas();

    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>("type", "canvas", "")
    );
}

RenderCanvas::~RenderCanvas()
{
    gdi_device->free_canvas(gdi_canvas);
}

void RenderCanvas::layout(Constraints* constraints, bool needSize)
{

}

void RenderCanvas::draw(const DrawParams* params)
{
    // TODO: virtual size?
    auto platform_window = params->window_context->get_platform_window();
    uint32_t w, h;
    platform_window->get_extent(&w, &h);
    const float window_width = (float)w, window_height = (float)h;

    // use this canvas for rendering (input canvas should be nullptr normally)
    DrawParams draw_params = *params;
    draw_params.canvas = gdi_canvas;
    draw_params.canvas->clear_elements();
    draw_params.canvas->set_size(window_width, window_height);

    draw_params.viewport->add_canvas(draw_params.canvas);
    RenderElement::draw(&draw_params);
}

skr_float2_t RenderCanvas::get_size(ECanvasSpace space) const
{
    return skr_float2_t();
}

void RenderCanvas::set_size(const skr_float2_t& size, ECanvasSpace space)
{
    
}


} }