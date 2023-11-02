#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/stateful_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "bb7b41aa-b827-4bb2-b025-e9803938ec2e",
    "rtti": true
)
SKR_GUI_API StatefulWidget : public Widget{
    SKR_RTTR_GENERATE_BODY()
};
} // namespace gui sreflect
} // namespace skr sreflect