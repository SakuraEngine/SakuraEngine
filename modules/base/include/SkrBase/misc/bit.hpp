#pragma once
#include "SkrBase/config.h"
#include <cstdint>
#include <limits>
#include <type_traits>
#if SKR_CXX_VERSION >= 20
    #include <bit>
#endif

namespace skr
{
// countLZero & countRZero
// 得到 bit 中左（高位起）和右（低位起）的 0 个数
template <typename T>
constexpr T countl_zero(T v);
template <typename T>
constexpr T countr_zero(T v);

// countLOne & countROne
// 得到 bit 中左（高位起）和右（低位起）的 1 个数
template <typename T>
constexpr T countl_one(T v);
template <typename T>
constexpr T countr_one(T v);

// bitWidth
// 得到存储这个数所需要的位数
template <typename T>
constexpr T bit_width(T v);

// bitFloor & bitCeil
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）
template <typename T>
constexpr T bit_floor(T v);
template <typename T>
constexpr T bit_ceil(T v);

// bitFloorLog2 & bitCeilLog2
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）的幂（n）
template <typename T>
constexpr T bit_floor_log2(T v);
template <typename T>
constexpr T bit_ceil_log2(T v);

// popCount
// 得到 bit 中 1 的个数
template <typename T>
constexpr T pop_count(T v);
} // namespace skr

// fallback
namespace skr::detail
{
template <typename T>
SKR_INLINE constexpr T countl_zero_fallback(T v)
{
    T    ret = 0;
    auto n   = std::numeric_limits<T>::digits;
    auto c   = std::numeric_limits<T>::digits / 2;
    do
    {
        ret = static_cast<T>(v >> c);
        if (ret != 0)
        {
            n -= c;
            v = ret;
        }
        c >>= 1;
    } while (c != 0);
    return static_cast<T>(n) - static_cast<T>(v);
}
template <typename T>
SKR_INLINE constexpr T countr_zero_fallback(T v)
{
    auto digits = std::numeric_limits<T>::digits;
    return digits - countl_zero(static_cast<T>(static_cast<T>(~v) & static_cast<T>(v - 1)));
}

template <typename T>
SKR_INLINE T pop_count_fallback(T v)
{
    auto digits = std::numeric_limits<T>::digits;
    v           = static_cast<T>(v - ((v >> 1) & static_cast<T>(0x5555'5555'5555'5555ull)));
    v           = static_cast<T>((v & static_cast<T>(0x3333'3333'3333'3333ull)) + ((v >> 2) & static_cast<T>(0x3333'3333'3333'3333ull)));
    v           = static_cast<T>((v + (v >> 4)) & static_cast<T>(0x0F0F'0F0F'0F0F'0F0Full));
    v           = static_cast<T>(v * static_cast<T>(0x0101'0101'0101'0101ull));
    return static_cast<int>(v >> (digits - 8));
}
} // namespace skr::detail

namespace skr
{
// countLZero & countRZero
// 得到 bit 中左（高位起）和右（低位起）的 0 个数
template <typename T>
SKR_INLINE constexpr T countl_zero(T v)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
#if SKR_CXX_VERSION >= 20
    return std::countl_zero(v);
#else
    return detail::countl_zero_fallback(v);
#endif
}
template <typename T>
SKR_INLINE constexpr T countr_zero(T v)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
#if SKR_CXX_VERSION >= 20
    return std::countr_zero(v);
#else
    return detail::countr_zero_fallback(v);
#endif
}

// countLOne & countROne
// 得到 bit 中左（高位起）和右（低位起）的 1 个数
template <typename T>
SKR_INLINE constexpr T countl_one(T v)
{
#if SKR_CXX_VERSION >= 20
    return std::countl_one(v);
#else
    return countl_zero(static_cast<T>(~v));
#endif
}
template <typename T>
SKR_INLINE constexpr T countr_one(T v)
{
#if SKR_CXX_VERSION >= 20
    return std::countr_one(v);
#else
    return countr_zero(static_cast<T>(~v));
#endif
}

// bitWidth
// 得到存储这个数所需要的位数
template <typename T>
SKR_INLINE constexpr T bit_width(T v)
{
#if SKR_CXX_VERSION >= 20
    return std::bit_width(v);
#else
    return std::numeric_limits<T>::digits - countl_zero(v);
#endif
}

// bitFloor & bitCeil
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）
template <typename T>
SKR_INLINE constexpr T bit_floor(T v)
{
#if SKR_CXX_VERSION >= 20
    return std::bit_floor(v);
#else
    return v == 0 ? 0 : static_cast<T>(T(1) << (std::numeric_limits<T>::digits - 1 - countl_zero(v)));
#endif
}
template <typename T>
SKR_INLINE constexpr T bit_ceil(T v)
{
#if SKR_CXX_VERSION >= 20
    return std::bit_ceil(v);
#else
    return v <= 1 ? 1 : static_cast<T>(T(1) << (std::numeric_limits<T>::digits - countl_zero(static_cast<T>(v - 1))));
#endif
}

// bitFloorLog2 & bitCeilLog2
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）的幂（n）
template <typename T>
SKR_INLINE constexpr T bit_floor_log2(T v)
{
    return v == 0 ? 0 : std::numeric_limits<T>::digits - 1 - countl_zero(v);
}
template <typename T>
SKR_INLINE constexpr T bit_ceil_log2(T v)
{
    return v <= 1 ? 0 : std::numeric_limits<T>::digits - countl_zero(static_cast<T>(v - 1));
}

// popCount
// 得到 bit 中 1 的个数
template <typename T>
SKR_INLINE constexpr T pop_count(T v)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
#if SKR_CXX_VERSION >= 20
    return std::popcount(v);
#else
    return detail::pop_count_fallback(v);
#endif
}
} // namespace skr