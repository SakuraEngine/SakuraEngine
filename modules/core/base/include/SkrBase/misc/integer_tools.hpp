#pragma once
#include "SkrBase/config.h"
#include <cstdint>
#include <limits>
#include <type_traits>

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
    return static_cast<UT>(val) | static_cast<UT>(flags);
}
template <typename T>
SKR_INLINE constexpr T flag_erase(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    using UT = std::underlying_type_t<T>;
    return static_cast<UT>(val) & (~static_cast<UT>(flags));
}
} // namespace skr

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