#pragma once
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/box_constraint.hpp"

namespace skr::gui
{
struct PositionalUnit {
    float pixel   = 0;
    float percent = 0;

private:
    bool is_set = false;

public:
    // construct
    inline constexpr PositionalUnit() SKR_NOEXCEPT = default;
    inline constexpr PositionalUnit(float pixel) SKR_NOEXCEPT
        : pixel(pixel),
          is_set(true)
    {
    }
    inline constexpr PositionalUnit(float pixel, float percent) SKR_NOEXCEPT
        : pixel(pixel),
          percent(percent),
          is_set(true)
    {
    }

    // is setted
    inline constexpr bool is_null() const SKR_NOEXCEPT { return !is_set; }
    inline constexpr bool has_value() const SKR_NOEXCEPT { return is_set; }
    inline constexpr void make_null() SKR_NOEXCEPT { is_set = false; }

    // compare
    inline constexpr operator bool() const SKR_NOEXCEPT { return has_value(); }
    inline constexpr bool operator==(std::nullptr_t) const SKR_NOEXCEPT { return is_null(); }
    inline constexpr bool operator!=(std::nullptr_t) const SKR_NOEXCEPT { return has_value(); }
    inline constexpr bool operator==(const PositionalUnit& rhs) const SKR_NOEXCEPT
    {
        return (is_null() && rhs.is_null()) || (has_value() && rhs.has_value() && pixel == rhs.pixel && percent == rhs.percent);
    }
    inline constexpr bool operator!=(const PositionalUnit& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // constant
    inline static constexpr PositionalUnit null() SKR_NOEXCEPT { return PositionalUnit{}; }

    // factory
    inline static constexpr PositionalUnit px(float value) SKR_NOEXCEPT
    {
        return PositionalUnit{ value, 0 };
    }
    inline static constexpr PositionalUnit pct(float value) SKR_NOEXCEPT
    {
        return PositionalUnit{ 0, value };
    }
    inline static constexpr PositionalUnit mixed(float pixel, float percent) SKR_NOEXCEPT
    {
        return PositionalUnit{ pixel, percent };
    }

    // operator
    inline PositionalUnit operator-()
    {
        if (is_null()) { SKR_GUI_LOG_ERROR(u8"can't negate a null value"); }
        return PositionalUnit{ -pixel, -percent };
    }
    inline PositionalUnit operator+(const PositionalUnit& rhs) const
    {
        if (is_null() || rhs.is_null()) { SKR_GUI_LOG_ERROR(u8"can't add a null value"); }
        return PositionalUnit{ pixel + rhs.pixel, percent + rhs.percent };
    }
    inline PositionalUnit operator-(const PositionalUnit& rhs) const
    {
        if (is_null() || rhs.is_null()) { SKR_GUI_LOG_ERROR(u8"can't subtract a null value"); }
        return PositionalUnit{ pixel - rhs.pixel, percent - rhs.percent };
    }
    inline PositionalUnit& operator+=(const PositionalUnit& rhs)
    {
        if (is_null() || rhs.is_null()) { SKR_GUI_LOG_ERROR(u8"can't add a null value"); }
        pixel += rhs.pixel;
        percent += rhs.percent;
        return *this;
    }
    inline PositionalUnit& operator-=(const PositionalUnit& rhs)
    {
        if (is_null() || rhs.is_null()) { SKR_GUI_LOG_ERROR(u8"can't subtract a null value"); }
        pixel -= rhs.pixel;
        percent -= rhs.percent;
        return *this;
    }

    // resolve
    inline float resolve(float parent_value) const SKR_NOEXCEPT
    {
        if (is_null()) { SKR_GUI_LOG_ERROR(u8"can't resolve from a null value"); }
        return pixel + percent * parent_value;
    }
    inline float inflate(float child_value) const SKR_NOEXCEPT
    {
        if (is_null()) { SKR_GUI_LOG_ERROR(u8"can't inflate from a null value"); }
        float child_pct   = 1 - percent;
        float inner_value = child_value + pixel;
        return child_pct <= 0 ? std::numeric_limits<float>::infinity() : inner_value / child_pct;
    }
};

// float literal
inline constexpr PositionalUnit operator""_px(long double value) SKR_NOEXCEPT { return PositionalUnit::px(static_cast<float>(value)); }
inline constexpr PositionalUnit operator""_pct(long double value) SKR_NOEXCEPT { return PositionalUnit::pct(static_cast<float>(value)); }

// int literal
inline constexpr PositionalUnit operator""_px(unsigned long long value) SKR_NOEXCEPT { return PositionalUnit::px(static_cast<float>(value)); }
inline constexpr PositionalUnit operator""_pct(unsigned long long value) SKR_NOEXCEPT { return PositionalUnit::pct(static_cast<float>(value)); }

// 布局定位 + 约束
struct Positional {
    // 约束定位 or 锚点定位
    PositionalUnit left   = PositionalUnit::null(); // constraints offset (both min and max)
    PositionalUnit top    = PositionalUnit::null(); // constraints offset (both min and max)
    PositionalUnit right  = PositionalUnit::null(); // constraints offset (both min and max)
    PositionalUnit bottom = PositionalUnit::null(); // constraints offset (both min and max)

    // 锚点定位中使用的尺寸约束
    PositionalUnit min_width  = PositionalUnit::null(); // min width constraints override(px) or scale(pct)
    PositionalUnit max_width  = PositionalUnit::null(); // max width constraints override(px) or scale(pct)
    PositionalUnit min_height = PositionalUnit::null(); // min height constraints override(px) or scale(pct)
    PositionalUnit max_height = PositionalUnit::null(); // max height constraints override(px) of scale(pct)

    // 锚点
    Alignment pivot = { 0, 0 }; // child pivot postion (in percent)

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

    // builder
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
        PaddingBuilder(const PaddingBuilder&)            = delete;
        PaddingBuilder(PaddingBuilder&&)                 = delete;
        PaddingBuilder& operator=(const PaddingBuilder&) = delete;
        PaddingBuilder& operator=(PaddingBuilder&&)      = delete;

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
            _positional.min_width  = 0;
            _positional.max_width  = width;
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
        inline constexpr AnchorBuilder& pivot(Alignment value) SKR_NOEXCEPT
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
        AnchorBuilder(const AnchorBuilder&)            = delete;
        AnchorBuilder(AnchorBuilder&&)                 = delete;
        AnchorBuilder& operator=(const AnchorBuilder&) = delete;
        AnchorBuilder& operator=(AnchorBuilder&&)      = delete;

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
            _positional.min_width  = 0;
            _positional.max_width  = width;
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
        top  = t;
        return AnchorBuilder(*this);
    }
    inline constexpr AnchorBuilder anchor_RT(PositionalUnit r, PositionalUnit t) SKR_NOEXCEPT
    {
        right = r;
        top   = t;
        return AnchorBuilder(*this);
    }
    inline constexpr AnchorBuilder anchor_LB(PositionalUnit l, PositionalUnit b) SKR_NOEXCEPT
    {
        left   = l;
        bottom = b;
        return AnchorBuilder(*this);
    }
    inline constexpr AnchorBuilder anchor_RB(PositionalUnit r, PositionalUnit b) SKR_NOEXCEPT
    {
        right  = r;
        bottom = b;
        return AnchorBuilder(*this);
    }
    inline constexpr AlignBuilder align(Alignment align_point) SKR_NOEXCEPT
    {
        left  = PositionalUnit::pct(align_point.x);
        top   = PositionalUnit::pct(align_point.y);
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
    inline constexpr bool has_constraints() const SKR_NOEXCEPT
    {
        return min_width.has_value() || max_width.has_value() || min_height.has_value() || max_height.has_value();
    }
    inline constexpr bool has_height_constraints() const SKR_NOEXCEPT
    {
        return min_height.has_value() || max_height.has_value();
    }
    inline constexpr bool has_width_constraints() const SKR_NOEXCEPT
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
        return (left.has_value() + right.has_value()) <= 1;
    }
    inline constexpr bool is_height_anchor() const SKR_NOEXCEPT
    {
        return (top.has_value() + bottom.has_value()) <= 1;
    }
    inline constexpr bool is_anchor() const SKR_NOEXCEPT
    {
        return is_width_anchor() && is_height_anchor();
    }
    inline constexpr bool is_valid() const SKR_NOEXCEPT
    {
        return (left.is_null() || right.is_null() || !has_width_constraints()) &&
               (top.is_null() || bottom.is_null() || !has_height_constraints());
    }
    inline constexpr bool is_width_valid() const SKR_NOEXCEPT
    {
        return (left.is_null() || right.is_null() || !has_width_constraints());
    }
    inline constexpr bool is_height_valid() const SKR_NOEXCEPT
    {
        return (top.is_null() || bottom.is_null() || !has_height_constraints());
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

    // layout
    inline BoxConstraints resolve_constraints(const BoxConstraints& parent_constraints) const SKR_NOEXCEPT
    {
        // 约束逻辑，以横轴为例，竖轴同理：
        //     padding mode: [left && right] -> width = parent_width - left - right
        //     anchor mode: 根据 min_width max_width 的设置情况传递约束，如果都没设则不做约束

        BoxConstraints result = {};

        // make horizontal constraints
        if (is_width_padding())
        {
            if (has_width_constraints())
            {
                SKR_GUI_LOG_WARN(u8"Both left and right are set, width will be ignored");
            }
            const float left_padding_min  = left.resolve(parent_constraints.min_width);
            const float left_padding_max  = left.resolve(parent_constraints.max_width);
            const float right_padding_min = right.resolve(parent_constraints.min_width);
            const float right_padding_max = right.resolve(parent_constraints.max_width);
            result.min_width              = std::max(0.f, parent_constraints.min_width - left_padding_min - right_padding_min);
            result.max_width              = std::max(result.min_width, parent_constraints.max_width - left_padding_max - right_padding_max);
        }
        else
        {
            if (min_width)
            {
                result.min_width = min_width.resolve(parent_constraints.min_width);
            }
            if (max_width)
            {
                result.max_width = max_width.resolve(parent_constraints.max_width);
            }
        }

        // make vertical constraints
        if (is_height_padding())
        {
            if (has_height_constraints())
            {
                SKR_GUI_LOG_WARN(u8"Both top and bottom are set, height will be ignored");
            }
            const float top_padding_min    = top.resolve(parent_constraints.min_height);
            const float top_padding_max    = top.resolve(parent_constraints.max_height);
            const float bottom_padding_min = bottom.resolve(parent_constraints.min_height);
            const float bottom_padding_max = bottom.resolve(parent_constraints.max_height);
            result.min_height              = std::max(0.f, parent_constraints.min_height - top_padding_min - bottom_padding_min);
            result.max_height              = std::max(result.min_height, parent_constraints.max_height - top_padding_max - bottom_padding_max);
        }
        else
        {
            if (min_height)
            {
                result.min_height = min_height.resolve(parent_constraints.min_height);
            }
            if (max_height)
            {
                result.max_height = max_height.resolve(parent_constraints.max_height);
            }
        }

        return result;
    }
    inline Offsetf resolve_offset(Sizef child_size, Sizef parent_size) const SKR_NOEXCEPT
    {
        float offset_x, offset_y;

        // calc horizontal
        if (is_width_padding())
        {
            offset_x = left.resolve(parent_size.width);
        }
        else
        {
            if (left)
            {
                const float anchor_x = left.resolve(parent_size.width);
                const float pivot_x  = pivot.x * child_size.width;
                offset_x             = anchor_x - pivot_x;
            }
            else if (right)
            {
                const float anchor_x = right.resolve(parent_size.width);
                const float pivot_x  = (1 - pivot.x) * child_size.width;
                offset_x             = parent_size.width - anchor_x - pivot_x;
            }
            else
            {
                SKR_GUI_LOG_WARN(u8"Both left and right are not set, default to left 0");
                const float anchor_x = 0;
                const float pivot_x  = pivot.x * child_size.width;
                offset_x             = anchor_x - pivot_x;
            }
        }

        // calc vertical
        if (is_height_padding())
        {
            offset_y = top.resolve(parent_size.height);
        }
        else
        {
            if (top)
            {
                const float anchor_y = top.resolve(parent_size.height);
                const float pivot_y  = pivot.y * child_size.height;
                offset_y             = anchor_y - pivot_y;
            }
            else if (bottom)
            {
                const float anchor_y = bottom.resolve(parent_size.height);
                const float pivot_y  = (1 - pivot.y) * child_size.height;
                offset_y             = parent_size.height - anchor_y - pivot_y;
            }
            else
            {
                SKR_GUI_LOG_WARN(u8"Both top and bottom are not set, default to top 0");
                const float anchor_y = 0;
                const float pivot_y  = pivot.y * child_size.height;
                offset_y             = anchor_y - pivot_y;
            }
        }

        return { offset_x, offset_y };
    }
    inline float resolve_padding_width(float parent_width) const SKR_NOEXCEPT
    {
        if (!is_width_padding()) { SKR_GUI_LOG_ERROR(u8"cannot resolve padding width without left and right"); }
        return left.resolve(parent_width) + right.resolve(parent_width);
    }
    inline float resolve_padding_height(float parent_height) const SKR_NOEXCEPT
    {
        if (!is_height_padding()) { SKR_GUI_LOG_ERROR(u8"cannot resolve padding height without top and bottom"); }
        return top.resolve(parent_height) + bottom.resolve(parent_height);
    }
};
} // namespace skr::gui