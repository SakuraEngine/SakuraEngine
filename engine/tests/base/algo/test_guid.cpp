#include <string.h>
#include <iostream>
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/types.h"

// helpers
inline bool _str_equal(const char8_t* a, const char8_t* b)
{
    return std::char_traits<char8_t>::compare(a, b, std::char_traits<char8_t>::length(a)) == 0;
}

TEST_CASE("test guid")
{
    using namespace skr;

    SUBCASE("test literal")
    {
        constexpr GUID literal1 = u8"{1c422611-31df-4a0b-a917-eebc29cc7217}"_guid;
        constexpr GUID literal2 = u8"1c422611-31df-4a0b-a917-eebc29cc7217"_guid;
        constexpr GUID literal3 = u8"1c42261131df4a0ba917eebc29cc7217"_guid;

        GUID dynamic1 = *GUID::Decode(
            u8"{1c422611-31df-4a0b-a917-eebc29cc7217}",
            GUID::EStyle::HexSpecBrace
        );
        GUID dynamic2 = *GUID::Decode(
            u8"1c422611-31df-4a0b-a917-eebc29cc7217",
            GUID::EStyle::HexSpec
        );
        GUID dynamic3 = *GUID::Decode(
            u8"1c42261131df4a0ba917eebc29cc7217",
            GUID::EStyle::Hex
        );

        REQUIRE_EQ(literal1, literal2);
        REQUIRE_EQ(literal2, literal3);

        REQUIRE_EQ(dynamic1, dynamic2);
        REQUIRE_EQ(dynamic2, dynamic3);

        REQUIRE_EQ(literal1, dynamic1);
        REQUIRE_EQ(literal2, dynamic2);
        REQUIRE_EQ(literal3, dynamic3);
    }

#define ENCODE_AND_CHECK(__STR, __LEN, ...) \
    buffer.clear();                         \
    buffer.resize(__LEN + 1, 0);                \
    __VA_ARGS__;                            \
    REQUIRE(_str_equal(buffer.data(), __STR));

    SUBCASE("test decode and encode")
    {
        const char8_t* str_hex = u8"1c42261131df4a0ba917eebc29cc7217";
        const char8_t* str_hex_spec = u8"1c422611-31df-4a0b-a917-eebc29cc7217";
        const char8_t* str_hex_spec_brace = u8"{1c422611-31df-4a0b-a917-eebc29cc7217}";
        const char8_t* str_hex_custom_spec = u8"1c422611_31df_4a0b_a917_eebc29cc7217";
        const char8_t* str_hex_custom_spec_brace = u8"(1c422611_31df_4a0b_a917_eebc29cc7217)";
        const char8_t* str_hex_upper = u8"1C42261131DF4A0BA917EEBC29CC7217";
        const char8_t* str_hex_mixed = u8"1C42261131DF4A0BA917eebc29cc7217";
        const char8_t* str_base64 = u8"HEImETHfSgupF+68KcxyFw";
        const char8_t* str_base64_padded = u8"HEImETHfSgupF+68KcxyFw==";

        GUID guid_hex = *GUID::DecodeAuto(str_hex);
        GUID guid_hex_spec = *GUID::DecodeAuto(str_hex_spec);
        GUID guid_hex_spec_brace = *GUID::DecodeAuto(str_hex_spec_brace);
        GUID guid_hex_custom_spec = *GUID::DecodeAuto(str_hex_custom_spec);
        GUID guid_hex_custom_spec_brace = *GUID::DecodeAuto(str_hex_custom_spec_brace);
        GUID guid_hex_upper = *GUID::DecodeAuto(str_hex_upper);
        GUID guid_hex_mixed = *GUID::DecodeAuto(str_hex_mixed);
        GUID guid_base64 = *GUID::DecodeAuto(str_base64);
        GUID guid_base64_padded = *GUID::DecodeAuto(str_base64_padded);

        REQUIRE_EQ(guid_hex, guid_hex_spec);
        REQUIRE_EQ(guid_hex, guid_hex_spec_brace);
        REQUIRE_EQ(guid_hex, guid_hex_custom_spec);
        REQUIRE_EQ(guid_hex, guid_hex_custom_spec_brace);
        REQUIRE_EQ(guid_hex, guid_hex_upper);
        REQUIRE_EQ(guid_hex, guid_hex_mixed);
        REQUIRE_EQ(guid_hex, guid_base64);
        REQUIRE_EQ(guid_hex, guid_base64_padded);

        std::vector<skr_char8> buffer;

        // test format functions
        ENCODE_AND_CHECK(str_hex, GUID::kHexLength, guid_hex.encode_hex(buffer.data()));
        ENCODE_AND_CHECK(str_hex_spec, GUID::kHexSpecLength, guid_hex.encode_spec(buffer.data()));
        ENCODE_AND_CHECK(str_hex_spec_brace, GUID::kHexSpecBraceLength, guid_hex.encode_spec_brace(buffer.data()));
        ENCODE_AND_CHECK(str_hex_custom_spec, GUID::kHexSpecLength, guid_hex.encode_spec(buffer.data(), u8'_'));
        ENCODE_AND_CHECK(str_hex_custom_spec_brace, GUID::kHexSpecBraceLength, guid_hex.encode_spec_brace(buffer.data(), u8'_', u8'(', u8')'));
        ENCODE_AND_CHECK(str_hex_upper, GUID::kHexLength, guid_hex.encode_hex(buffer.data(), false));
        ENCODE_AND_CHECK(str_base64, GUID::kBase64Length, guid_hex.encode_base64(buffer.data()));
        ENCODE_AND_CHECK(str_base64_padded, GUID::kBase64PaddedLength, guid_hex.encode_base64(buffer.data(), true));

        // test standard encode
        ENCODE_AND_CHECK(str_hex, GUID::kHexLength, guid_hex.encode(buffer.data(), GUID::EStyle::Hex));
        ENCODE_AND_CHECK(str_hex_spec, GUID::kHexSpecLength, guid_hex.encode(buffer.data(), GUID::EStyle::HexSpec));
        ENCODE_AND_CHECK(str_hex_spec_brace, GUID::kHexSpecBraceLength, guid_hex.encode(buffer.data(), GUID::EStyle::HexSpecBrace));
        ENCODE_AND_CHECK(str_base64, GUID::kBase64Length, guid_hex.encode(buffer.data(), GUID::EStyle::Base64));
        ENCODE_AND_CHECK(str_base64_padded, GUID::kBase64PaddedLength, guid_hex.encode(buffer.data(), GUID::EStyle::Base64Padded));
    }
#undef ENCODE_AND_CHECK
}