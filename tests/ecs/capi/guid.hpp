#pragma once
#include "platform/debug.h"
#include "ecs/dual.h"
#include <string>

namespace guid_parse
{
    using guid_t = dual_guid_t;
    namespace details
    {
        constexpr const size_t short_guid_form_length = 36;	// XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
        constexpr const size_t long_guid_form_length = 38;	// {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

        //
        constexpr int parse_hex_digit(const char8_t c)
        {
            using namespace std::string_literals;
            if ('0' <= c && c <= '9')
                return c - '0';
            else if ('a' <= c && c <= 'f')
                return 10 + c - 'a';
            else if ('A' <= c && c <= 'F')
                return 10 + c - 'A';
            else
                SKR_ASSERT(0 && "invalid character in GUID");
            return -1;
        }

        template<class T>
        constexpr T parse_hex(const char8_t* ptr)
        {
            constexpr size_t digits = sizeof(T) * 2;
            T result{};
            for (size_t i = 0; i < digits; ++i)
                result |= parse_hex_digit(ptr[i]) << (4 * (digits - i - 1));
            return result;
        }

        constexpr guid_t make_guid_helper(const char8_t* begin)
        {
            auto Data1 = parse_hex<uint32_t>(begin);
            begin += 8 + 1;
            auto Data2 = parse_hex<uint16_t>(begin);
            begin += 4 + 1;
            auto Data3 = parse_hex<uint16_t>(begin);
            begin += 4 + 1;
            uint8_t Data4[8] = {};
            Data4[0] = parse_hex<uint8_t>(begin);
            begin += 2;
            Data4[1] = parse_hex<uint8_t>(begin);
            begin += 2 + 1;
            for (size_t i = 0; i < 6; ++i)
                Data4[i + 2] = parse_hex<uint8_t>(begin + i * 2);
            return guid_t(Data1, Data2, Data3, Data4);
        }

        template<size_t N>
        constexpr guid_t make_guid(const char8_t(&str)[N])
        {
            using namespace std::string_literals;
            static_assert(N == (long_guid_form_length + 1) || N == (short_guid_form_length + 1), "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");

            if constexpr (N == (long_guid_form_length + 1))
            {
                if (str[0] != '{' || str[long_guid_form_length - 1] != '}')
                    SKR_ASSERT(0 && "Missing opening or closing brace");
            }

            return make_guid_helper(str + (N == (long_guid_form_length + 1) ? 1 : 0));
        }
    }
    using details::make_guid;

    namespace literals
    {
        constexpr guid_t operator ""_guid(const char8_t* str, size_t N)
        {
            using namespace std::string_literals;
            using namespace details;

            if (!(N == long_guid_form_length || N == short_guid_form_length))
                assert(0 && "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");
            if (N == long_guid_form_length && (str[0] != '{' || str[long_guid_form_length - 1] != '}'))
                assert(0 && "Missing opening or closing brace");

            return make_guid_helper(str + (N == long_guid_form_length ? 1 : 0));
        }
    }

}