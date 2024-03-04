#include <string>
#include "SkrBase/types.h"
#include "SkrBase/misc/debug.h"

constexpr int parse_hex_digit(const char8_t c)
{
    if (u8'0' <= c && c <= u8'9')
        return c - u8'0';
    else if (u8'a' <= c && c <= u8'f')
        return 10 + c - u8'a';
    else if (u8'A' <= c && c <= u8'F')
        return 10 + c - u8'A';
    else
        SKR_ASSERT((c == u8'0') && false && u8"Invalid character in GUID. Expected hex digit");
    return -1;
}

template <class T>
bool parse_hex(const char8_t* ptr, T& value)
{
    constexpr size_t digits = sizeof(T) * 2;
    value = 0;
    for (size_t i = 0; i < digits; ++i)
    {
        int result = parse_hex_digit(ptr[i]);
        if (result < 0)
            return false;
        value |= result << (4 * (digits - i - 1));
    }
    return true;
}

bool make_md5(const std::u8string_view& str, skr_md5_t& value)
{
    constexpr size_t md5_form_length = 32;

    if (str.size() != md5_form_length)
    {
        std::u8string str2(str);
        SKR_ASSERT(str2.c_str() && false && u8"String MD5 of the form XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX is expected");
        return false;
    }
    const auto begin = str.data();
    if (!parse_hex(begin, value.digest[0])) return false;
    if (!parse_hex(begin + 2, value.digest[1])) return false;
    if (!parse_hex(begin + 4, value.digest[2])) return false;
    if (!parse_hex(begin + 6, value.digest[3])) return false;
    if (!parse_hex(begin + 8, value.digest[4])) return false;
    if (!parse_hex(begin + 10, value.digest[5])) return false;
    if (!parse_hex(begin + 12, value.digest[6])) return false;
    if (!parse_hex(begin + 14, value.digest[7])) return false;
    if (!parse_hex(begin + 16, value.digest[8])) return false;
    if (!parse_hex(begin + 18, value.digest[9])) return false;
    if (!parse_hex(begin + 20, value.digest[10])) return false;
    if (!parse_hex(begin + 22, value.digest[11])) return false;
    if (!parse_hex(begin + 24, value.digest[12])) return false;
    if (!parse_hex(begin + 26, value.digest[13])) return false;
    if (!parse_hex(begin + 28, value.digest[14])) return false;
    if (!parse_hex(begin + 30, value.digest[15])) return false;    
    return true;
}

// exports
#include "crypt/md5.h"

SKR_EXTERN_C bool skr_parse_md5(const char8_t* str32, skr_md5_t* out_md5)
{
    std::u8string_view sv(str32, 32);
    if (!out_md5) return false;
    return make_md5(sv, *out_md5);
}

SKR_EXTERN_C void skr_make_md5(const char8_t* str, uint32_t str_size, skr_md5_t* out_md5)
{
    return skr_crypt_make_md5(str, str_size, out_md5);
}
