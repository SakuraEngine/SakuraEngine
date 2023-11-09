#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/input_context.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "b4454cc3-68b6-4ac5-b5e1-b0dc5ba25334",
    "rtti": true
)
InputContext {
    InputContext(NotNull<RenderBox*> widget);

public:
    RenderBox* widget = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect