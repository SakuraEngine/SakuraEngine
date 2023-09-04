// TODO. move this file to SkrBase
#pragma once
#include <cstdint>
#include <type_traits>
#include "SkrBase/misc/hash.hpp"
#include "SkrRT/misc/macros.h"
#include "SkrRT/platform/configure.h"

// FIXME. temporal solution
#define SKR_IS_BIG_ENDIAN 0
#define SKR_IS_LITTLE_ENDIAN 1

namespace skr
{
struct GUID {
    SKR_INLINE constexpr GUID() = default;
    SKR_INLINE constexpr GUID(uint32_t b0, uint16_t b1, uint16_t b2, const uint8_t b3s[8])
        : _storage0(b0)
        , _storage1((uint32_t)b1 << 16 | (uint32_t)b2)
        ,
#if SKR_IS_LITTLE_ENDIAN
        _storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24)
        , _storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
#else
        Storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24)
        , Storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
#endif
    {
    }
    constexpr GUID(uint32_t b0, uint16_t b1, uint16_t b2, std::initializer_list<uint8_t> b3s_l)
        : _storage0(b0)
        , _storage1((uint32_t)b1 << 16 | (uint32_t)b2)
        ,
#define b3s (b3s_l).begin()
#if SKR_IS_LITTLE_ENDIAN
        _storage2((uint32_t)b3s[0] | (uint32_t)b3s[1] << 8 | (uint32_t)b3s[2] << 16 | (uint32_t)b3s[3] << 24)
        , _storage3((uint32_t)b3s[4] | (uint32_t)b3s[5] << 8 | (uint32_t)b3s[6] << 16 | (uint32_t)b3s[7] << 24)
#else
        Storage2((uint32_t)b3s[3] | (uint32_t)b3s[2] << 8 | (uint32_t)b3s[1] << 16 | (uint32_t)b3s[0] << 24)
        , Storage3((uint32_t)b3s[7] | (uint32_t)b3s[6] << 8 | (uint32_t)b3s[5] << 16 | (uint32_t)b3s[4] << 24)
#endif
#undef b3s
    {
    }

    constexpr bool     is_zero() const { return !(_storage0 && _storage1 && _storage2 && _storage3); }
    constexpr uint32_t data1() const { return _storage0; }
    constexpr uint16_t data2() const { return (uint16_t)(_storage1 >> 16); }
    constexpr uint16_t data3() const { return (uint16_t)(_storage1 & UINT16_MAX); }
    constexpr uint16_t data4(uint8_t idx0_7) const { return ((uint8_t*)&_storage2)[idx0_7]; }

    SKR_INLINE constexpr bool operator==(const GUID& rhs) const
    {
        return _storage0 == rhs._storage0 && _storage1 == rhs._storage1 && _storage2 == rhs._storage2 && _storage3 == rhs._storage3;
    }
    SKR_INLINE constexpr bool operator!=(const GUID& rhs) const { return !(*this == rhs); }
    SKR_INLINE constexpr bool operator<(const GUID& rhs) const
    {
        return _storage0 < rhs._storage0 || (!(_storage0 < rhs._storage0) && _storage1 < rhs._storage1) ||
               (!(_storage0 < rhs._storage0) && !(_storage1 < rhs._storage1) && _storage2 < rhs._storage2) ||
               (!(_storage0 < rhs._storage0) && !(_storage1 < rhs._storage1) && !(_storage2 < rhs._storage2) && _storage3 < rhs._storage3);
    }
    SKR_INLINE constexpr bool operator>(const GUID& rhs) const { return rhs < *this; }
    SKR_INLINE constexpr bool operator<=(const GUID& rhs) const { return !(*this > rhs); }
    SKR_INLINE constexpr bool operator>=(const GUID& rhs) const { return !(*this < rhs); }

    SKR_INLINE size_t _skr_hash() const
    {
        Hash<uint32_t> hasher;

        size_t result = hasher(_storage0);
        result        = hash_combine(result, hasher(_storage1));
        result        = hash_combine(result, hasher(_storage2));
        result        = hash_combine(result, hasher(_storage3));
        return result;
    }

private:
    uint32_t _storage0 = 0;
    uint32_t _storage1 = 0;
    uint32_t _storage2 = 0;
    uint32_t _storage3 = 0;
};
} // namespace skr

namespace skr
{
namespace __help
{
constexpr int parse_hex_digit(const char8_t c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        return -1;
}
template <class T>
constexpr T parse_hex(const char8_t* ptr)
{
    constexpr size_t digits = sizeof(T) * 2;
    T                result{};
    for (size_t i = 0; i < digits; ++i)
        result |= parse_hex_digit(ptr[i]) << (4 * (digits - i - 1));
    return result;
}
constexpr GUID make_guid_helper(const char8_t* begin)
{
    auto Data1 = parse_hex<uint32_t>(begin);
    begin += 8 + 1;
    auto Data2 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    auto Data3 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    uint8_t Data4[8] = {};
    Data4[0]         = parse_hex<uint8_t>(begin);
    begin += 2;
    Data4[1] = parse_hex<uint8_t>(begin);
    begin += 2 + 1;
    for (size_t i = 0; i < 6; ++i)
        Data4[i + 2] = parse_hex<uint8_t>(begin + i * 2);
    return GUID(Data1, Data2, Data3, Data4);
}
} // namespace __help

constexpr GUID operator""_guid(const char8_t* str, size_t N)
{
    auto Data1 = __help::parse_hex<uint32_t>(str);
    str += 8 + 1;

    auto Data2 = __help::parse_hex<uint16_t>(str);
    str += 4 + 1;

    auto Data3 = __help::parse_hex<uint16_t>(str);
    str += 4 + 1;

    uint8_t Data4[8] = {};
    Data4[0]         = __help::parse_hex<uint8_t>(str);
    str += 2;

    Data4[1] = __help::parse_hex<uint8_t>(str);
    str += 2 + 1;

    for (size_t i = 0; i < 6; ++i)
    {
        Data4[i + 2] = __help::parse_hex<uint8_t>(str + i * 2);
    }
    return GUID(Data1, Data2, Data3, Data4);
}
} // namespace skr