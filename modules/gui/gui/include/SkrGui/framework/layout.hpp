#pragma once
#include "SkrGui/framework/geometry.hpp"

namespace skr::gui
{
// 文字布局朝向
enum class TextDirection
{
    LTR,
    RTL,
};

// Box 约束器
struct BoxConstraint {
    float min_width = 0;
    float max_width = std::numeric_limits<float>::infinity();
    float min_height = 0;
    float max_height = std::numeric_limits<float>::infinity();

    // factory
    inline static BoxConstraint Sized(Size size) SKR_NOEXCEPT
    {
        return {
            size.width,
            size.width,
            size.height,
            size.height,
        };
    }
    inline static BoxConstraint Loose(Size size) SKR_NOEXCEPT
    {
        return {
            0,
            size.width,
            0,
            size.height,
        };
    }
    inline static BoxConstraint Expand(Optional<float> width = {}, Optional<float> height = {})
    {
        return {
            width ? width.get() : std::numeric_limits<float>::infinity(),
            width ? width.get() : std::numeric_limits<float>::infinity(),
            height ? height.get() : std::numeric_limits<float>::infinity(),
            height ? height.get() : std::numeric_limits<float>::infinity(),
        };
    }

    // getter setter
    inline constexpr Size min_size() const SKR_NOEXCEPT
    {
        return { min_width, min_height };
    }
    inline constexpr Size max_size() const SKR_NOEXCEPT { return { max_width, max_height }; }
    inline constexpr void set_min_size(Size size) SKR_NOEXCEPT
    {
        min_width = size.width;
        min_height = size.height;
    }
    inline constexpr void set_max_size(Size size) SKR_NOEXCEPT
    {
        max_width = size.width;
        max_height = size.height;
    }

    // ops
    inline constexpr float constrain_width(float width) const SKR_NOEXCEPT
    {
        return std::clamp(width, min_width, max_width);
    }
    inline constexpr float constrain_height(float height) const SKR_NOEXCEPT
    {
        return std::clamp(height, min_height, max_height);
    }
    inline constexpr Size constrain(Size size) const SKR_NOEXCEPT
    {
        size.width = constrain_width(size.width);
        size.height = constrain_height(size.height);
        return size;
    }
};

// 布局定位使用的基本单位，在实现 Optional 功能的同时
struct PositionalUnit {
    enum Type
    {
        Unset,
        Pixel,
        Percent,
    };

    float value = 0;
    uint8_t type = Type::Unset;

    // construct
    constexpr PositionalUnit() = default;
    constexpr PositionalUnit(float value)
        : value(value)
        , type(Type::Pixel)
    {
    }
    constexpr PositionalUnit(float value, Type type)
        : value(value)
        , type(type)
    {
    }

    // type
    inline constexpr bool is_null() const SKR_NOEXCEPT { return type == Type::Unset; }
    inline constexpr bool has_value() const SKR_NOEXCEPT { return type != Type::Unset; }
    inline constexpr bool is_px() const SKR_NOEXCEPT { return type == Type::Pixel; }
    inline constexpr bool is_pct() const SKR_NOEXCEPT { return type == Type::Percent; }

    // compare
    inline constexpr operator bool() const SKR_NOEXCEPT { return type != Type::Unset; }
    inline constexpr bool operator==(std::nullptr_t) const SKR_NOEXCEPT { return type == Type::Unset; }
    inline constexpr bool operator!=(std::nullptr_t) const SKR_NOEXCEPT { return type != Type::Unset; }

    // factory
    inline static constexpr PositionalUnit null() SKR_NOEXCEPT { return PositionalUnit{}; }
    inline static constexpr PositionalUnit px(float value) SKR_NOEXCEPT { return PositionalUnit{ value, Type::Pixel }; }
    inline static constexpr PositionalUnit pct(float value) SKR_NOEXCEPT { return PositionalUnit{ value, Type::Percent }; }

    // calc
    inline constexpr float get_pixel(float parent_value) const SKR_NOEXCEPT
    {
        if (type == Type::Percent)
        {
            return parent_value * value;
        }
        else
        {
            return value;
        }
    }
};

// float literal
constexpr PositionalUnit operator""_px(long double value) SKR_NOEXCEPT { return PositionalUnit::px(static_cast<float>(value)); }
constexpr PositionalUnit operator""_pct(long double value) SKR_NOEXCEPT { return PositionalUnit::pct(static_cast<float>(value)); }

// int literal
constexpr PositionalUnit operator""_px(unsigned long long value) SKR_NOEXCEPT { return PositionalUnit::px(static_cast<float>(value)); }
constexpr PositionalUnit operator""_pct(unsigned long long value) SKR_NOEXCEPT { return PositionalUnit::pct(static_cast<float>(value)); }

// 布局定位 + 约束
struct Positional {
    // 约束定位 or 锚点定位
    PositionalUnit left = PositionalUnit::null();
    PositionalUnit top = PositionalUnit::null();
    PositionalUnit right = PositionalUnit::null();
    PositionalUnit bottom = PositionalUnit::null();

    // 锚点定位中使用的尺寸约束
    PositionalUnit min_width = PositionalUnit::null();  // as 0 if needs
    PositionalUnit max_width = PositionalUnit::null();  // as inf if needs
    PositionalUnit min_height = PositionalUnit::null(); // as o if needs
    PositionalUnit max_height = PositionalUnit::null(); // as inf if needs

    // 锚点
    Offset pivot = { 0, 0 };

    // constants
    inline static constexpr Positional fill() SKR_NOEXCEPT { return { 0, 0, 0, 0 }; }

    // factory
    struct PaddingParams {
        PositionalUnit all = PositionalUnit::null();

        PositionalUnit horizontal = PositionalUnit::null();
        PositionalUnit vertical = PositionalUnit::null();

        PositionalUnit left = PositionalUnit::null();
        PositionalUnit top = PositionalUnit::null();
        PositionalUnit right = PositionalUnit::null();
        PositionalUnit bottom = PositionalUnit::null();

        inline constexpr bool is_valid() SKR_NOEXCEPT
        {
            // all 与其它所有互斥
            // horizontal 与 left、right 互斥
            // vertical 与 top、bottom 互斥
            return (all.has_value() && (horizontal.is_null() && vertical.is_null() && left.is_null() && top.is_null() && right.is_null() && bottom.is_null())) ||
                   (all.is_null() && (horizontal.has_value() && left.is_null() && top.is_null()) && (vertical.has_value() && right.is_null() && bottom.is_null()));
        }
    };
    struct ConstraintsParams {
        PositionalUnit width = PositionalUnit::null();
        PositionalUnit height = PositionalUnit::null();

        PositionalUnit min_width = PositionalUnit::null();
        PositionalUnit max_width = PositionalUnit::null();
        PositionalUnit min_height = PositionalUnit::null();
        PositionalUnit max_height = PositionalUnit::null();
        inline constexpr bool is_valid() SKR_NOEXCEPT
        {
            // width 与 min_width、max_width 互斥
            // height 与 min_height、max_height 互斥
            return (width.has_value() && (min_width.is_null() && max_width.is_null())) ||
                   (height.has_value() && (min_height.is_null() && max_height.is_null()));
        }
    };
    struct PivotParams {
        PositionalUnit left = PositionalUnit::null();
        PositionalUnit top = PositionalUnit::null();
        PositionalUnit right = PositionalUnit::null();
        PositionalUnit bottom = PositionalUnit::null();
        inline constexpr bool is_valid() SKR_NOEXCEPT
        {
            // left 与 right 互斥，定位横向锚点
            // top 与 bottom 互斥，定位纵向锚点
            return ((left.has_value() + right.has_value()) == 1) &&
                   ((top.has_value() + bottom.has_value()) == 1);
        }
    };
    inline constexpr static Positional Padding(PaddingParams params) SKR_NOEXCEPT
    {
        Positional result;
        if (params.all.has_value())
        {
            result.left = result.top = result.right = result.bottom = params.all;
        }
        else
        {
            if (params.horizontal.has_value())
            {
                result.left = result.right = params.horizontal;
            }
            else
            {
                result.left = params.left;
                result.right = params.right;
            }
            if (params.vertical.has_value())
            {
                result.top = result.bottom = params.vertical;
            }
            else
            {
                result.top = params.top;
                result.bottom = params.bottom;
            }
        }
        return result;
    };
    inline constexpr static Positional Anchor(PivotParams parent_pivot, ConstraintsParams constraints, Offset child_pivot) SKR_NOEXCEPT
    {
        Positional result;

        // parent pivot
        if (parent_pivot.left) // protect for invalid value
        {
            result.left = parent_pivot.left;
        }
        else
        {
            result.right = parent_pivot.right;
        }
        if (parent_pivot.top) // protect for invalid value
        {
            result.top = parent_pivot.top;
        }
        else
        {
            result.bottom = parent_pivot.bottom;
        }

        // child pivot
        result.pivot = child_pivot;

        // size
        if (constraints.width)
        {
            result.min_width = result.max_width = constraints.width;
        }
        else
        {
            result.min_width = constraints.min_width;
            result.max_width = constraints.max_width;
        }
        if (constraints.height)
        {
            result.min_height = result.max_height = constraints.height;
        }
        else
        {
            result.min_height = constraints.min_height;
            result.max_height = constraints.max_height;
        }

        return result;
    }

    // checker
    inline constexpr bool with_constraints() SKR_NOEXCEPT
    {
        return min_width.has_value() || max_width.has_value() || min_height.has_value() || max_height.has_value();
    }
    inline constexpr bool with_height_constraints() SKR_NOEXCEPT
    {
        return min_height.has_value() || max_height.has_value();
    }
    inline constexpr bool with_width_constraints() SKR_NOEXCEPT
    {
        return min_width.has_value() && max_width.has_value();
    }
    inline constexpr bool is_padding() SKR_NOEXCEPT
    {
        return !with_constraints();
    }
    inline constexpr bool is_anchor() SKR_NOEXCEPT
    {
        return ((left.has_value() + right.has_value()) == 1) &&
               ((top).has_value() + right.has_value()) == 1 &&
               with_constraints();
    }
    inline constexpr bool is_valid() SKR_NOEXCEPT
    {
        // padding mode: [left] + [right]
        // anchor mode: [left/right] + [width constraints] + [pivot.x]
        // anchor mode fallback: 未设父锚点的情况下，使用 0_pct 作为父锚点
        // 无约束 0 点对齐: 全空
        // error: 全有值
        // 横竖两轴可以使用各自的模式来完成布局
        return (left.is_null() || right.is_null() || !with_width_constraints()) &&
               (top.is_null() || bottom.is_null() || !with_height_constraints());
    }

    // setter
    inline constexpr void clear_constraints() SKR_NOEXCEPT
    {
        min_width = max_width = min_height = max_height = PositionalUnit::null();
    }
    inline constexpr void constraints_inf() SKR_NOEXCEPT
    {
        min_width = 0;
        max_width = std::numeric_limits<float>::infinity();
        min_height = 0;
        max_height = std::numeric_limits<float>::infinity();
    }
    inline constexpr void constraints_sized(PositionalUnit width, PositionalUnit height) SKR_NOEXCEPT
    {
        min_width = max_width = width;
        min_height = max_height = height;
    }
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