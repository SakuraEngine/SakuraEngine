#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/bit.hpp"

// ============================================================================
// Base16 (Hexadecimal) Implementation
// use [0-9] and [A-F], case insensitive
// 2 char => 1 byte, no padding
// write group by big-endian
// ============================================================================
namespace skr
{
// validation
bool base16_is_valid_char(char8_t c);
bool base16_is_valid_string(const char8_t* str, uint64_t len);

// length calculation
uint64_t base16_encode_length(uint64_t byte_length);
uint64_t base16_decode_length(uint64_t char_length);

// ensure out_str has enough space use base16_encode_length
void base16_encode(
    skr_char8*  out_str,
    const void* in_data,
    uint64_t    in_data_length,
    bool        lower = false
);

// if meet invalid char, parse will stop, and return false
// if out_data_length is not enough, parse will stop, but still return true
bool base16_decode(
    void*            out_data,
    uint64_t         out_data_length,
    const skr_char8* in_str,
    uint64_t         in_str_length
);
} // namespace skr

// ============================================================================
// Base32 (RFC 4648) Implementation
// use [A-Z] and [2-7], case insensitive
// 8 char => 5 byte, padding with '='
// write group by big-endian
// ============================================================================
namespace skr
{
// validation
bool base32_is_valid_char(char8_t c);
bool base32_is_valid_string(const char8_t* str, uint64_t len);

// length calculation
//! Note, decode length is approximate, actual length may be less due to padding and incomplete groups
uint64_t base32_encode_length(uint64_t byte_length, bool with_padding = true);
uint64_t base32_decode_length(uint64_t char_length);

// padding check
bool base32_is_strictly_padded(uint64_t char_length);

// minimal decode length, will parse last group and discard 0 bits at the end
uint64_t base32_decode_length_minimal(const skr_char8* str, uint64_t char_length);

// ensure out_str has enough space use base32_encode_length
void base32_encode(
    skr_char8*  out_str,
    const void* in_data,
    uint64_t    in_data_length,
    bool        with_padding = true,
    bool        lower        = false
);

// if meet invalid char, parse will stop, and return false
// if out_data_length is not enough, parse will stop, but still return true
bool base32_decode(
    void*            out_data,
    uint64_t         out_data_length,
    const skr_char8* in_str,
    uint64_t         in_str_length
);
} // namespace skr

// ============================================================================
// Base64 (RFC 4648) Implementation
// use [A-Z], [a-z], [0-9], '+', '/'
// 4 char => 3 byte, padding with '='
// write group by big-endian
// ============================================================================
namespace skr
{
// validation
bool base64_is_valid_char(char8_t c);
bool base64_is_valid_string(const char8_t* str, uint64_t len);

// length calculation
//! Note, decode length is approximate, actual length may be less due to padding and incomplete groups
uint64_t base64_encode_length(uint64_t byte_length, bool with_padding = true);
uint64_t base64_decode_length(uint64_t char_length);

// padding check
bool base64_is_strictly_padded(uint64_t char_length);

// minimal decode length, will parse last group and discard 0 bits at the end
uint64_t base64_decode_length_minimal(const skr_char8* str, uint64_t char_length);

// ensure out_str has enough space use base64_encode_length
void base64_encode(
    skr_char8*  out_str,
    const void* in_data,
    uint64_t    in_data_length,
    bool        with_padding = true
);

// if meet invalid char, parse will stop, and return false
// if out_data_length is not enough, parse will stop, but still return true
bool base64_decode(
    void*            out_data,
    uint64_t         out_data_length,
    const skr_char8* in_str,
    uint64_t         in_str_length
);
} // namespace skr

// ============================================================================
// Hex encoding, same as base16, but typed, and optimized for consteval
// use [0-9] and [A-F], case insensitive
// 2 char => 1 byte, no padding
// write group by big-endian,
// ============================================================================
namespace skr
{
// some consteval api for GUID/MD5 literals
constexpr uint8_t hex_decode_char(const char8_t c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        return 0xFF; // invalid
}
template <typename T>
constexpr bool hex_decode(const skr_char8* str, uint64_t len, T& out_value)
{
    static_assert(std::is_integral_v<T>, "hex_decode only supports integral types");
    constexpr size_t byte_len = sizeof(T);
    constexpr size_t char_len = byte_len * 2;

    if (len != char_len)
    {
        return false;
    }

    out_value = {};
    for (size_t i = 0; i < char_len; ++i)
    {
        auto segment = hex_decode_char(str[i]);
        if (segment == 0xFF) [[unlikely]]
        {
            return false;
        }
        out_value |= segment << (4 * (char_len - i - 1));
    }
    return true;
}
constexpr void hex_encode_char(uint8_t val, skr_char8* str, bool lower = false)
{
    constexpr skr_char8 kBase16TableLower[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    constexpr skr_char8 kBase16TableUpper[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    auto table = lower ? kBase16TableLower : kBase16TableUpper;
    str[0]     = table[(val >> 4) & 0x0F];
    str[1]     = table[(val >> 0) & 0x0F];
}
template <typename T>
constexpr void hex_encode(T val, skr_char8* str, uint64_t len, bool lower = false)
{
    static_assert(std::is_integral_v<T>, "hex_encode only supports integral types");
    constexpr size_t byte_len = sizeof(T);
    constexpr size_t char_len = byte_len * 2;

    if (len != char_len)
    {
        SKR_ASSERT(0 && "invalid length for base16 consteval encode");
        return;
    }

    for (size_t i = 0; i < byte_len; ++i)
    {
        auto byte_val = (val >> (8 * (byte_len - i - 1))) & 0xFF;
        hex_encode_char(static_cast<uint8_t>(byte_val), &str[i * 2], lower);
    }
}
} // namespace skr
