#include "SkrTestFramework/framework.hpp"

#include "SkrBase/unicode/unicode_algo.hpp"

TEST_CASE("Test Unicode")
{
    using namespace skr;

    SUBCASE("UTF-8 seq len")
    {
        const auto u8_len_4 = u8"🐓";
        const auto u8_len_3 = u8"鸡";
        const auto u8_len_2 = u8"Ĝ";
        const auto u8_len_1 = u8"G";

        const auto u16_len_4 = u"🐓";
        const auto u16_len_3 = u"鸡";
        const auto u16_len_2 = u"Ĝ";
        const auto u16_len_1 = u"G";

        const auto u32_len_4 = U"🐓";
        const auto u32_len_3 = U"鸡";
        const auto u32_len_2 = U"Ĝ";
        const auto u32_len_1 = U"G";

        REQUIRE_EQ(utf8_seq_len(u8_len_4[0]), 4);
        REQUIRE_EQ(utf8_seq_len(u8_len_3[0]), 3);
        REQUIRE_EQ(utf8_seq_len(u8_len_2[0]), 2);
        REQUIRE_EQ(utf8_seq_len(u8_len_1[0]), 1);

        REQUIRE_EQ(utf8_seq_len(u16_len_4[0]), 4);
        REQUIRE_EQ(utf8_seq_len(u16_len_3[0]), 3);
        REQUIRE_EQ(utf8_seq_len(u16_len_2[0]), 2);
        REQUIRE_EQ(utf8_seq_len(u16_len_1[0]), 1);

        REQUIRE_EQ(utf8_seq_len(u32_len_4[0]), 4);
        REQUIRE_EQ(utf8_seq_len(u32_len_3[0]), 3);
        REQUIRE_EQ(utf8_seq_len(u32_len_2[0]), 2);
        REQUIRE_EQ(utf8_seq_len(u32_len_1[0]), 1);

        REQUIRE_EQ(utf8_seq_len(u8_len_4[1]), 0);
        REQUIRE_EQ(utf8_seq_len(u16_len_4[1]), 4);

        REQUIRE_EQ(utf8_seq_len(static_cast<skr_char8>(0)), 1);
        REQUIRE_EQ(utf8_seq_len(static_cast<skr_char16>(0)), 1);
        REQUIRE_EQ(utf8_seq_len(static_cast<skr_char32>(0)), 1);
    }

    SUBCASE("UTF-16 seq len")
    {
        const auto u8_len_2 = u8"🐓";
        const auto u8_len_1 = u8"鸡";

        const auto u16_len_2 = u"🐓";
        const auto u16_len_1 = u"鸡";

        const auto u32_len_2 = U"🐓";
        const auto u32_len_1 = U"鸡";

        REQUIRE_EQ(utf16_seq_len(u8_len_2[0]), 2);
        REQUIRE_EQ(utf16_seq_len(u8_len_1[0]), 1);

        REQUIRE_EQ(utf16_seq_len(u16_len_2[0]), 2);
        REQUIRE_EQ(utf16_seq_len(u16_len_1[0]), 1);

        REQUIRE_EQ(utf16_seq_len(u32_len_2[0]), 2);
        REQUIRE_EQ(utf16_seq_len(u32_len_1[0]), 1);

        REQUIRE_EQ(utf16_seq_len(u8_len_2[1]), 0);
        REQUIRE_EQ(utf16_seq_len(u16_len_2[1]), 0);

        REQUIRE_EQ(utf16_seq_len(static_cast<skr_char8>(0)), 1);
        REQUIRE_EQ(utf16_seq_len(static_cast<skr_char16>(0)), 1);
        REQUIRE_EQ(utf16_seq_len(static_cast<skr_char32>(0)), 1);
    }

    SUBCASE("Convert")
    {
        uint64_t seq_index = 0;

        const auto u8_4 = u8"🐓";
        const auto u8_3 = u8"鸡";
        const auto u8_2 = u8"Ĝ";
        const auto u8_1 = u8"G";

        const auto u8_4_seq = utf8_parse_seq(u8_4, 0, 4, seq_index);
        const auto u8_3_seq = utf8_parse_seq(u8_3, 0, 3, seq_index);
        const auto u8_2_seq = utf8_parse_seq(u8_2, 0, 2, seq_index);
        const auto u8_1_seq = utf8_parse_seq(u8_1, 0, 1, seq_index);

        const auto u16_4 = u"🐓";
        const auto u16_3 = u"鸡";
        const auto u16_2 = u"Ĝ";
        const auto u16_1 = u"G";

        const auto u16_4_seq = utf16_parse_seq(u16_4, 0, 2, seq_index);
        const auto u16_3_seq = utf16_parse_seq(u16_3, 0, 1, seq_index);
        const auto u16_2_seq = utf16_parse_seq(u16_2, 0, 1, seq_index);
        const auto u16_1_seq = utf16_parse_seq(u16_1, 0, 1, seq_index);

        const auto u32_4 = U'🐓';
        const auto u32_3 = U'鸡';
        const auto u32_2 = U'Ĝ';
        const auto u32_1 = U'G';

        REQUIRE_EQ(utf8_to_utf16(u8_4_seq), u16_4_seq);
        REQUIRE_EQ(utf8_to_utf16(u8_3_seq), u16_3_seq);
        REQUIRE_EQ(utf8_to_utf16(u8_2_seq), u16_2_seq);
        REQUIRE_EQ(utf8_to_utf16(u8_1_seq), u16_1_seq);

        REQUIRE_EQ(utf8_to_utf32(u8_4_seq), u32_4);
        REQUIRE_EQ(utf8_to_utf32(u8_3_seq), u32_3);
        REQUIRE_EQ(utf8_to_utf32(u8_2_seq), u32_2);
        REQUIRE_EQ(utf8_to_utf32(u8_1_seq), u32_1);

        REQUIRE_EQ(utf16_to_utf8(u16_4_seq), u8_4_seq);
        REQUIRE_EQ(utf16_to_utf8(u16_3_seq), u8_3_seq);
        REQUIRE_EQ(utf16_to_utf8(u16_2_seq), u8_2_seq);
        REQUIRE_EQ(utf16_to_utf8(u16_1_seq), u8_1_seq);

        REQUIRE_EQ(utf16_to_utf32(u16_4_seq), u32_4);
        REQUIRE_EQ(utf16_to_utf32(u16_3_seq), u32_3);
        REQUIRE_EQ(utf16_to_utf32(u16_2_seq), u32_2);
        REQUIRE_EQ(utf16_to_utf32(u16_1_seq), u32_1);

        REQUIRE_EQ(utf32_to_utf8(u32_4), u8_4_seq);
        REQUIRE_EQ(utf32_to_utf8(u32_3), u8_3_seq);
        REQUIRE_EQ(utf32_to_utf8(u32_2), u8_2_seq);
        REQUIRE_EQ(utf32_to_utf8(u32_1), u8_1_seq);

        REQUIRE_EQ(utf32_to_utf16(u32_4), u16_4_seq);
        REQUIRE_EQ(utf32_to_utf16(u32_3), u16_3_seq);
        REQUIRE_EQ(utf32_to_utf16(u32_2), u16_2_seq);
        REQUIRE_EQ(utf32_to_utf16(u32_1), u16_1_seq);
    }
}