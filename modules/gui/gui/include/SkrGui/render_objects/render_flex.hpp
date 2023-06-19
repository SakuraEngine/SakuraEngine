#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{

class SKR_GUI_API RenderFlex : public RenderBox
{
public:
    SKR_GUI_TYPE(RenderFlex, "d3987dfd-24d2-478a-910e-537f24c4bae7", RenderBox);
    RenderFlex(IGDIDevice* gdi_device);

    struct Slot {
        float      flex = 1;                  // determines how much the child should grow or shrink relative to other flex items
        FlexFit    flex_fit = FlexFit::Loose; // determines how much the child should be allowed to shrink relative to its own size
        RenderBox* child = nullptr;
    };

private:
    JustifyContent justify_content = JustifyContent::FlexStart;
    FlexDirection  flex_direction = FlexDirection::Row;
    AlignItems     align_items = AlignItems::FlexStart;
    Array<Slot>    flexible_slots;
};

} // namespace skr::gui