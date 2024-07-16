#pragma once
#include "SkrBase/config.h"
#include <cstdint>
#include <limits>
#include <type_traits>
#include <concepts>

// flag tools
namespace skr
{
template <typename T>
SKR_INLINE constexpr bool flag_all(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    using UT = std::underlying_type_t<T>;
    return (static_cast<UT>(val) & static_cast<UT>(flags)) == static_cast<UT>(flags);
}
template <typename T>
SKR_INLINE constexpr bool flag_any(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    using UT = std::underlying_type_t<T>;
    return static_cast<UT>(val) & static_cast<UT>(flags);
}
template <typename T>
SKR_INLINE constexpr T flag_set(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    using UT = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<UT>(val) | static_cast<UT>(flags));
}
template <typename T>
SKR_INLINE constexpr T flag_erase(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    using UT = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<UT>(val) & (~static_cast<UT>(flags)));
}
} // namespace skr

// global operator tools
inline namespace scoped_enum_tools
{
template <typename E>
concept ScopedEnum = requires {
    requires std::is_enum_v<E>;
    requires !std::is_convertible_v<E, std::underlying_type_t<E>>;
};

template <ScopedEnum E>
SKR_INLINE constexpr E operator|(E a, E b) noexcept
{
    using UT = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<UT>(a) | static_cast<UT>(b));
}

template <ScopedEnum E>
SKR_INLINE constexpr E operator&(E a, E b) noexcept
{
    static_assert(std::is_same_v<E, E*>, "please use flag_any or flag_all instead of operator&");
}
} // namespace scoped_enum_tools

// integer div
namespace skr
{
template <typename T>
SKR_INLINE constexpr T int_div_ceil(T a, T b)
{
    static_assert(std::is_integral_v<T>);
    return (a + b - 1) / b;
}
template <typename T>
SKR_INLINE constexpr T int_div_floor(T a, T b)
{
    static_assert(std::is_integral_v<T>);
    return a / b;
}
template <typename T>
SKR_INLINE constexpr T int_div_round(T a, T b)
{
    static_assert(std::is_integral_v<T>);
    return (a >= 0) ? (a + b / 2) / b : (a - b / 2 + 1) / b;
}
} // namespace skr

// npos
namespace skr
{
template <typename T>
inline constexpr T npos_of = static_cast<T>(-1);

template <typename T>
inline constexpr T max_size_of = std::is_signed_v<T> ? std::numeric_limits<T>::max() : std::numeric_limits<T>::max() - 1;
} // namespace skr