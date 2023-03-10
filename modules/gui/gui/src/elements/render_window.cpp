#include "SkrGui/render_elements/render_window.hpp"
#include "SkrGui/gdi/gdi.hpp"

namespace skr {
namespace gui {

RenderWindow::RenderWindow(skr_gdi_device_id gdi_device)
    : gdi_device(gdi_device), gdi_viewport(nullptr)
{
    gdi_viewport = gdi_device->create_viewport();
}

RenderWindow::~RenderWindow()
{
    gdi_device->free_viewport(gdi_viewport);
}

void RenderWindow::layout(Constraints* constraints, bool needSize)
{

}

void RenderWindow::draw(skr_gdi_viewport_id viewport, skr_gdi_canvas_id canvas)
{
    viewport = gdi_viewport;
    viewport->clear_canvas();

    RenderElement::draw(viewport, canvas);
}

skr_float2_t RenderWindow::get_size() const
{
    return skr_float2_t();
}

void RenderWindow::set_size(const skr_float2_t& size)
{

}


} }