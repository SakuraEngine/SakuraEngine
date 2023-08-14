#pragma once
#include "SkrBase/config.h"
#include <cstdint>
#include <limits>
#include <type_traits>

// TODO. 使用 EASTL 的实现
namespace skr
{
// countLZero & countRZero
// 得到 bit 中左（高位起）和右（低位起）的 0 个数
template <typename T>
T countl_zero(T v);
template <typename T>
T countr_zero(T v);

// countLOne & countROne
// 得到 bit 中左（高位起）和右（低位起）的 1 个数
template <typename T>
T countl_one(T v);
template <typename T>
T countr_one(T v);

// bitWidth
// 得到存储这个数所需要的位数
template <typename T>
T bit_width(T v);

// bitFloor & bitCeil
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）
template <typename T>
T bit_floor(T v);
template <typename T>
T bit_ceil(T v);

// bitFloorLog2 & bitCeilLog2
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）的幂（n）
template <typename T>
T bit_floor_log2(T v);
template <typename T>
T bit_ceil_log2(T v);

// popCount
// 得到 bit 中 1 的个数
template <typename T>
T pop_count(T v);
} // namespace skr

// fallback
namespace skr::detail
{
template <typename T>
SKR_INLINE T countl_zero_fallback(T v)
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
SKR_INLINE T countr_zero_fallback(T v)
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
SKR_INLINE T countl_zero(T v)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);

    return detail::countl_zero_fallback(v);
}
template <typename T>
SKR_INLINE T countr_zero(T v)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);

    return detail::countr_zero_fallback(v);
}

// countLOne & countROne
// 得到 bit 中左（高位起）和右（低位起）的 1 个数
template <typename T>
SKR_INLINE T countl_one(T v) { return countl_zero(static_cast<T>(~v)); }
template <typename T>
SKR_INLINE T countr_one(T v) { return countr_zero(static_cast<T>(~v)); }

// bitWidth
// 得到存储这个数所需要的位数
template <typename T>
T bit_width(T v) { return std::numeric_limits<T>::digits - countl_zero(v); }

// bitFloor & bitCeil
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）
template <typename T>
T bit_floor(T v)
{
    return v == 0 ? 0 : static_cast<T>(T(1) << (std::numeric_limits<T>::digits - 1 - countl_zero(v)));
}
template <typename T>
T bit_ceil(T v)
{
    return v <= 1 ? 1 : static_cast<T>(T(1) << (std::numeric_limits<T>::digits - countl_zero(static_cast<T>(v - 1))));
}

// bitFloorLog2 & bitCeilLog2
// 相当于得到最近的一个小于/大于该数的二次幂（2^n）的幂（n）
template <typename T>
T bit_floor_log2(T v)
{
    return v == 0 ? 0 : std::numeric_limits<T>::digits - 1 - countl_zero(v);
}
template <typename T>
T bit_ceil_log2(T v)
{
    return v <= 1 ? 0 : std::numeric_limits<T>::digits - countl_zero(static_cast<T>(v - 1));
}

// popCount
// 得到 bit 中 1 的个数
template <typename T>
T pop_count(T v)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
    return detail::pop_count_fallback(v);
}
} // namespace skr