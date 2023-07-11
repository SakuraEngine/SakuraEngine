#pragma once
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
// Box 约束器
struct BoxConstraints {
    float min_width  = 0;
    float max_width  = std::numeric_limits<float>::infinity();
    float min_height = 0;
    float max_height = std::numeric_limits<float>::infinity();

    // factory
    inline static BoxConstraints Tight(Sizef size) SKR_NOEXCEPT
    {
        return {
            size.width,
            size.width,
            size.height,
            size.height,
        };
    }
    inline static BoxConstraints TightWidth(float width) SKR_NOEXCEPT
    {
        return {
            width,
            width,
            0,
            std::numeric_limits<float>::infinity(),
        };
    }
    inline static BoxConstraints TightHeight(float height) SKR_NOEXCEPT
    {
        return {
            0,
            std::numeric_limits<float>::infinity(),
            height,
            height,
        };
    }
    inline static BoxConstraints Loose(Sizef size) SKR_NOEXCEPT
    {
        return {
            0,
            size.width,
            0,
            size.height,
        };
    }
    inline static BoxConstraints LooseWidth(float width) SKR_NOEXCEPT
    {
        return {
            0,
            width,
            0,
            std::numeric_limits<float>::infinity(),
        };
    }
    inline static BoxConstraints LooseHeight(float height) SKR_NOEXCEPT
    {
        return {
            0,
            std::numeric_limits<float>::infinity(),
            0,
            height,
        };
    }
    inline static BoxConstraints Expand() SKR_NOEXCEPT
    {
        return {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
        };
    }
    inline static BoxConstraints ExpandWidth(float height) SKR_NOEXCEPT
    {
        return {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            height,
            height,
        };
    }
    inline static BoxConstraints ExpandWidth(float min_height, float max_height) SKR_NOEXCEPT
    {
        return {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            min_height,
            max_height,
        };
    }
    inline static BoxConstraints ExpandHeight(float width) SKR_NOEXCEPT
    {
        return {
            width,
            width,
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
        };
    }
    inline static BoxConstraints ExpandHeight(float min_width, float max_width) SKR_NOEXCEPT
    {
        return {
            min_width,
            max_width,
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
        };
    }

    // getter setter
    inline constexpr Sizef min_size() const SKR_NOEXCEPT
    {
        return { min_width, min_height };
    }
    inline constexpr Sizef max_size() const SKR_NOEXCEPT { return { max_width, max_height }; }
    inline constexpr void  set_min_size(Sizef size) SKR_NOEXCEPT
    {
        min_width  = size.width;
        min_height = size.height;
    }
    inline constexpr void set_max_size(Sizef size) SKR_NOEXCEPT
    {
        max_width  = size.width;
        max_height = size.height;
    }
    inline constexpr Sizef smallest() const SKR_NOEXCEPT { return constrain({ 0, 0 }); }
    inline constexpr Sizef biggest() const SKR_NOEXCEPT { return constrain({ std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() }); }
    inline constexpr bool  has_bounded_width() const SKR_NOEXCEPT { return max_width < std::numeric_limits<float>::infinity(); }
    inline constexpr bool  has_bounded_height() const SKR_NOEXCEPT { return max_height < std::numeric_limits<float>::infinity(); }
    inline constexpr bool  has_infinite_width() const SKR_NOEXCEPT { return min_width >= std::numeric_limits<float>::infinity(); }
    inline constexpr bool  has_infinite_height() const SKR_NOEXCEPT { return min_height >= std::numeric_limits<float>::infinity(); }
    inline constexpr bool  has_tight_width() const SKR_NOEXCEPT { return min_width >= max_width; }
    inline constexpr bool  has_tight_height() const SKR_NOEXCEPT { return min_height >= max_height; }
    inline constexpr bool  is_tight() const SKR_NOEXCEPT { return has_tight_width() && has_tight_height(); }

    // compare
    inline constexpr bool operator==(const BoxConstraints& rhs) const SKR_NOEXCEPT
    {
        return min_width == rhs.min_width && max_width == rhs.max_width && min_height == rhs.min_height && max_height == rhs.max_height;
    }
    inline constexpr bool operator!=(const BoxConstraints& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // ops
    inline constexpr float constrain_width(float width = std::numeric_limits<float>::infinity()) const SKR_NOEXCEPT
    {
        return std::clamp(width, min_width, max_width);
    }
    inline constexpr float constrain_height(float height = std::numeric_limits<float>::infinity()) const SKR_NOEXCEPT
    {
        return std::clamp(height, min_height, max_height);
    }
    inline constexpr Sizef constrain(Sizef size) const SKR_NOEXCEPT
    {
        size.width  = constrain_width(size.width);
        size.height = constrain_height(size.height);
        return size;
    }
    inline constexpr BoxConstraints loosen() const SKR_NOEXCEPT
    {
        return {
            0,
            max_width,
            0,
            max_height,
        };
    }
    inline BoxConstraints enforce(BoxConstraints constraints) const
    {
        return {
            std::clamp(min_width, constraints.min_width, constraints.max_width),
            std::clamp(max_width, constraints.min_width, constraints.max_width),
            std::clamp(min_height, constraints.min_height, constraints.max_height),
            std::clamp(max_height, constraints.min_height, constraints.max_height),
        };
    }
};
} // namespace skr::gui