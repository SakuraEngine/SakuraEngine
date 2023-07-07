#pragma once
#include "SkrGui/framework/widget/slot_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API FlexSlot : public SlotWidget {
    SKR_GUI_OBJECT(FlexSlot, "dcfce0f5-8993-4dd1-9692-4a0fb0cfea06", SlotWidget)

    float    flex     = 1;               // determines how much the child should grow or shrink relative to other flex items
    EFlexFit flex_fit = EFlexFit::Loose; // determines how much the child should be allowed to shrink relative to its own size
};
} // namespace skr::gui