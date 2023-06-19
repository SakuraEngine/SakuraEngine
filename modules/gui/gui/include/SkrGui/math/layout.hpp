#pragma once
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/box_constraint.hpp"
#include "SkrGui/math/positional.hpp"

namespace skr::gui
{
// 文字布局朝向
enum class TextDirection
{
    LTR,
    RTL,
};
} // namespace skr::gui

// flex
namespace skr::gui
{
// Defines the direction in which the flex container's children are laid out.
enum class FlexDirection
{
    Row,          // Children are laid out horizontally from left to right.
    RowReverse,   // Children are laid out horizontally from right to left.
    Column,       // Children are laid out vertically from top to bottom.
    ColumnReverse // Children are laid out vertically from bottom to top.
};

// Defines how the children are distributed along the main axis of the flex container.
enum class JustifyContent
{
    FlexStart,    // Children are packed at the start of the main axis.
    FlexEnd,      // Children are packed at the end of the main axis.
    Center,       // Children are centered along the main axis.
    SpaceBetween, // Children are evenly distributed with the first child at the start and the last child at the end.
    SpaceAround,  // Children are evenly distributed with equal space around them.
    SpaceEvenly   // Children are evenly distributed with equal space between them.
};

// Defines how the children are aligned along the cross axis of the flex container.
enum class AlignItems
{
    FlexStart, // Children are aligned at the start of the cross axis.
    FlexEnd,   // Children are aligned at the end of the cross axis.
    Center,    // Children are centered along the cross axis.
    Stretch,   // Children are stretched to fill the cross axis.
    Baseline   // Children are aligned based on their baseline.
};

enum class FlexFit
{
    Tight,
    Loose,
};
} // namespace skr::gui