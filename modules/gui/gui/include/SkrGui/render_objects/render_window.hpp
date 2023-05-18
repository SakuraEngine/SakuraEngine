#pragma once
#include "SkrGui/framework/render_box.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderWindow : public RenderBox
{
public:
    SKR_GUI_TYPE(RenderWindow, RenderBox, u8"bf44681e-380e-4c21-9d3b-def143c20df1");
    RenderWindow(skr_gdi_device_id gdi_device);
    virtual ~RenderWindow();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    skr_gdi_viewport_id get_gdi_viewport() { return gdi_viewport; }

protected:
    skr_gdi_viewport_id gdi_viewport = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderWindow, skr_gui_render_window);