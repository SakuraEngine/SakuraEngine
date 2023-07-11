#pragma once
#include "SkrRT/base/config.hpp"
#include <cstdint>
#include <limits>
#include <type_traits>

// flag tools
namespace skr
{
template <typename T>
SKR_INLINE bool flag_all(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    return (val & flags) == flags;
}
template <typename T>
SKR_INLINE bool flag_any(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    return val & flags;
}
template <typename T>
SKR_INLINE T flag_set(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    return val | flags;
}
template <typename T>
SKR_INLINE T flag_erase(T val, T flags) noexcept
{
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    return val & (~flags);
}
} // namespace skr

// integer div
namespace skr
{
template <typename T>
SKR_INLINE T int_div_ceil(T a, T b)
{
    static_assert(std::is_integral_v<T>);
    return (a + b - 1) / b;
}
template <typename T>
SKR_INLINE T int_div_floor(T a, T b)
{
    static_assert(std::is_integral_v<T>);
    return a / b;
}
template <typename T>
SKR_INLINE T int_div_round(T a, T b)
{
    static_assert(std::is_integral_v<T>);
    return (a >= 0) ? (a + b / 2) / b : (a - b / 2 + 1) / b;
}
} // namespace skr
