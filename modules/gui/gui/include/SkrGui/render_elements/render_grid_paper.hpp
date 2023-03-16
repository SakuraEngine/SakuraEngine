#pragma once
#include "SkrGui/framework/render_box.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIElement, skr_gdi_element)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderGridPaper : public RenderBox
{
public:
    RenderGridPaper(skr_gdi_device_id gdi_device);
    virtual ~RenderGridPaper();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    skr_gdi_element_id gdi_element = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderGridPaper, skr_gui_render_grid_paper);