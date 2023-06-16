#include "SkrGui/render_objects/render_canvas.hpp"
#include "SkrGui/dev/gdi/gdi.hpp"
#include "SkrGui/framework/window_context.hpp"
#include "SkrGui/dev/interface/window.hpp"

namespace skr::gui
{

RenderCanvas::RenderCanvas(IGDIDevice* gdi_device)
    : RenderBox(gdi_device)
    , gdi_canvas(nullptr)
{
    // SKR_GUI_ASSERT(this->IsA<RenderBox>() && "RenderCanvas should be a RenderBox");

    gdi_canvas = gdi_device->create_canvas();

    diagnostic_builder.add_properties(
    SkrNew<TextDiagnosticProperty>(u8"type", u8"canvas", u8""));
}

RenderCanvas::~RenderCanvas()
{
    gdi_device->free_canvas(gdi_canvas);
}

void RenderCanvas::layout(BoxConstraint constraints, bool needSize)
{
}

void RenderCanvas::draw(const DrawParams* params)
{
    // TEST
    auto     platform_window = params->window_context->get_platform_window();
    uint32_t w, h;
    platform_window->get_extent(&w, &h);
    const float window_width = (float)w, window_height = (float)h;
    pos.x = pos.y = 0;
    size.width = window_width;
    size.height = window_height;
    // END TEST

    // use this canvas for rendering (input canvas should be nullptr normally)
    DrawParams draw_params = *params;
    draw_params.canvas = gdi_canvas;
    draw_params.canvas->clear_elements();
    draw_params.canvas->set_size(window_width, window_height);

    draw_params.viewport->add_canvas(draw_params.canvas);

    RenderBox::draw(&draw_params);
}

} // namespace skr::gui