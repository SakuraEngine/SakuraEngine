#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
#if (__cplusplus >= 202002L)

using std::lerp;

#else

constexpr float lerp(float a, float b, float t) SKR_NOEXCEPT
{
    return a + (b - a) * t;
}

#endif

struct Offset {
    float x = 0;
    float y = 0;

    // constant
    inline static constexpr Offset zero() SKR_NOEXCEPT { return { 0, 0 }; }
    inline static constexpr Offset infinite() SKR_NOEXCEPT { return { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() }; }

    // infinite
    inline constexpr bool is_infinite() const SKR_NOEXCEPT
    {
        return x >= std::numeric_limits<float>::infinity() || y >= std::numeric_limits<float>::infinity();
    }
    inline constexpr bool is_finite() const SKR_NOEXCEPT
    {
        return x != std::numeric_limits<float>::infinity() && x != std::numeric_limits<float>::quiet_NaN() && x != std::numeric_limits<float>::signaling_NaN() &&
               y != std::numeric_limits<float>::infinity() && y != std::numeric_limits<float>::quiet_NaN() && y != std::numeric_limits<float>::signaling_NaN();
    }

    // info
    inline float length() const SKR_NOEXCEPT { return std::sqrt(x * x + y * y); }
    inline float length_squared() const SKR_NOEXCEPT { return x * x + y * y; }
    inline float radians() const SKR_NOEXCEPT { return std::atan2(y, x); }

    // compare
    inline constexpr bool operator==(const Offset& rhs) const SKR_NOEXCEPT { return x == rhs.x && y == rhs.y; }
    inline constexpr bool operator!=(const Offset& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // arithmetic
    inline constexpr Offset operator-() const SKR_NOEXCEPT { return { -x, -y }; }

    inline constexpr Offset operator+(float rhs) const SKR_NOEXCEPT { return { x + rhs, y + rhs }; }
    inline constexpr Offset operator-(float rhs) const SKR_NOEXCEPT { return { x - rhs, y - rhs }; }
    inline constexpr Offset operator*(float rhs) const SKR_NOEXCEPT { return { x * rhs, y * rhs }; }
    inline constexpr Offset operator/(float rhs) const SKR_NOEXCEPT { return { x / rhs, y / rhs }; }
    inline Offset           operator%(float rhs) const SKR_NOEXCEPT { return { std::fmod(x, rhs), std::fmod(y, rhs) }; }

    inline constexpr Offset operator+(const Offset& rhs) const SKR_NOEXCEPT { return { x + rhs.x, y + rhs.y }; }
    inline constexpr Offset operator-(const Offset& rhs) const SKR_NOEXCEPT { return { x - rhs.x, y - rhs.y }; }
    inline constexpr Offset operator*(const Offset& rhs) const SKR_NOEXCEPT { return { x * rhs.x, y * rhs.y }; }
    inline constexpr Offset operator/(const Offset& rhs) const SKR_NOEXCEPT { return { x / rhs.x, y / rhs.y }; }
    inline Offset           operator%(const Offset& rhs) const SKR_NOEXCEPT { return { std::fmod(x, rhs.x), std::fmod(y, rhs.y) }; }

    inline constexpr Offset& operator+=(float rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline constexpr Offset& operator-=(float rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline constexpr Offset& operator*=(float rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline constexpr Offset& operator/=(float rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Offset&           operator%=(float rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline constexpr Offset& operator+=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline constexpr Offset& operator-=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline constexpr Offset& operator*=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline constexpr Offset& operator/=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Offset&           operator%=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline static Offset lerp(const Offset& a, const Offset& b, float t) SKR_NOEXCEPT
    {
        return { ::skr::gui::lerp(a.x, b.x, t), ::skr::gui::lerp(a.y, b.y, t) };
    }
};

struct Size {
    float width = 0;
    float height = 0;

private:
    struct __InfinityParams {
        float width = std::numeric_limits<float>::infinity();
        float height = std::numeric_limits<float>::infinity();
    };

public:
    // factory
    inline static constexpr Size Inf(__InfinityParams params) SKR_NOEXCEPT { return { params.width, params.height }; }
    inline static constexpr Size Square(float size) SKR_NOEXCEPT { return { size, size }; }
    inline static constexpr Size Radius(float radius) SKR_NOEXCEPT { return { radius * 2, radius * 2 }; }

public:
    // constant
    inline static constexpr Size zero() SKR_NOEXCEPT
    {
        return { 0, 0 };
    }
    inline static constexpr Size infinite() SKR_NOEXCEPT { return { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() }; }

    // infinite
    inline constexpr bool is_infinite() const SKR_NOEXCEPT
    {
        return width >= std::numeric_limits<float>::infinity() || height >= std::numeric_limits<float>::infinity();
    }
    inline constexpr bool is_finite() const SKR_NOEXCEPT
    {
        return width != std::numeric_limits<float>::infinity() && width != std::numeric_limits<float>::quiet_NaN() && width != std::numeric_limits<float>::signaling_NaN() &&
               height != std::numeric_limits<float>::infinity() && height != std::numeric_limits<float>::quiet_NaN() && height != std::numeric_limits<float>::signaling_NaN();
    }

    // info
    inline constexpr float aspect_ratio() const SKR_NOEXCEPT
    {
        if (height != 0.0)
        {
            return width / height;
        }
        if (width > 0.0)
        {
            return std::numeric_limits<float>::infinity();
        }
        if (width < 0.0)
        {
            return -std::numeric_limits<float>::infinity();
        }
        return 0.0;
    }
    inline constexpr bool  is_empty() const SKR_NOEXCEPT { return width <= 0.0 || height <= 0.0; }
    inline constexpr float area() const SKR_NOEXCEPT { return is_finite() ? std::abs(width * height) : std::numeric_limits<float>::infinity(); }
    inline constexpr float shortest_side() const SKR_NOEXCEPT { return std::min(std::abs(width), std::abs(height)); }
    inline constexpr float longest_side() const SKR_NOEXCEPT { return std::max(std::abs(width), std::abs(height)); }
    inline constexpr Size  flipped() const SKR_NOEXCEPT { return { height, width }; }

    // offset ops
    inline constexpr bool   contains(const Offset& offset) const SKR_NOEXCEPT { return offset.x >= 0.0 && offset.x <= width && offset.y >= 0.0 && offset.y <= height; }
    inline constexpr Offset top_left(const Offset& offset) const SKR_NOEXCEPT { return { offset.x, offset.y }; }
    inline constexpr Offset top_center(const Offset& offset) const SKR_NOEXCEPT { return { offset.x + width / 2.0f, offset.y }; }
    inline constexpr Offset top_right(const Offset& offset) const SKR_NOEXCEPT { return { offset.x + width, offset.y }; }
    inline constexpr Offset center_left(const Offset& offset) const SKR_NOEXCEPT { return { offset.x, offset.y + height / 2.0f }; }
    inline constexpr Offset center(const Offset& offset) const SKR_NOEXCEPT { return { offset.x + width / 2.0f, offset.y + height / 2.0f }; }
    inline constexpr Offset center_right(const Offset& offset) const SKR_NOEXCEPT { return { offset.x + width, offset.y + height / 2.0f }; }
    inline constexpr Offset bottom_left(const Offset& offset) const SKR_NOEXCEPT { return { offset.x, offset.y + height }; }
    inline constexpr Offset bottom_center(const Offset& offset) const SKR_NOEXCEPT { return { offset.x + width / 2.0f, offset.y + height }; }
    inline constexpr Offset bottom_right(const Offset& offset) const SKR_NOEXCEPT { return { offset.x + width, offset.y + height }; }

    // compare
    inline constexpr bool operator==(const Size& rhs) const SKR_NOEXCEPT { return width == rhs.width && height == rhs.height; }
    inline constexpr bool operator!=(const Size& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // arithmetic
    inline constexpr Size operator+(float rhs) const SKR_NOEXCEPT { return { width + rhs, height + rhs }; }
    inline constexpr Size operator-(float rhs) const SKR_NOEXCEPT { return { width - rhs, height - rhs }; }
    inline constexpr Size operator*(float rhs) const SKR_NOEXCEPT { return { width * rhs, height * rhs }; }
    inline constexpr Size operator/(float rhs) const SKR_NOEXCEPT { return { width / rhs, height / rhs }; }
    inline Size           operator%(float rhs) const SKR_NOEXCEPT { return { std::fmod(width, rhs), std::fmod(height, rhs) }; }

    inline constexpr Size operator+(const Size& rhs) const SKR_NOEXCEPT { return { width + rhs.width, height + rhs.height }; }
    inline constexpr Size operator-(const Size& rhs) const SKR_NOEXCEPT { return { width - rhs.width, height - rhs.height }; }
    inline constexpr Size operator*(const Size& rhs) const SKR_NOEXCEPT { return { width * rhs.width, height * rhs.height }; }
    inline constexpr Size operator/(const Size& rhs) const SKR_NOEXCEPT { return { width / rhs.width, height / rhs.height }; }
    inline Size           operator%(const Size& rhs) const SKR_NOEXCEPT { return { std::fmod(width, rhs.width), std::fmod(height, rhs.height) }; }

    inline constexpr Size& operator+=(float rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline constexpr Size& operator-=(float rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline constexpr Size& operator*=(float rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline constexpr Size& operator/=(float rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Size&           operator%=(float rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline constexpr Size& operator+=(const Size& rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline constexpr Size& operator-=(const Size& rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline constexpr Size& operator*=(const Size& rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline constexpr Size& operator/=(const Size& rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Size&           operator%=(const Size& rhs) SKR_NOEXCEPT { return *this = *this % rhs; }
};
struct Rect {
    float left = 0;
    float top = 0;
    float right = 0;
    float bottom = 0;

private:
    struct __LTWHParams {
        float left = 0;
        float top = 0;
        float width = 0;
        float height = 0;
    };
    struct __CircleParams {
        Offset center = { 0, 0 };
        float  radius = 0;
    };
    struct __CenterParams {
        Offset center = { 0, 0 };
        Size   size = { 0, 0 };
    };

public:
    // factory
    inline static constexpr Rect LTWH(__LTWHParams params) SKR_NOEXCEPT
    {
        return {
            params.left,
            params.top,
            params.left + params.width,
            params.top + params.height,
        };
    }
    inline static constexpr Rect Circle(__CircleParams params) SKR_NOEXCEPT
    {
        return {
            params.center.x - params.radius,
            params.center.y - params.radius,
            params.center.x + params.radius,
            params.center.y + params.radius,
        };
    }
    inline static constexpr Rect Center(__CenterParams params) SKR_NOEXCEPT
    {
        return {
            params.center.x - params.size.width / 2.0f,
            params.center.y - params.size.height / 2.0f,
            params.center.x + params.size.width / 2.0f,
            params.center.y + params.size.height / 2.0f,
        };
    }
    inline static constexpr Rect Points(const Offset& a, const Offset& b) SKR_NOEXCEPT
    {
        return {
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::max(a.x, b.x),
            std::max(a.y, b.y),
        };
    }

    // constant
    inline static constexpr Rect zero() SKR_NOEXCEPT { return { 0, 0, 0, 0 }; }
    inline static constexpr Rect largest() SKR_NOEXCEPT
    {
        return {
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
        };
    }

    // infinite & nan
    inline constexpr bool is_infinite() const SKR_NOEXCEPT
    {
        return left >= -std::numeric_limits<float>::infinity() ||
               top >= -std::numeric_limits<float>::infinity() ||
               right >= std::numeric_limits<float>::infinity() ||
               bottom >= std::numeric_limits<float>::infinity();
    }
    inline constexpr bool is_finite() const SKR_NOEXCEPT
    {
        return left != std::numeric_limits<float>::infinity() && left != std::numeric_limits<float>::quiet_NaN() && left != std::numeric_limits<float>::signaling_NaN() &&
               top != std::numeric_limits<float>::infinity() && top != std::numeric_limits<float>::quiet_NaN() && top != std::numeric_limits<float>::signaling_NaN() &&
               right != std::numeric_limits<float>::infinity() && right != std::numeric_limits<float>::quiet_NaN() && right != std::numeric_limits<float>::signaling_NaN() &&
               bottom != std::numeric_limits<float>::infinity() && bottom != std::numeric_limits<float>::quiet_NaN() && bottom != std::numeric_limits<float>::signaling_NaN();
    }
    inline constexpr bool has_nan() const SKR_NOEXCEPT
    {
        return left == std::numeric_limits<float>::quiet_NaN() || left == std::numeric_limits<float>::signaling_NaN() ||
               top == std::numeric_limits<float>::quiet_NaN() || top == std::numeric_limits<float>::signaling_NaN() ||
               right == std::numeric_limits<float>::quiet_NaN() || right == std::numeric_limits<float>::signaling_NaN() ||
               bottom == std::numeric_limits<float>::quiet_NaN() || bottom == std::numeric_limits<float>::signaling_NaN();
    }

    // info
    inline constexpr bool   is_empty() const SKR_NOEXCEPT { return left >= right || top >= bottom; }
    inline constexpr bool   is_point() const SKR_NOEXCEPT { return left == right && top == bottom; }
    inline constexpr float  width() const SKR_NOEXCEPT { return right - left; }
    inline constexpr float  height() const SKR_NOEXCEPT { return bottom - top; }
    inline constexpr Size   size() const SKR_NOEXCEPT { return { width(), height() }; }
    inline constexpr Offset top_left() const SKR_NOEXCEPT { return { left, top }; }
    inline constexpr Offset top_center() const SKR_NOEXCEPT { return { (left + right) / 2.0f, top }; }
    inline constexpr Offset top_right() const SKR_NOEXCEPT { return { right, top }; }
    inline constexpr Offset center_left() const SKR_NOEXCEPT { return { left, (top + bottom) / 2.0f }; }
    inline constexpr Offset center() const SKR_NOEXCEPT { return { (left + right) / 2.0f, (top + bottom) / 2.0f }; }
    inline constexpr Offset center_right() const SKR_NOEXCEPT { return { right, (top + bottom) / 2.0f }; }
    inline constexpr Offset bottom_left() const SKR_NOEXCEPT { return { left, bottom }; }
    inline constexpr Offset bottom_center() const SKR_NOEXCEPT { return { (left + right) / 2.0f, bottom }; }
    inline constexpr Offset bottom_right() const SKR_NOEXCEPT { return { right, bottom }; }

    // offset & size & rect ops
    inline constexpr Rect
    shift(const Offset& offset) SKR_NOEXCEPT
    {
        return {
            left + offset.x,
            top + offset.y,
            right + offset.x,
            bottom + offset.y,
        };
    }
    inline constexpr Rect hold(const Offset& point) SKR_NOEXCEPT
    {
        return {
            std::min(left, point.x),
            std::min(top, point.y),
            std::max(right, point.x),
            std::max(bottom, point.y),
        };
    }
    inline constexpr Rect inflate(float delta) SKR_NOEXCEPT
    {
        return {
            left - delta,
            top - delta,
            right + delta,
            bottom + delta,
        };
    }
    inline constexpr Rect deflate(float delta) SKR_NOEXCEPT { return inflate(-delta); }
    inline constexpr Rect intersect(const Rect& rhs) SKR_NOEXCEPT
    {
        return {
            std::max(left, rhs.left),
            std::max(top, rhs.top),
            std::min(right, rhs.right),
            std::min(bottom, rhs.bottom),
        };
    }
    inline constexpr Rect unite(const Rect& rhs) SKR_NOEXCEPT
    {
        return {
            std::min(left, rhs.left),
            std::min(top, rhs.top),
            std::max(right, rhs.right),
            std::max(bottom, rhs.bottom),
        };
    }
    inline constexpr bool overlaps(const Rect& rhs) SKR_NOEXCEPT
    {
        if (right <= rhs.left || rhs.right <= left) return false;
        if (bottom <= rhs.top || rhs.bottom <= top) return false;
        return true;
    }
    inline constexpr bool contains(const Offset& offset) SKR_NOEXCEPT
    {
        return offset.x >= left && offset.x < right && offset.y >= top && offset.y < bottom;
    }

    // compare
    inline constexpr bool operator==(const Rect& rhs) const SKR_NOEXCEPT
    {
        return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom;
    }
    inline constexpr bool operator!=(const Rect& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // arithmetic
    inline static Optional<Rect> lerp(Optional<Rect> a, Optional<Rect> b, float t)
    {
        if (!b)
        {
            if (!a)
            {
                return {};
            }
            else
            {
                const float k = 1.0f - t;
                return Rect{ (*a).left * k, (*a).top * k, (*a).right * k, (*a).bottom * k };
            }
        }
        else
        {
            if (!a)
            {
                return Rect{ (*b).left * t, (*b).top * t, (*b).right * t, (*b).bottom * t };
            }
            else
            {
                return Rect{
                    ::skr::gui::lerp((*a).left, (*b).left, t),
                    ::skr::gui::lerp((*a).top, (*b).top, t),
                    ::skr::gui::lerp((*a).right, (*b).right, t),
                    ::skr::gui::lerp((*a).bottom, (*b).bottom, t),
                };
            }
        }
    }
};

// TODO. Ray Hit Test
struct Ray {
    skr_float3_t origin;
    skr_float3_t direction;
};
} // namespace skr::gui