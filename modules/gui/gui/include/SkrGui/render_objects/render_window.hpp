#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gdi
{
struct IGDIDevice;
struct IGDIViewport;
} // namespace skr::gdi

namespace skr::gui
{

struct SKR_GUI_API RenderWindow : public RenderBox {
public:
    SKR_GUI_TYPE(RenderWindow, "bf44681e-380e-4c21-9d3b-def143c20df1", RenderBox);
    RenderWindow(gdi::IGDIDevice* gdi_device);
    virtual ~RenderWindow();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    gdi::IGDIViewport* get_gdi_viewport() { return gdi_viewport; }

protected:
    gdi::IGDIViewport* gdi_viewport = nullptr;
};

} // namespace skr::gui