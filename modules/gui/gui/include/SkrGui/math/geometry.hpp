#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
#define SKR_GUI_MATH_ENABLE_IF_FLOAT(__TYPE) template <typename TT = __TYPE, std::enable_if_t<std::is_floating_point_v<TT>, int> = 0>
#define SKR_GUI_MATH_UTILITY_MIXIN(__TYPE)                                                                      \
private:                                                                                                        \
    inline static constexpr bool _is_floating = std::is_floating_point_v<__TYPE>;                               \
    SKR_GUI_MATH_ENABLE_IF_FLOAT(__TYPE)                                                                        \
    inline static constexpr __TYPE _infinity() SKR_NOEXCEPT { return std::numeric_limits<__TYPE>::infinity(); } \
    inline static constexpr __TYPE _min() { return std::numeric_limits<__TYPE>::min(); }                        \
    inline static constexpr __TYPE _max() { return std::numeric_limits<__TYPE>::max(); }                        \
    SKR_GUI_MATH_ENABLE_IF_FLOAT(__TYPE)                                                                        \
    inline static bool _is_nan(__TYPE v) SKR_NOEXCEPT { return std::isnan(v); }                                 \
    SKR_GUI_MATH_ENABLE_IF_FLOAT(__TYPE)                                                                        \
    inline static __TYPE _lerp(__TYPE a, __TYPE b, __TYPE t) { return a + (b - a) * t; }                        \
    template <typename TShit>                                                                                   \
    inline static TShit  _shit_mod(TShit v, TShit mod) { return v % mod; }                                      \
    inline static __TYPE _mod(__TYPE v, __TYPE mod)                                                             \
    {                                                                                                           \
        if constexpr (_is_floating)                                                                             \
        {                                                                                                       \
            return std::fmod(v, mod);                                                                           \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            return _shit_mod(v, mod);                                                                           \
        }                                                                                                       \
    }                                                                                                           \
                                                                                                                \
public:

template <typename T>
struct Offset {
    T x = { 0 };
    T y = { 0 };

    SKR_GUI_MATH_UTILITY_MIXIN(T)

    // factory
    inline static Offset Zero() SKR_NOEXCEPT
    {
        return { 0, 0 };
    }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Offset Infinite() SKR_NOEXCEPT { return { _infinity(), _infinity() }; }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Offset Radians(T radians, T radius = 1) SKR_NOEXCEPT { return { radius * std::cos(radians), radius * std::sin(radians) }; }

    // infinite
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool is_infinite() const SKR_NOEXCEPT
    {
        return x >= _infinity() || y >= _infinity();
    }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool is_finite() const SKR_NOEXCEPT
    {
        return x != _infinity() && !_is_nan(x) &&
               y != _infinity() && !_is_nan(x);
    }

    // info
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline T length() const SKR_NOEXCEPT { return std::sqrt(x * x + y * y); }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline T length_squared() const SKR_NOEXCEPT { return x * x + y * y; }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline T radians() const SKR_NOEXCEPT { return std::atan2(y, x); }

    // compare
    inline bool operator==(const Offset& rhs) const SKR_NOEXCEPT { return x == rhs.x && y == rhs.y; }
    inline bool operator!=(const Offset& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // cast
    template <typename U>
    inline operator Offset<U>() const SKR_NOEXCEPT { return { static_cast<U>(x), static_cast<U>(y) }; }

    // arithmetic ops
    inline Offset operator-() const SKR_NOEXCEPT { return { -x, -y }; }

    friend inline Offset operator+(const Offset& lhs, T rhs) SKR_NOEXCEPT { return { lhs.x + rhs, lhs.y + rhs }; }
    friend inline Offset operator-(const Offset& lhs, T rhs) SKR_NOEXCEPT { return { lhs.x - rhs, lhs.y - rhs }; }
    friend inline Offset operator*(const Offset& lhs, T rhs) SKR_NOEXCEPT { return { lhs.x * rhs, lhs.y * rhs }; }
    friend inline Offset operator/(const Offset& lhs, T rhs) SKR_NOEXCEPT { return { lhs.x / rhs, lhs.y / rhs }; }
    friend inline Offset operator%(const Offset& lhs, T rhs) SKR_NOEXCEPT { return { _mod(lhs.x, rhs), _mod(lhs.y, rhs) }; }

    friend inline Offset operator+(T lhs, const Offset& rhs) SKR_NOEXCEPT { return { lhs + rhs.x, lhs + rhs.y }; }
    friend inline Offset operator-(T lhs, const Offset& rhs) SKR_NOEXCEPT { return { lhs - rhs.x, lhs - rhs.y }; }
    friend inline Offset operator*(T lhs, const Offset& rhs) SKR_NOEXCEPT { return { lhs * rhs.x, lhs * rhs.y }; }
    friend inline Offset operator/(T lhs, const Offset& rhs) SKR_NOEXCEPT { return { lhs / rhs.x, lhs / rhs.y }; }
    friend inline Offset operator%(T lhs, const Offset& rhs) SKR_NOEXCEPT { return { _mod(lhs, rhs.x), _mod(lhs, rhs.y) }; }

    inline Offset operator+(const Offset& rhs) const SKR_NOEXCEPT { return { x + rhs.x, y + rhs.y }; }
    inline Offset operator-(const Offset& rhs) const SKR_NOEXCEPT { return { x - rhs.x, y - rhs.y }; }
    inline Offset operator*(const Offset& rhs) const SKR_NOEXCEPT { return { x * rhs.x, y * rhs.y }; }
    inline Offset operator/(const Offset& rhs) const SKR_NOEXCEPT { return { x / rhs.x, y / rhs.y }; }
    inline Offset operator%(const Offset& rhs) const SKR_NOEXCEPT { return { _mod(x, rhs.x), _mod(y, rhs.y) }; }

    inline Offset& operator+=(T rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Offset& operator-=(T rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Offset& operator*=(T rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Offset& operator/=(T rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Offset& operator%=(T rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline Offset& operator+=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Offset& operator-=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Offset& operator*=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Offset& operator/=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Offset& operator%=(const Offset& rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    // calculations
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Offset lerp(const Offset& a, const Offset& b, T t) SKR_NOEXCEPT
    {
        return { _lerp(a.x, b.x, t), _lerp(a.y, b.y, t) };
    }
};

template <typename T>
struct Size {
    T width  = { 0 };
    T height = { 0 };

    SKR_GUI_MATH_UTILITY_MIXIN(T)

    // factory
    inline static Size Zero() SKR_NOEXCEPT { return { 0, 0 }; }
    inline static Size Square(T size) SKR_NOEXCEPT { return { size, size }; }
    inline static Size Radius(T radius) SKR_NOEXCEPT { return { radius * 2, radius * 2 }; }

    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Size Infinite() SKR_NOEXCEPT { return { _infinity(), _infinity() }; }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Size InfiniteWidth(T height) SKR_NOEXCEPT { return { _infinity(), height }; }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Size InfiniteHeight(T width) SKR_NOEXCEPT { return { width, _infinity() }; }

    // infinite
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool is_infinite() const SKR_NOEXCEPT
    {
        return width >= _infinity() || height >= _infinity();
    }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool is_finite() const SKR_NOEXCEPT
    {
        return width != _infinity() && !_is_nan(width) &&
               height != _infinity() && !_is_nan(height);
    }

    // info
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline T aspect_ratio() const SKR_NOEXCEPT
    {
        if (height != 0.0)
        {
            return width / height;
        }
        if (width > 0.0)
        {
            return _infinity();
        }
        if (width < 0.0)
        {
            return -_infinity();
        }
        return 0.0;
    }
    inline T area() const SKR_NOEXCEPT
    {
        if constexpr (_is_floating)
        {
            return is_finite() ? std::abs(width * height) : _infinity();
        }
        else
        {
            return std::abs(width * height);
        }
    }
    inline bool is_empty() const SKR_NOEXCEPT { return width <= 0.0 || height <= 0.0; }
    inline T    shortest_side() const SKR_NOEXCEPT { return std::min(std::abs(width), std::abs(height)); }
    inline T    longest_side() const SKR_NOEXCEPT { return std::max(std::abs(width), std::abs(height)); }
    inline Size flipped() const SKR_NOEXCEPT { return { height, width }; }

    // offset ops
    inline bool      contains(const Offset<T>& offset) const SKR_NOEXCEPT { return offset.x >= 0.0 && offset.x <= width && offset.y >= 0.0 && offset.y <= height; }
    inline Offset<T> top_left(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x, offset.y }; }
    inline Offset<T> top_center(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x + width / T(2), offset.y }; }
    inline Offset<T> top_right(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x + width, offset.y }; }
    inline Offset<T> center_left(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x, offset.y + height / T(2) }; }
    inline Offset<T> center(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x + width / T(2), offset.y + height / T(2) }; }
    inline Offset<T> center_right(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x + width, offset.y + height / T(2) }; }
    inline Offset<T> bottom_left(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x, offset.y + height }; }
    inline Offset<T> bottom_center(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x + width / T(2), offset.y + height }; }
    inline Offset<T> bottom_right(const Offset<T>& offset) const SKR_NOEXCEPT { return { offset.x + width, offset.y + height }; }

    // compare
    inline bool operator==(const Size<T>& rhs) const SKR_NOEXCEPT { return width == rhs.width && height == rhs.height; }
    inline bool operator!=(const Size<T>& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // cast
    template <typename U>
    inline operator Size<U>() const SKR_NOEXCEPT { return { static_cast<U>(width), static_cast<U>(height) }; }

    // arithmetic ops
    friend inline Size operator+(const Size& lhs, float rhs) SKR_NOEXCEPT { return { lhs.width + rhs, lhs.height + rhs }; }
    friend inline Size operator-(const Size& lhs, float rhs) SKR_NOEXCEPT { return { lhs.width - rhs, lhs.height - rhs }; }
    friend inline Size operator*(const Size& lhs, float rhs) SKR_NOEXCEPT { return { lhs.width * rhs, lhs.height * rhs }; }
    friend inline Size operator/(const Size& lhs, float rhs) SKR_NOEXCEPT { return { lhs.width / rhs, lhs.height / rhs }; }
    friend inline Size operator%(const Size& lhs, float rhs) SKR_NOEXCEPT { return { _mod(lhs.width, rhs), _mod(lhs.height, rhs) }; }

    friend inline Size operator+(float lhs, const Size& rhs) SKR_NOEXCEPT { return { lhs + rhs.width, lhs + rhs.height }; }
    friend inline Size operator-(float lhs, const Size& rhs) SKR_NOEXCEPT { return { lhs - rhs.width, lhs - rhs.height }; }
    friend inline Size operator*(float lhs, const Size& rhs) SKR_NOEXCEPT { return { lhs * rhs.width, lhs * rhs.height }; }
    friend inline Size operator/(float lhs, const Size& rhs) SKR_NOEXCEPT { return { lhs / rhs.width, lhs / rhs.height }; }
    friend inline Size operator%(float lhs, const Size& rhs) SKR_NOEXCEPT { return { _mod(lhs, rhs.width), _mod(lhs, rhs.height) }; }

    inline Size operator+(const Size& rhs) const SKR_NOEXCEPT { return { width + rhs.width, height + rhs.height }; }
    inline Size operator-(const Size& rhs) const SKR_NOEXCEPT { return { width - rhs.width, height - rhs.height }; }
    inline Size operator*(const Size& rhs) const SKR_NOEXCEPT { return { width * rhs.width, height * rhs.height }; }
    inline Size operator/(const Size& rhs) const SKR_NOEXCEPT { return { width / rhs.width, height / rhs.height }; }
    inline Size operator%(const Size& rhs) const SKR_NOEXCEPT { return { _mod(width, rhs.width), _mod(height, rhs.height) }; }

    inline Size& operator+=(float rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Size& operator-=(float rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Size& operator*=(float rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Size& operator/=(float rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Size& operator%=(float rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline Size& operator+=(const Size& rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Size& operator-=(const Size& rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Size& operator*=(const Size& rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Size& operator/=(const Size& rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Size& operator%=(const Size& rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Size lerp(const Size& a, const Size& b, T t) SKR_NOEXCEPT
    {
        return {
            _lerp(a.width, b.width, t),
            _lerp(a.height, b.height, t),
        };
    }
    inline static Size min(const Size& a, const Size& b) SKR_NOEXCEPT
    {
        return {
            std::min(a.width, b.width),
            std::min(a.height, b.height),
        };
    }
    inline static Size max(const Size& a, const Size& b) SKR_NOEXCEPT
    {
        return {
            std::max(a.width, b.width),
            std::max(a.height, b.height),
        };
    }
};

template <typename T>
struct Rect {
    T left   = { 0 };
    T top    = { 0 };
    T right  = { 0 };
    T bottom = { 0 };

    SKR_GUI_MATH_UTILITY_MIXIN(T)

    // factory
    inline static Rect Zero() SKR_NOEXCEPT { return { 0, 0, 0, 0 }; }
    inline static Rect Largest() SKR_NOEXCEPT { return { _min(), _min(), _max(), _max() }; }
    inline static Rect LTWH(T left, T top, T width, T height) SKR_NOEXCEPT { return { left, top, left + width, top + height }; }
    inline static Rect Circle(Offset<T> center, T radius) SKR_NOEXCEPT { return { center.x - radius, center.y - radius, center.x + radius, center.y + radius }; }
    inline static Rect Center(Offset<T> center, Size<T> size) SKR_NOEXCEPT { return { center.x - size.width / T(2), center.y - size.height / T(2), center.x + size.width / T(2), center.y + size.height / T(2) }; }
    inline static Rect OffsetSize(const Offset<T>& offset, const Size<T>& size) SKR_NOEXCEPT { return { offset.x, offset.y, offset.x + size.width, offset.y + size.height }; }
    inline static Rect Points(const Offset<T>& a, const Offset<T>& b) SKR_NOEXCEPT { return { std::min(a.x, b.x), std::min(a.y, b.y), std::max(a.x, b.x), std::max(a.y, b.y) }; }

    // infinite & nan
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool is_infinite() const SKR_NOEXCEPT
    {
        return left <= -_infinity() || top <= -_infinity() ||
               right >= _infinity() || bottom >= _infinity();
    }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool is_finite() const SKR_NOEXCEPT
    {
        return left != _infinity() && !_is_nan(left) &&
               top != _infinity() && !_is_nan(top) &&
               right != _infinity() && !_is_nan(right) &&
               bottom != _infinity() && !_is_nan(bottom);
    }
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline bool has_nan() const SKR_NOEXCEPT
    {
        return _is_nan(left) ||
               _is_nan(top) ||
               _is_nan(right) ||
               _is_nan(bottom);
    }

    // info
    inline bool      is_empty() const SKR_NOEXCEPT { return left >= right || top >= bottom; }
    inline bool      is_point() const SKR_NOEXCEPT { return left == right && top == bottom; }
    inline T         width() const SKR_NOEXCEPT { return right - left; }
    inline T         height() const SKR_NOEXCEPT { return bottom - top; }
    inline Size<T>   size() const SKR_NOEXCEPT { return { width(), height() }; }
    inline Offset<T> top_left() const SKR_NOEXCEPT { return { left, top }; }
    inline Offset<T> top_center() const SKR_NOEXCEPT { return { (left + right) / T(2), top }; }
    inline Offset<T> top_right() const SKR_NOEXCEPT { return { right, top }; }
    inline Offset<T> center_left() const SKR_NOEXCEPT { return { left, (top + bottom) / T(2) }; }
    inline Offset<T> center() const SKR_NOEXCEPT { return { (left + right) / T(2), (top + bottom) / T(2) }; }
    inline Offset<T> center_right() const SKR_NOEXCEPT { return { right, (top + bottom) / T(2) }; }
    inline Offset<T> bottom_left() const SKR_NOEXCEPT { return { left, bottom }; }
    inline Offset<T> bottom_center() const SKR_NOEXCEPT { return { (left + right) / T(2), bottom }; }
    inline Offset<T> bottom_right() const SKR_NOEXCEPT { return { right, bottom }; }

    // offset & size & rect ops
    inline Rect shift(const Offset<T>& offset) SKR_NOEXCEPT
    {
        return {
            left + offset.x,
            top + offset.y,
            right + offset.x,
            bottom + offset.y,
        };
    }
    inline Rect hold(const Offset<T>& point) SKR_NOEXCEPT
    {
        return {
            std::min(left, point.x),
            std::min(top, point.y),
            std::max(right, point.x),
            std::max(bottom, point.y),
        };
    }
    inline Rect inflate(T delta) SKR_NOEXCEPT
    {
        return {
            left - delta,
            top - delta,
            right + delta,
            bottom + delta,
        };
    }
    inline Rect deflate(T delta) SKR_NOEXCEPT { return inflate(-delta); }
    inline Rect intersect(const Rect<T>& rhs) SKR_NOEXCEPT
    {
        return {
            std::max(left, rhs.left),
            std::max(top, rhs.top),
            std::min(right, rhs.right),
            std::min(bottom, rhs.bottom),
        };
    }
    inline Rect unite(const Rect& rhs) SKR_NOEXCEPT
    {
        return {
            std::min(left, rhs.left),
            std::min(top, rhs.top),
            std::max(right, rhs.right),
            std::max(bottom, rhs.bottom),
        };
    }
    inline bool overlaps(const Rect& rhs) SKR_NOEXCEPT
    {
        if (right <= rhs.left || rhs.right <= left) return false;
        if (bottom <= rhs.top || rhs.bottom <= top) return false;
        return true;
    }
    inline bool contains(const Offset<T>& offset) SKR_NOEXCEPT
    {
        return offset.x >= left && offset.x < right && offset.y >= top && offset.y < bottom;
    }

    // compare
    inline bool operator==(const Rect& rhs) const SKR_NOEXCEPT
    {
        return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom;
    }
    inline bool operator!=(const Rect& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }

    // cast
    template <typename U>
    inline operator Rect<U>() const SKR_NOEXCEPT { return { static_cast<U>(left), static_cast<U>(top), static_cast<U>(right), static_cast<U>(bottom) }; }

    // arithmetic
    SKR_GUI_MATH_ENABLE_IF_FLOAT(T)
    inline static Optional<Rect> lerp(const Optional<Rect>& a, const Optional<Rect>& b, float t)
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
                    _lerp((*a).left, (*b).left, t),
                    _lerp((*a).top, (*b).top, t),
                    _lerp((*a).right, (*b).right, t),
                    _lerp((*a).bottom, (*b).bottom, t),
                };
            }
        }
    }
};

struct Alignment {
    float x = 0;
    float y = 0;

    SKR_GUI_MATH_UTILITY_MIXIN(float)

    // factory
    inline static constexpr Alignment TopLeft() { return { 0, 0 }; }
    inline static constexpr Alignment TopCenter() { return { 0.5f, 0 }; }
    inline static constexpr Alignment TopRight() { return { 1, 0 }; }
    inline static constexpr Alignment CenterLeft() { return { 0, 0.5f }; }
    inline static constexpr Alignment Center() { return { 0.5f, 0.5f }; }
    inline static constexpr Alignment CenterRight() { return { 1, 0.5f }; }
    inline static constexpr Alignment BottomLeft() { return { 0, 1 }; }
    inline static constexpr Alignment BottomCenter() { return { 0.5f, 1 }; }
    inline static constexpr Alignment BottomRight() { return { 1, 1 }; }

    // arithmetic
    inline Alignment operator-() const SKR_NOEXCEPT { return { -x, -y }; }

    friend inline Alignment operator+(const Alignment& lhs, float rhs) SKR_NOEXCEPT { return { lhs.x + rhs, lhs.y + rhs }; }
    friend inline Alignment operator-(const Alignment& lhs, float rhs) SKR_NOEXCEPT { return { lhs.x - rhs, lhs.y - rhs }; }
    friend inline Alignment operator*(const Alignment& lhs, float rhs) SKR_NOEXCEPT { return { lhs.x * rhs, lhs.y * rhs }; }
    friend inline Alignment operator/(const Alignment& lhs, float rhs) SKR_NOEXCEPT { return { lhs.x / rhs, lhs.y / rhs }; }
    friend inline Alignment operator%(const Alignment& lhs, float rhs) SKR_NOEXCEPT { return { std::fmod(lhs.x, rhs), std::fmod(lhs.y, rhs) }; }

    friend inline Alignment operator+(float lhs, const Alignment& rhs) SKR_NOEXCEPT { return { lhs + rhs.x, lhs + rhs.y }; }
    friend inline Alignment operator-(float lhs, const Alignment& rhs) SKR_NOEXCEPT { return { lhs - rhs.x, lhs - rhs.y }; }
    friend inline Alignment operator*(float lhs, const Alignment& rhs) SKR_NOEXCEPT { return { lhs * rhs.x, lhs * rhs.y }; }
    friend inline Alignment operator/(float lhs, const Alignment& rhs) SKR_NOEXCEPT { return { lhs / rhs.x, lhs / rhs.y }; }
    friend inline Alignment operator%(float lhs, const Alignment& rhs) SKR_NOEXCEPT { return { std::fmod(lhs, rhs.x), std::fmod(lhs, rhs.y) }; }

    inline Alignment operator+(const Alignment& rhs) const SKR_NOEXCEPT { return { x + rhs.x, y + rhs.y }; }
    inline Alignment operator-(const Alignment& rhs) const SKR_NOEXCEPT { return { x - rhs.x, y - rhs.y }; }
    inline Alignment operator*(const Alignment& rhs) const SKR_NOEXCEPT { return { x * rhs.x, y * rhs.y }; }
    inline Alignment operator/(const Alignment& rhs) const SKR_NOEXCEPT { return { x / rhs.x, y / rhs.y }; }
    inline Alignment operator%(const Alignment& rhs) const SKR_NOEXCEPT { return { std::fmod(x, rhs.x), std::fmod(y, rhs.y) }; }

    inline Alignment& operator+=(float rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Alignment& operator-=(float rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Alignment& operator*=(float rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Alignment& operator/=(float rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Alignment& operator%=(float rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline Alignment& operator+=(const Alignment& rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Alignment& operator-=(const Alignment& rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Alignment& operator*=(const Alignment& rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Alignment& operator/=(const Alignment& rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Alignment& operator%=(const Alignment& rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    // alignment along
    inline Offset<float> along_offset(const Offset<float>& offset) const SKR_NOEXCEPT { return { offset.x * x, offset.y * y }; }
    inline Offset<float> along_size(const Size<float>& size) const SKR_NOEXCEPT { return { size.width * x, size.height * y }; }
    inline Offset<float> along_rect(const Rect<float>& rect) const SKR_NOEXCEPT { return { rect.left + rect.width() * x, rect.top + rect.height() * y }; }

    // compare
    inline constexpr bool operator==(const Alignment& rhs) const SKR_NOEXCEPT { return x == rhs.x && y == rhs.y; }
    inline constexpr bool operator!=(const Alignment& rhs) const SKR_NOEXCEPT { return x != rhs.x || y != rhs.y; }

    // location rect
    inline Rect<float> inscribe(const Size<float>& child_size, const Rect<float>& parent_rect) const SKR_NOEXCEPT
    {
        const Size<float> size_delta = parent_rect.size() - child_size;
        return Rect<float>::LTWH(parent_rect.left + size_delta.width * x, parent_rect.top + size_delta.height * y, child_size.width, child_size.height);
    }

    // lerp
    inline Alignment lerp(const Alignment& rhs, float t) const SKR_NOEXCEPT { return { _lerp(x, rhs.x, t), _lerp(y, rhs.y, t) }; }
};

using Offsetf     = Offset<float>;
using Sizef       = Size<float>;
using Rectf       = Rect<float>;
using EdgeInsetsf = Rect<float>;
using Offseti     = Offset<int32_t>;
using Sizei       = Size<int32_t>;
using Recti       = Rect<int32_t>;
using EdgeInsetsi = Rect<int32_t>;

// TODO. Ray Hit Test
struct Ray {
    skr_float3_t origin;
    skr_float3_t direction;
};

#undef SKR_GUI_MATH_ENABLE_IF_FLOAT
#undef SKR_GUI_MATH_UTILITY_MIXIN
} // namespace skr::gui