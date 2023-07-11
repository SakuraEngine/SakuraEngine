#pragma once
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/box_constraint.hpp"
#include "SkrGui/math/positional.hpp"
#include "SkrGui/math/flex_layout.hpp"
#include "SkrGui/math/stack_layout.hpp"

namespace skr::gui
{
// 文字布局朝向
enum class TextDirection : uint8_t
{
    LTR,
    RTL,
};
} // namespace skr::gui