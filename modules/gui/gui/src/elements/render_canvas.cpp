#include "SkrGui/render_elements/render_canvas.hpp"
#include "SkrGui/gdi/gdi.hpp"

namespace skr {
namespace gui {

RenderCanvas::RenderCanvas(skr_gdi_device_id gdi_device)
    : gdi_device(gdi_device), gdi_canvas(nullptr)
{
    gdi_canvas = gdi_device->create_canvas();
}

RenderCanvas::~RenderCanvas()
{
    gdi_device->free_canvas(gdi_canvas);
}

void RenderCanvas::layout(Constraints* constraints, bool needSize)
{

}

void RenderCanvas::draw(skr_gdi_viewport_id viewport, skr_gdi_canvas_id canvas)
{
    // use this canvas for rendering (input canvas should be nullptr normally)
    canvas = gdi_canvas;
    canvas->clear_elements();
    canvas->size = { 900.f, 900.f };

    viewport->add_canvas(canvas);
    RenderElement::draw(viewport, canvas);
}

skr_float2_t RenderCanvas::get_size() const
{
    return skr_float2_t();
}

void RenderCanvas::set_size(const skr_float2_t& size)
{
    
}


} }