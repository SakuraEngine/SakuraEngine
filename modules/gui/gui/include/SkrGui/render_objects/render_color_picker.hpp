#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderColorPicker : public RenderBox {
public:
    SKR_GUI_TYPE(RenderColorPicker, "25a95354-b3fa-4729-b06f-1a85d0f227c4", RenderBox);
    RenderColorPicker(IGDIDevice* gdi_device);
    virtual ~RenderColorPicker();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    void draw_color_picker(IGDIElement* element, IGDIPaint* paint, float x, float y, float w, float h);

    float get_current_hue_by_degree() const { return current_degree; }

protected:
    float        current_degree = 0.0f;
    IGDIElement* gdi_element = nullptr;
    IGDIPaint*   gdi_paint = nullptr;
};

} // namespace skr::gui
