#include "SkrGui/render_objects/render_window.hpp"
#include "SkrGui/gdi/gdi.hpp"

namespace skr {
namespace gui {

RenderWindow::RenderWindow(skr_gdi_device_id gdi_device)
    : RenderBox(gdi_device), gdi_viewport(nullptr)
{
    gdi_viewport = gdi_device->create_viewport();

    diagnostic_builder.add_properties(
        SkrNew<TextDiagnosticProperty>(u8"type", u8"window", u8"a virtual window to place canvas")
    );
}

RenderWindow::~RenderWindow()
{
    gdi_device->free_viewport(gdi_viewport);
}

void RenderWindow::layout(BoxConstraint constraints, bool needSize)
{

}

void RenderWindow::draw(const DrawParams* params)
{
    DrawParams draw_params = *params;
    draw_params.viewport = gdi_viewport;
    draw_params.viewport->clear_canvas();

    RenderBox::draw(&draw_params);
}

SKR_GUI_TYPE_IMPLMENTATION(RenderWindow);

} }