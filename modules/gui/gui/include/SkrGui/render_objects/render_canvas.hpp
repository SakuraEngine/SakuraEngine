#pragma once
#include "SkrGui/framework/render_box.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDICanvas, skr_gdi_canvas)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderCanvas : public RenderBox
{
public:
    RenderCanvas(skr_gdi_device_id gdi_device);
    virtual ~RenderCanvas();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    skr_gdi_canvas_id get_gdi_canvas() { return gdi_canvas; }

protected:
    skr_gdi_canvas_id gdi_canvas = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderCanvas, skr_gui_render_canvas);