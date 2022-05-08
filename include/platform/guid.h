#pragma once
#include <inttypes.h>

typedef struct skr_guid_t {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} skr_guid_t;

#if defined(__cplusplus)
    #include <EASTL/string.h>
    #include <string_view>
    #include "platform/debug.h"
    #include "utils/hash.h"
inline bool operator==(const skr_guid_t& a, const skr_guid_t& b)
{
    return std::memcmp(&a, &b, sizeof(skr_guid_t)) == 0;
}
namespace skr::guid
{
struct hash {
    size_t operator()(const skr_guid_t& a) const
    {
        return skr_hash(&a, sizeof(skr_guid_t), 0);
    }
};
namespace details
{
constexpr const size_t short_guid_form_length = 36; // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
constexpr const size_t long_guid_form_length = 38;  // {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

//
constexpr int parse_hex_digit(const char c)
{
    using namespace eastl::string_literals;
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        assert(0 && "invalid character in GUID");
    return -1;
}

template <class T>
constexpr T parse_hex(const char* ptr)
{
    constexpr size_t digits = sizeof(T) * 2;
    T result{};
    for (size_t i = 0; i < digits; ++i)
        result |= parse_hex_digit(ptr[i]) << (4 * (digits - i - 1));
    return result;
}

constexpr skr_guid_t make_guid_helper(const char* begin)
{
    skr_guid_t result{};
    result.Data1 = parse_hex<uint32_t>(begin);
    begin += 8 + 1;
    result.Data2 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    result.Data3 = parse_hex<uint16_t>(begin);
    begin += 4 + 1;
    result.Data4[0] = parse_hex<uint8_t>(begin);
    begin += 2;
    result.Data4[1] = parse_hex<uint8_t>(begin);
    begin += 2 + 1;
    for (size_t i = 0; i < 6; ++i)
        result.Data4[i + 2] = parse_hex<uint8_t>(begin + i * 2);
    return result;
}

template <size_t N>
constexpr skr_guid_t make_guid(const char (&str)[N])
{
    using namespace eastl::string_literals;
    static_assert(N == (long_guid_form_length + 1) || N == (short_guid_form_length + 1), "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");

    if constexpr (N == (long_guid_form_length + 1))
    {
        if (str[0] != '{' || str[long_guid_form_length - 1] != '}')
            SKR_ASSERT(0 && "Missing opening or closing brace");
    }

    return make_guid_helper(str + (N == (long_guid_form_length + 1) ? 1 : 0));
}
constexpr skr_guid_t make_guid(const std::string_view& str)
{
    using namespace eastl::string_literals;
    if (str.size() != (long_guid_form_length + 1) && str.size() == (short_guid_form_length + 1))
        SKR_ASSERT(0 && "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");

    if (str.size() == (long_guid_form_length + 1))
    {
        if (str[0] != '{' || str[long_guid_form_length - 1] != '}')
            SKR_ASSERT(0 && "Missing opening or closing brace");
    }

    return make_guid_helper(str.data() + (str.size() == (long_guid_form_length + 1) ? 1 : 0));
}
} // namespace details
using details::make_guid;

namespace literals
{
constexpr skr_guid_t operator""_guid(const char* str, size_t N)
{
    using namespace eastl::string_literals;
    using namespace details;

    if (!(N == long_guid_form_length || N == short_guid_form_length))
        assert(0 && "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");
    if (N == long_guid_form_length && (str[0] != '{' || str[long_guid_form_length - 1] != '}'))
        assert(0 && "Missing opening or closing brace");

    return make_guid_helper(str + (N == long_guid_form_length ? 1 : 0));
}
} // namespace literals

} // namespace skr::guid
#endif