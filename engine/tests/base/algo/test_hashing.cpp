#include "SkrBase/types.h"
#include <string>
#include <iostream>
#include "SkrTestFramework/framework.hpp"

// helpers
inline bool _str_equal(const char8_t* a, const char8_t* b)
{
    return std::char_traits<char8_t>::compare(a, b, std::char_traits<char8_t>::length(a)) == 0;
}

//! test data from https://the-x.cn/hash/MessageDigestAlgorithm.aspx
// raw data
std::u8string_view test_data =
    u8R"__(寻寻觅觅，冷冷清清，凄凄惨惨戚戚。乍暖还寒时候，最难将息。三杯两盏淡酒，怎敌他、晚来风急？雁过也，正伤心，却是旧时相识。
满地黄花堆积。憔悴损，如今有谁堪摘？守著窗儿，独自怎生得黑？梧桐更兼细雨，到黄昏、点点滴滴。这次第，怎一个愁字了得！)__";

// md5 datas
std::u8string_view test_md5_hex = u8"196e990172bcca33f42244d1810c05ba";
std::u8string_view test_md5_base64 = u8"GW6ZAXK8yjP0IkTRgQwFug";
std::u8string_view test_md5_base64_padded = u8"GW6ZAXK8yjP0IkTRgQwFug==";

// sha256 datas
std::u8string_view test_sha256_hex = u8"19e15ed96b629a9e3678759437087da42219ae73d8063d0b3f239f7d5aac359c";
std::u8string_view test_sha256_base64 = u8"GeFe2Wtimp42eHWUNwh9pCIZrnPYBj0LPyOffVqsNZw";
std::u8string_view test_sha256_base64_padded = u8"GeFe2Wtimp42eHWUNwh9pCIZrnPYBj0LPyOffVqsNZw=";

// sha512 datas
std::u8string_view test_sha512_hex = u8"832f693cbff5444510031cb380166c042ef892a70b6490d79d3c923d1c211b521001e93b6d4d5c152f95e0338c141a000b46cc663d3ac1952bc1745208965d37";
std::u8string_view test_sha512_base64 = u8"gy9pPL/1REUQAxyzgBZsBC74kqcLZJDXnTySPRwhG1IQAek7bU1cFS+V4DOMFBoAC0bMZj06wZUrwXRSCJZdNw";
std::u8string_view test_sha512_base64_padded = u8"gy9pPL/1REUQAxyzgBZsBC74kqcLZJDXnTySPRwhG1IQAek7bU1cFS+V4DOMFBoAC0bMZj06wZUrwXRSCJZdNw==";

TEST_CASE("test md5")
{
    using namespace skr;

    SUBCASE("test crypt decode")
    {
        MD5 md5_hex = *MD5::Decode(test_md5_hex.data(), MD5::EStyle::Hex);
        MD5 md5_b64 = *MD5::Decode(test_md5_base64.data(), MD5::EStyle::Base64);
        MD5 md5_b64p = *MD5::Decode(test_md5_base64_padded.data(), MD5::EStyle::Base64Padded);
        REQUIRE_EQ(md5_hex, md5_b64);
        REQUIRE_EQ(md5_hex, md5_b64p);
        REQUIRE_EQ(md5_b64, md5_b64p);
    }

    SUBCASE("test build")
    {
        MD5 md5_parse = *MD5::DecodeAuto(test_md5_hex.data());

        // basic build
        {
            MD5 md5_build = MD5::Build((const char8_t*)test_data.data(), (uint32_t)test_data.size());
            REQUIRE_EQ(md5_build, md5_parse);
        }

        // segmented build
        {
            constexpr uint64_t kSegmentSize = 8;

            MD5Builder builder;
            builder.init();
            for (uint64_t i = 0; i < test_data.size(); i += kSegmentSize)
            {
                uint64_t segment_size = skr::min(kSegmentSize, test_data.size() - i);
                builder.update(test_data.data() + i, (uint32_t)segment_size);
            }
            MD5 md5_build = builder.finalize();

            REQUIRE_EQ(md5_build, md5_parse);
        }
    }

#define ENCODE_AND_CHECK(__STR, __LEN, ...) \
    buffer.clear();                         \
    buffer.resize(__LEN + 1, 0);            \
    __VA_ARGS__;                            \
    REQUIRE(_str_equal(buffer.data(), __STR));
    SUBCASE("test encode")
    {
        MD5 md5_build = MD5::Build((const char8_t*)test_data.data(), (uint32_t)test_data.size());
        std::vector<skr_char8> buffer;

        ENCODE_AND_CHECK(test_md5_hex.data(), MD5::kHexLength, md5_build.encode_hex(buffer.data()));
        ENCODE_AND_CHECK(test_md5_base64.data(), MD5::kBase64Length, md5_build.encode_base64(buffer.data(), false));
        ENCODE_AND_CHECK(test_md5_base64_padded.data(), MD5::kBase64PaddedLength, md5_build.encode_base64(buffer.data(), true));
    }
#undef ENCODE_AND_CHECK
}

TEST_CASE("test sha256")
{
    using namespace skr;
    SUBCASE("test crypt decode")
    {
        SHA256 sha256_hex = *SHA256::Decode(test_sha256_hex.data(), SHA256::EStyle::Hex);
        SHA256 sha256_b64 = *SHA256::Decode(test_sha256_base64.data(), SHA256::EStyle::Base64);
        SHA256 sha256_b64p = *SHA256::Decode(test_sha256_base64_padded.data(), SHA256::EStyle::Base64Padded);
        REQUIRE_EQ(sha256_hex, sha256_b64);
        REQUIRE_EQ(sha256_hex, sha256_b64p);
        REQUIRE_EQ(sha256_b64, sha256_b64p);
    }

    SUBCASE("test build")
    {
        SHA256 sha256_parse = *SHA256::DecodeAuto(test_sha256_hex.data());

        // basic build
        {
            SHA256 sha256_build = SHA256::Build((const char8_t*)test_data.data(), (uint32_t)test_data.size());
            REQUIRE_EQ(sha256_build, sha256_parse);
        }

        // segmented build
        {
            constexpr uint64_t kSegmentSize = 8;

            SHA256Builder builder;
            builder.init();
            for (uint64_t i = 0; i < test_data.size(); i += kSegmentSize)
            {
                uint64_t segment_size = skr::min(kSegmentSize, test_data.size() - i);
                builder.update(test_data.data() + i, (uint32_t)segment_size);
            }
            SHA256 sha256_build = builder.finalize();

            REQUIRE_EQ(sha256_build, sha256_parse);
        }
    }

#define ENCODE_AND_CHECK(__STR, __LEN, ...) \
    buffer.clear();                         \
    buffer.resize(__LEN + 1, 0);            \
    __VA_ARGS__;                            \
    REQUIRE(_str_equal(buffer.data(), __STR));
    SUBCASE("test encode")
    {
        SHA256 sha256_build = SHA256::Build((const char8_t*)test_data.data(), (uint32_t)test_data.size());
        std::vector<skr_char8> buffer;

        ENCODE_AND_CHECK(test_sha256_hex.data(), SHA256::kHexLength, sha256_build.encode_hex(buffer.data()));
        ENCODE_AND_CHECK(test_sha256_base64.data(), SHA256::kBase64Length, sha256_build.encode_base64(buffer.data(), false));
        ENCODE_AND_CHECK(test_sha256_base64_padded.data(), SHA256::kBase64PaddedLength, sha256_build.encode_base64(buffer.data(), true));
    }
#undef ENCODE_AND_CHECK
}
