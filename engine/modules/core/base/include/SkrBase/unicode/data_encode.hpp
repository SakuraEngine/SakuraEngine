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