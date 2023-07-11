#pragma once
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
// 在父子 Constraints 传递的过程中，Positional 可以覆写 Constraints，但是无法对其 min-max 模式进行修改
// 这里决定了如何处理 min-max
// constraints 传递流程如下
// parent-constraints ==> padding/child-constraints ==> child.layout()
enum class EPositionalFit : uint8_t
{
    Loose,       // min = 0.0f
    Expand,      // min = max = max
    PassThrough, // non modify
};

// Canvas 如何决定自己的大小
enum class EStackSize : uint8_t
{
    Shrink, // size = max(child_size)
    Expand, // size = constraints.biggest()
};
} // namespace skr::gui