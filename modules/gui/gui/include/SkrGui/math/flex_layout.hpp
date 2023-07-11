#pragma once
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
// Defines the direction in which the flex container's children are laid out.
enum class EFlexDirection : uint8_t
{
    Row,          // Children are laid out horizontally from left to right.
    RowReverse,   // Children are laid out horizontally from right to left.
    Column,       // Children are laid out vertically from top to bottom.
    ColumnReverse // Children are laid out vertically from bottom to top.
};

// Defines how the children are distributed along the main axis of the flex container.
enum class EMainAxisAlignment : uint8_t
{
    Start,        // Children are packed at the start of the main axis.
    End,          // Children are packed at the end of the main axis.
    Center,       // Children are centered along the main axis.
    SpaceBetween, // Children are evenly distributed with the first child at the start and the last child at the end.
    SpaceAround,  // Children are evenly distributed with equal space around them.
    SpaceEvenly   // Children are evenly distributed with equal space between them.
};

// Defines how the children are aligned along the cross axis of the flex container.
enum class ECrossAxisAlignment : uint8_t
{
    Start,   // Children are aligned at the start of the cross axis.
    End,     // Children are aligned at the end of the cross axis.
    Center,  // Children are centered along the cross axis.
    Stretch, // Children are stretched to fill the cross axis.
    Baseline // Children are aligned based on their baseline.
};

// Main axis 如何决定自己的尺寸
enum class EMainAxisSize : uint8_t
{
    Min, // 不允许缝隙的存在
    Max, // 允许缝隙的存在
};

// Flex 的 Child 约束传递方式
enum class EFlexFit : uint8_t
{
    Tight, // min = max = max
    Loose, // min = 0.0f
};
} // namespace skr::gui