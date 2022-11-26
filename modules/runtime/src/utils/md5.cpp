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
    if (!parse_hex(begin, value.a))
        return false;
    if (!parse_hex(begin + 8, value.b))
        return false;
    if (!parse_hex(begin + 16, value.c))
        return false;
    if (!parse_hex(begin + 24, value.d))
        return false;
    return true;
}

bool skr_parse_md5(const char* str32, skr_md5_t* out_md5)
{
    skr::string_view sv(str32, 32);
    if (!out_md5) return false;
    return make_md5(sv, *out_md5);
}