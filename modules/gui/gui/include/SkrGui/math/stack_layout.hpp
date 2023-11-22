#pragma once
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/math/stack_layout.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
// 在父子 Constraints 传递的过程中，Positional 可以覆写 Constraints，但是无法对其 min-max 模式进行修改
// 这里决定了如何处理 min-max
// constraints 传递流程如下
// parent-constraints ==> padding/child-constraints ==> child.layout()
sreflect_enum_class(
    "guid": "01092beb-ecd0-4292-8217-7998997a8746"
)
EPositionalFit : uint8_t
{
    Loose,       // min = 0.0f
    Expand,      // min = max = max
    PassThrough, // non modify
};

// Canvas 如何决定自己的大小
sreflect_enum_class(
    "guid": "1a43aede-a6f7-4750-8f29-da2965b9abcb"
)
EStackSize : uint8_t
{
    Shrink, // size = max(child_size)
    Expand, // size = constraints.biggest()
};
} // namespace gui sreflect
} // namespace skr sreflect