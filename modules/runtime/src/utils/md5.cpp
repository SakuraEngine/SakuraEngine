#include "containers/string.hpp"
#include "utils/types.h"
#include "utils/log.h"

constexpr int parse_hex_digit(const char c)
{
    using namespace skr::string_literals;
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        SKR_LOG_ERROR("Invalid character in GUID. Expected hex digit, got %c", c);
    return -1;
}

template <class T>
bool parse_hex(const char* ptr, T& value)
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

bool make_md5(const skr::string_view& str, skr_md5_t& value)
{
    using namespace skr::string_literals;
    constexpr size_t md5_form_length = 32;

    if (str.size() != md5_form_length)
    {
        skr::string str2(str.data(), str.size());
        SKR_LOG_ERROR("String MD5 of the form XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX is expected, got %s", str2.c_str());
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
#include "./../crypt/md5.h"

bool skr_parse_md5(const char* str32, skr_md5_t* out_md5)
{
    skr::string_view sv(str32, 32);
    if (!out_md5) return false;
    return make_md5(sv, *out_md5);
}

void skr_make_md5(const char* str, uint32_t str_size, skr_md5_t* out_md5)
{
    return skr_crypt_make_md5(str, str_size, out_md5);
}
