#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gdi
{
struct IGDIDevice;
struct IGDICanvas;
} // namespace skr::gdi

namespace skr::gui
{

struct SKR_GUI_API RenderCanvas : public RenderBox {
public:
    SKR_GUI_TYPE(RenderCanvas, "b3c8ede6-d878-472c-a1c1-6b3acdc9f1f0", RenderBox);
    RenderCanvas(gdi::IGDIDevice* gdi_device);
    virtual ~RenderCanvas();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    gdi::IGDICanvas* get_gdi_canvas() { return gdi_canvas; }

protected:
    gdi::IGDICanvas* gdi_canvas = nullptr;
};

} // namespace skr::gui
