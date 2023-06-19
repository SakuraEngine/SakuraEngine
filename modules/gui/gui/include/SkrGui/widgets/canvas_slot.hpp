#pragma once
#include "SkrGui/framework/widget/slot_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API CanvasSlot : public SlotWidget {
    SKR_GUI_TYPE(CanvasSlot, "5f77ec84-857f-43bf-ba21-b98691545271", SlotWidget)

    Positional positional = Positional::Fill();
    int32_t    z_index = 0;
};
} // namespace skr::gui