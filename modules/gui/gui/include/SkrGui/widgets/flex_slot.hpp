#pragma once
#include "SkrGui/framework/widget/slot_widget.hpp"
#include "SkrGui/math/layout.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/flex_slot.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "32dc9243-8d16-4854-9ce8-3039a47991bf",
    "rtti": true
)
SKR_GUI_API FlexSlot : public SlotWidget
{
    SKR_RTTR_GENERATE_BODY()

    float    flex     = 1;               // determines how much the child should grow or shrink relative to other flex items
    EFlexFit flex_fit = EFlexFit::Loose; // determines how much the child should be allowed to shrink relative to its own size
};
} // namespace gui sreflect
} // namespace skr sreflect