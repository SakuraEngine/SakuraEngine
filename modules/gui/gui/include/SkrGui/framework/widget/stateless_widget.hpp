#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/stateless_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "8e344044-7b4b-4a69-b7f4-a41672f2c346",
    "rtti": true
)
SKR_GUI_API StatelessWidget : public Widget{
    SKR_RTTR_GENERATE_BODY()
};
} // namespace gui sreflect
} // namespace skr sreflect