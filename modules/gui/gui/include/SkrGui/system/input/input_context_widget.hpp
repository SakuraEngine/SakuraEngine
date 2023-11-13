#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/system/input/input_context.hpp"
#include "SkrGui/framework/widget/stateful_widget.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/input_context_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
// 作为 InputContext 干涉控件树的入口
sreflect_struct("guid": "7f7e0555-c7da-423f-a4cd-f4d8193aa0eb")
SKR_GUI_API InputContextWidget : public StatefulWidget {
};
} // namespace gui sreflect
} // namespace skr sreflect