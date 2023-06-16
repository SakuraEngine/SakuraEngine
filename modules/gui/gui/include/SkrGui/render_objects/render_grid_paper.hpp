#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIElement, skr_gdi_element)

namespace skr
{
namespace gui
{

struct SKR_GUI_API RenderGridPaper : public RenderBox {
public:
    SKR_GUI_TYPE(RenderGridPaper, "13dd33c9-5d56-4b06-94ce-d1c526fe75d0", RenderBox);
    RenderGridPaper(skr_gdi_device_id gdi_device);
    virtual ~RenderGridPaper();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    skr_gdi_element_id gdi_element = nullptr;
};

} // namespace gui
} // namespace skr

SKR_DECLARE_TYPE_ID(skr::gui::RenderGridPaper, skr_gui_render_grid_paper);