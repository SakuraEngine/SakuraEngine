#pragma once
#include "SkrGui/math/geometry.hpp"

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
    inline constexpr Size smallest() const SKR_NOEXCEPT { return constrain({ 0, 0 }); }
    inline constexpr Size biggest() const SKR_NOEXCEPT { return constrain({ std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() }); }
    inline constexpr bool has_bounded_width() const SKR_NOEXCEPT { return max_width < std::numeric_limits<float>::infinity(); }
    inline constexpr bool has_bounded_height() const SKR_NOEXCEPT { return max_height < std::numeric_limits<float>::infinity(); }
    inline constexpr bool has_infinite_width() const SKR_NOEXCEPT { return min_width >= std::numeric_limits<float>::infinity(); }
    inline constexpr bool has_infinite_height() const SKR_NOEXCEPT { return min_height >= std::numeric_limits<float>::infinity(); }
    inline constexpr bool has_tight_width() const SKR_NOEXCEPT { return min_width >= max_width; }
    inline constexpr bool has_tight_height() const SKR_NOEXCEPT { return min_height >= max_height; }
    inline constexpr bool is_tight() const SKR_NOEXCEPT { return has_tight_width() && has_tight_height(); }

    // compare
    inline constexpr bool operator==(const BoxConstraint& rhs) const SKR_NOEXCEPT
    {
        return min_width == rhs.min_width && max_width == rhs.max_width && min_height == rhs.min_height && max_height == rhs.max_height;
    }
    inline constexpr bool operator!=(const BoxConstraint& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // ops
    inline constexpr float constrain_width(float width = std::numeric_limits<float>::infinity()) const SKR_NOEXCEPT
    {
        return std::clamp(width, min_width, max_width);
    }
    inline constexpr float constrain_height(float height = std::numeric_limits<float>::infinity()) const SKR_NOEXCEPT
    {
        return std::clamp(height, min_height, max_height);
    }
    inline constexpr Size constrain(Size size) const SKR_NOEXCEPT
    {
        size.width = constrain_width(size.width);
        size.height = constrain_height(size.height);
        return size;
    }
    inline BoxConstraint enforce(BoxConstraint constraints) const
    {
        return {
            std::clamp(min_width, constraints.min_width, constraints.max_width),
            std::clamp(max_width, constraints.min_width, constraints.max_width),
            std::clamp(min_height, constraints.min_height, constraints.max_height),
            std::clamp(max_height, constraints.min_height, constraints.max_height),
        };
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

    float   value = 0;
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
    inline constexpr bool operator==(const PositionalUnit& rhs) const SKR_NOEXCEPT { return type == rhs.type && (type == Unset || value == rhs.value); }
    inline constexpr bool operator!=(const PositionalUnit& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

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
    Offset pivot = { 0, 0 }; // percent of child size

    // factory
    inline constexpr static Positional Fill()
    {
        return {
            0,
            0,
            0,
            0,
            PositionalUnit::null(),
            PositionalUnit::null(),
            PositionalUnit::null(),
            PositionalUnit::null(),
            { 0, 0 }
        };
    }

    // setter
    struct PaddingBuilder {
        inline constexpr PaddingBuilder& all(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.left = _positional.top = _positional.right = _positional.bottom = value;
            return *this;
        }
        inline constexpr PaddingBuilder& horizontal(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.left = _positional.right = value;
            return *this;
        }
        inline constexpr PaddingBuilder& vertical(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.top = _positional.bottom = value;
            return *this;
        }
        inline constexpr PaddingBuilder& left(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.left = value;
            return *this;
        }
        inline constexpr PaddingBuilder& top(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.top = value;
            return *this;
        }
        inline constexpr PaddingBuilder& right(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.right = value;
            return *this;
        }
        inline constexpr PaddingBuilder& bottom(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.bottom = value;
            return *this;
        }
        inline constexpr PaddingBuilder& width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = value;
            return *this;
        }
        inline constexpr PaddingBuilder& height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = _positional.max_height = value;
            return *this;
        }
        inline constexpr PaddingBuilder& width_loose(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = value;
            return *this;
        }
        inline constexpr PaddingBuilder& height_loose(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = 0;
            _positional.max_height = value;
            return *this;
        }
        inline constexpr PaddingBuilder& min_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = value;
            return *this;
        }
        inline constexpr PaddingBuilder& min_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = value;
            return *this;
        }
        inline constexpr PaddingBuilder& max_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.max_width = value;
            return *this;
        }
        inline constexpr PaddingBuilder& max_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.max_height = value;
            return *this;
        }

    private:
        friend struct Positional;
        inline constexpr PaddingBuilder(Positional& positional) SKR_NOEXCEPT
            : _positional(positional)
        {
        }
        PaddingBuilder(const PaddingBuilder&) = delete;
        PaddingBuilder(PaddingBuilder&&) = delete;
        PaddingBuilder& operator=(const PaddingBuilder&) = delete;
        PaddingBuilder& operator=(PaddingBuilder&&) = delete;

        Positional& _positional;
    };
    struct AnchorBuilder {
        inline constexpr AnchorBuilder& sized(PositionalUnit width, PositionalUnit height) SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = width;
            _positional.min_height = _positional.max_height = height;
            return *this;
        }
        inline constexpr AnchorBuilder& width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = value;
            return *this;
        }
        inline constexpr AnchorBuilder& height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = _positional.max_height = value;
            return *this;
        }
        inline constexpr AnchorBuilder& loose(PositionalUnit width, PositionalUnit height) SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = width;
            _positional.min_height = 0;
            _positional.max_height = height;
            return *this;
        }
        inline constexpr AnchorBuilder& loose_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = value;
            return *this;
        }
        inline constexpr AnchorBuilder& loose_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = 0;
            _positional.max_height = value;
            return *this;
        }
        inline constexpr AnchorBuilder& loose_inf() SKR_NOEXCEPT
        {
            _positional.min_width = _positional.min_height = 0;
            _positional.max_width = _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AnchorBuilder& loose_inf_width() SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AnchorBuilder& loose_inf_height() SKR_NOEXCEPT
        {
            _positional.min_height = 0;
            _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AnchorBuilder& expand() SKR_NOEXCEPT
        {
            _positional.min_width = _positional.min_height = _positional.max_width = _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AnchorBuilder& expand_width() SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AnchorBuilder& expand_height() SKR_NOEXCEPT
        {
            _positional.min_height = _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AnchorBuilder& min_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = value;
            return *this;
        }
        inline constexpr AnchorBuilder& min_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = value;
            return *this;
        }
        inline constexpr AnchorBuilder& max_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.max_width = value;
            return *this;
        }
        inline constexpr AnchorBuilder& max_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.max_height = value;
            return *this;
        }
        inline constexpr AnchorBuilder& pivot(Offset value) SKR_NOEXCEPT
        {
            _positional.pivot = value;
            return *this;
        }

    private:
        friend struct Positional;
        inline constexpr AnchorBuilder(Positional& positional) SKR_NOEXCEPT
            : _positional(positional)
        {
        }
        AnchorBuilder(const AnchorBuilder&) = delete;
        AnchorBuilder(AnchorBuilder&&) = delete;
        AnchorBuilder& operator=(const AnchorBuilder&) = delete;
        AnchorBuilder& operator=(AnchorBuilder&&) = delete;

        Positional& _positional;
    };
    struct AlignBuilder {
        inline constexpr AlignBuilder(Positional& positional) SKR_NOEXCEPT
            : _positional(positional)
        {
        }
        inline constexpr AlignBuilder& sized(PositionalUnit width, PositionalUnit height) SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = width;
            _positional.min_height = _positional.max_height = height;
            return *this;
        }
        inline constexpr AlignBuilder& width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = value;
            return *this;
        }
        inline constexpr AlignBuilder& height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = _positional.max_height = value;
            return *this;
        }
        inline constexpr AlignBuilder& loose(PositionalUnit width, PositionalUnit height) SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = width;
            _positional.min_height = 0;
            _positional.max_height = height;
            return *this;
        }
        inline constexpr AlignBuilder& loose_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = value;
            return *this;
        }
        inline constexpr AlignBuilder& loose_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = 0;
            _positional.max_height = value;
            return *this;
        }
        inline constexpr AlignBuilder& loose_inf() SKR_NOEXCEPT
        {
            _positional.min_width = _positional.min_height = 0;
            _positional.max_width = _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AlignBuilder& loose_inf_width() SKR_NOEXCEPT
        {
            _positional.min_width = 0;
            _positional.max_width = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AlignBuilder& loose_inf_height() SKR_NOEXCEPT
        {
            _positional.min_height = 0;
            _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AlignBuilder& expand() SKR_NOEXCEPT
        {
            _positional.min_width = _positional.min_height = _positional.max_width = _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AlignBuilder& expand_width() SKR_NOEXCEPT
        {
            _positional.min_width = _positional.max_width = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AlignBuilder& expand_height() SKR_NOEXCEPT
        {
            _positional.min_height = _positional.max_height = std::numeric_limits<PositionalUnit>::infinity();
            return *this;
        }
        inline constexpr AlignBuilder& min_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_width = value;
            return *this;
        }
        inline constexpr AlignBuilder& min_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.min_height = value;
            return *this;
        }
        inline constexpr AlignBuilder& max_width(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.max_width = value;
            return *this;
        }
        inline constexpr AlignBuilder& max_height(PositionalUnit value) SKR_NOEXCEPT
        {
            _positional.max_height = value;
            return *this;
        }

    private:
        Positional& _positional;
    };
    inline constexpr void           fill() SKR_NOEXCEPT { left = top = right = bottom = 0; }
    inline constexpr PaddingBuilder padding() SKR_NOEXCEPT { return PaddingBuilder(*this); }
    inline constexpr AnchorBuilder  anchor_LT(PositionalUnit l, PositionalUnit t) SKR_NOEXCEPT
    {
        left = l;
        top = t;
        return AnchorBuilder(*this);
    }
    inline constexpr AnchorBuilder anchor_RT(PositionalUnit r, PositionalUnit t) SKR_NOEXCEPT
    {
        right = r;
        top = t;
        return AnchorBuilder(*this);
    }
    inline constexpr AnchorBuilder anchor_LB(PositionalUnit l, PositionalUnit b) SKR_NOEXCEPT
    {
        left = l;
        bottom = b;
        return AnchorBuilder(*this);
    }
    inline constexpr AnchorBuilder anchor_RB(PositionalUnit r, PositionalUnit b) SKR_NOEXCEPT
    {
        right = r;
        bottom = b;
        return AnchorBuilder(*this);
    }
    inline constexpr AlignBuilder align(Offset align_point) SKR_NOEXCEPT
    {
        left = PositionalUnit::pct(align_point.x);
        top = PositionalUnit::pct(align_point.y);
        pivot = align_point;
        return AlignBuilder(*this);
    }
    inline constexpr AlignBuilder left_top() SKR_NOEXCEPT { return align({ 0, 0 }); }
    inline constexpr AlignBuilder center_top() SKR_NOEXCEPT { return align({ 0.5, 0 }); }
    inline constexpr AlignBuilder right_top() SKR_NOEXCEPT { return align({ 1, 0 }); }
    inline constexpr AlignBuilder left_center() SKR_NOEXCEPT { return align({ 0, 0.5 }); }
    inline constexpr AlignBuilder center() SKR_NOEXCEPT { return align({ 0.5, 0.5 }); }
    inline constexpr AlignBuilder right_center() SKR_NOEXCEPT { return align({ 1, 0.5 }); }
    inline constexpr AlignBuilder left_bottom() SKR_NOEXCEPT { return align({ 0, 1 }); }
    inline constexpr AlignBuilder center_bottom() SKR_NOEXCEPT { return align({ 0.5, 1 }); }
    inline constexpr AlignBuilder right_bottom() SKR_NOEXCEPT { return align({ 1, 1 }); }

    // checker
    inline constexpr bool with_constraints() const SKR_NOEXCEPT
    {
        return min_width.has_value() || max_width.has_value() || min_height.has_value() || max_height.has_value();
    }
    inline constexpr bool with_height_constraints() const SKR_NOEXCEPT
    {
        return min_height.has_value() || max_height.has_value();
    }
    inline constexpr bool with_width_constraints() const SKR_NOEXCEPT
    {
        return min_width.has_value() && max_width.has_value();
    }
    inline constexpr bool is_width_padding() const SKR_NOEXCEPT
    {
        return left.has_value() && right.has_value();
    }
    inline constexpr bool is_height_padding() const SKR_NOEXCEPT
    {
        return top.has_value() && bottom.has_value();
    }
    inline constexpr bool is_padding() const SKR_NOEXCEPT
    {
        return left.has_value() && right.has_value() && top.has_value() && bottom.has_value();
    }
    inline constexpr bool is_width_anchor() const SKR_NOEXCEPT
    {
        return (left.has_value() + right.has_value()) == 1;
    }
    inline constexpr bool is_height_anchor() const SKR_NOEXCEPT
    {
        return (top.has_value() + bottom.has_value()) == 1;
    }
    inline constexpr bool is_anchor() const SKR_NOEXCEPT
    {
        return is_width_anchor() && is_height_anchor();
    }
    inline constexpr bool is_valid() const SKR_NOEXCEPT
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
    inline constexpr bool is_width_valid() const SKR_NOEXCEPT
    {
        return (left.is_null() || right.is_null() || !with_width_constraints());
    }
    inline constexpr bool is_height_valid() const SKR_NOEXCEPT
    {
        return (top.is_null() || bottom.is_null() || !with_height_constraints());
    }

    // setter
    inline constexpr void clear_constraints() SKR_NOEXCEPT
    {
        min_width = max_width = min_height = max_height = PositionalUnit::null();
    }

    // compare
    inline constexpr bool operator==(const Positional& rhs) const SKR_NOEXCEPT
    {
        return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom &&
               min_width == rhs.min_width && min_height == rhs.min_height && max_width == rhs.max_width &&
               max_height == rhs.max_height && pivot == rhs.pivot;
    }
    inline constexpr bool operator!=(const Positional& rhs) const SKR_NOEXCEPT
    {
        return !(*this == rhs);
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