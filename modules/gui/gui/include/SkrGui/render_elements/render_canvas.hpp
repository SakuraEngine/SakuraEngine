#pragma once
#include "SkrGui/render_elements/element.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDICanvas, skr_gdi_canvas)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderCanvas : public RenderElement
{
public:
    RenderCanvas(skr_gdi_device_id gdi_device);
    virtual ~RenderCanvas();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);

    skr_gdi_canvas_id get_gdi_canvas() { return gdi_canvas; }

protected:
    skr_gdi_device_id gdi_device = nullptr;
    skr_gdi_canvas_id gdi_canvas = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderCanvas, skr_gui_render_canvas);