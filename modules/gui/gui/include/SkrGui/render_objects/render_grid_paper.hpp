#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderGridPaper : public RenderBox {
public:
    SKR_GUI_TYPE(RenderGridPaper, "13dd33c9-5d56-4b06-94ce-d1c526fe75d0", RenderBox);
    RenderGridPaper(IGDIDevice* gdi_device);
    virtual ~RenderGridPaper();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    IGDIElement* gdi_element = nullptr;
};

} // namespace skr::gui