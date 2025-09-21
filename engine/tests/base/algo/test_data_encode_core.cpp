#include <span>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "SkrBase/misc/data_encode.hpp"
#include "SkrTestFramework/framework.hpp"

struct BaseEncodeTestCase
{
    std::string_view data;
    std::string_view base16;
    std::string_view base32;
    std::string_view base64;

    std::string_view base32_no_padding;
    std::string_view base64_no_padding;
};

static std::span<BaseEncodeTestCase> test_cases()
{
    static BaseEncodeTestCase test_cases[] = {
        {
            .data = "",
            .base16 = "",
            .base32 = "",
            .base64 = "",
            .base32_no_padding = "",
            .base64_no_padding = "",
        },
        {
            .data = "f",
            .base16 = "66",
            .base32 = "MY======",
            .base64 = "Zg==",
            .base32_no_padding = "MY",
            .base64_no_padding = "Zg",
        },
        {
            .data = "fo",
            .base16 = "666F",
            .base32 = "MZXQ====",
            .base64 = "Zm8=",
            .base32_no_padding = "MZXQ",
            .base64_no_padding = "Zm8",
        },
        {
            .data = "foo",
            .base16 = "666F6F",
            .base32 = "MZXW6===",
            .base64 = "Zm9v",
            .base32_no_padding = "MZXW6",
            .base64_no_padding = "Zm9v",
        },
        {
            .data = "foob",
            .base16 = "666F6F62",
            .base32 = "MZXW6YQ=",
            .base64 = "Zm9vYg==",
            .base32_no_padding = "MZXW6YQ",
            .base64_no_padding = "Zm9vYg",
        },
        {
            .data = "fooba",
            .base16 = "666F6F6261",
            .base32 = "MZXW6YTB",
            .base64 = "Zm9vYmE=",
            .base32_no_padding = "MZXW6YTB",
            .base64_no_padding = "Zm9vYmE",
        },
        {
            .data = "foobar",
            .base16 = "666F6F626172",
            .base32 = "MZXW6YTBOI======",
            .base64 = "Zm9vYmFy",
            .base32_no_padding = "MZXW6YTBOI",
            .base64_no_padding = "Zm9vYmFy",
        },
        {
            .data = "Hello World!",
            .base16 = "48656C6C6F20576F726C6421",
            .base32 = "JBSWY3DPEBLW64TMMQQQ====",
            .base64 = "SGVsbG8gV29ybGQh",
            .base32_no_padding = "JBSWY3DPEBLW64TMMQQQ",
            .base64_no_padding = "SGVsbG8gV29ybGQh",
        },
        {
            .data = "a quick brown fox jumps over the lazy dog",
            .base16 = "6120717569636B2062726F776E20666F78206A756D7073206F76657220746865206C617A7920646F67",
            .base32 = "MEQHC5LJMNVSAYTSN53W4IDGN54CA2TVNVYHGIDPOZSXEIDUNBSSA3DBPJ4SAZDPM4======",
            .base64 = "YSBxdWljayBicm93biBmb3gganVtcHMgb3ZlciB0aGUgbGF6eSBkb2c=",
            .base32_no_padding = "MEQHC5LJMNVSAYTSN53W4IDGN54CA2TVNVYHGIDPOZSXEIDUNBSSA3DBPJ4SAZDPM4",
            .base64_no_padding = "YSBxdWljayBicm93biBmb3gganVtcHMgb3ZlciB0aGUgbGF6eSBkb2c",
        }
    };
    return test_cases;
}

// tool functions
namespace skr
{
const uint8_t* _encode_data(std::string_view view)
{
    return (const uint8_t*)view.data();
}
const skr_char8* _decode_str(std::string_view view)
{
    return (const skr_char8*)view.data();
}
const std::string _to_lower(std::string_view view)
{
    std::string lower{ view.begin(), view.end() };
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}
const std::string_view _to_string_view(const std::vector<char8_t>& vec)
{
    return std::string_view((const char*)vec.data()); // remove null terminator
}
const std::string_view _to_string_view(const std::vector<uint8_t>& vec)
{
    return std::string_view((const char*)vec.data());
}
} // namespace skr

TEST_CASE("base16")
{
    using namespace skr;

    SUBCASE("standard test vectors")
    {
        for (const auto& test_case : test_cases())
        {
            // test encode length
            {
                auto encode_len = base16_encode_length(test_case.data.size());
                auto expected_len = test_case.base16.size();
                REQUIRE_EQ(encode_len, expected_len);
            }

            // test decode length
            {
                auto decode_len = base16_decode_length(test_case.base16.size());
                auto expected_decode_len = test_case.data.size();
                REQUIRE_EQ(decode_len, expected_decode_len);
            }

            // test encode
            {
                std::vector<char8_t> encoded_buffer;
                std::string_view encoded_view;
                auto expected_len = test_case.base16.size();

                // test encode
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base16_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size()
                );
                encoded_view = _to_string_view(encoded_buffer);
                REQUIRE_EQ(encoded_view, test_case.base16);

                // test encode_lower
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base16_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size(),
                    true
                );
                encoded_view = _to_string_view(encoded_buffer);
                std::string excepted_lower = _to_lower(test_case.base16);
                REQUIRE_EQ(encoded_view, excepted_lower);
            }

            // test decode
            {
                std::vector<uint8_t> decoded_buffer;
                std::string_view encoded_view;
                auto expected_len = test_case.data.size();
                bool success;

                // test decode
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base16_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(test_case.base16),
                    test_case.base16.size()
                );
                REQUIRE(success);
                auto decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);

                // test decode lower
                std::string lower_input = _to_lower(test_case.base16);
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base16_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(lower_input),
                    lower_input.size()
                );
                REQUIRE(success);
                decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);
            }
        }
    }

    SUBCASE("error handling")
    {
        // 无效字符测试
        std::vector<std::string_view> invalid_inputs = {
            "G", "g", "Z", "z", " ", "!", "@", "#",
            "DEADBEEG", "deadbeeg", "12345G78", "ABCD EF"
        };

        for (const auto& invalid : invalid_inputs)
        {
            std::vector<uint8_t> output(10);
            bool success = skr::base16_decode(
                output.data(),
                output.size(),
                _decode_str(invalid.data()),
                invalid.size()
            );
            REQUIRE_FALSE(success);
        }
    }

    SUBCASE("validation functions")
    {
        // 有效字符
        REQUIRE(skr::base16_is_valid_char('0'));
        REQUIRE(skr::base16_is_valid_char('9'));
        REQUIRE(skr::base16_is_valid_char('A'));
        REQUIRE(skr::base16_is_valid_char('F'));
        REQUIRE(skr::base16_is_valid_char('a'));
        REQUIRE(skr::base16_is_valid_char('f'));

        // 无效字符
        REQUIRE_FALSE(skr::base16_is_valid_char('G'));
        REQUIRE_FALSE(skr::base16_is_valid_char('g'));
        REQUIRE_FALSE(skr::base16_is_valid_char(' '));

        // 字符串验证
        std::string valid = "DEADBEEF";
        REQUIRE(skr::base16_is_valid_string(reinterpret_cast<const char8_t*>(valid.data()), valid.size()));

        std::string invalid = "DEADBEEG";
        REQUIRE_FALSE(skr::base16_is_valid_string(reinterpret_cast<const char8_t*>(invalid.data()), invalid.size()));
    }
}

TEST_CASE("base32")
{
    using namespace skr;

    SUBCASE("standard test vectors")
    {
        for (const auto& test_case : test_cases())
        {
            // test encode length
            {
                auto encode_len = base32_encode_length(test_case.data.size());
                auto expected_len = test_case.base32.size();
                REQUIRE_EQ(encode_len, expected_len);

                auto encode_len_no_pad = base32_encode_length(test_case.data.size(), false);
                auto expected_len_no_pad = test_case.base32_no_padding.size();
                REQUIRE_EQ(encode_len_no_pad, expected_len_no_pad);
            }

            // test decode length
            {
                // conservative length should be greater than or equal to expected length
                auto decode_len = base32_decode_length(test_case.base32.size());
                auto expected_decode_len = test_case.data.size();
                REQUIRE_GE(decode_len, expected_decode_len);

                // minimal length should be less than or equal to expected length
                auto decode_len_minimal = base32_decode_length_minimal(
                    _decode_str(test_case.base32),
                    test_case.base32.size()
                );
                REQUIRE_LE(decode_len_minimal, expected_decode_len);
            }

            // test encode
            {
                std::vector<char8_t> encoded_buffer;
                std::string_view encoded_view;
                auto expected_len = test_case.base32.size();

                // test encode
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base32_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size()
                );
                encoded_view = _to_string_view(encoded_buffer);
                REQUIRE_EQ(encoded_view, test_case.base32);

                // test encode no padding
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base32_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size(),
                    false
                );
                encoded_view = _to_string_view(encoded_buffer);
                REQUIRE_EQ(encoded_view, test_case.base32_no_padding);

                // test encode lower
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base32_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size(),
                    true,
                    true
                );
                encoded_view = _to_string_view(encoded_buffer);
                std::string excepted_lower = _to_lower(test_case.base32);
                REQUIRE_EQ(encoded_view, excepted_lower);

                // test encode lower no padding
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base32_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size(),
                    false,
                    true
                );
                encoded_view = _to_string_view(encoded_buffer);
                std::string expected_lower_no_padding = _to_lower(test_case.base32_no_padding);
                REQUIRE_EQ(encoded_view, expected_lower_no_padding);
            }

            // test decode
            {
                std::vector<uint8_t> decoded_buffer;
                std::string_view encoded_view;
                auto expected_len = test_case.data.size();
                bool success;

                // test decode
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base32_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(test_case.base32),
                    test_case.base32.size()
                );
                REQUIRE(success);
                auto decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);

                // test decode no padding
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base32_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(test_case.base32_no_padding),
                    test_case.base32_no_padding.size()
                );
                REQUIRE(success);
                decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);

                // test decode lower
                std::string lower_input = _to_lower(test_case.base32);
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base32_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(lower_input),
                    lower_input.size()
                );
                REQUIRE(success);
                decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);

                // test decode lower no padding
                std::string lower_no_pad_input = _to_lower(test_case.base32_no_padding);
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base32_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(lower_no_pad_input),
                    lower_no_pad_input.size()
                );
                REQUIRE(success);
                decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);
            }
        }
    }

    SUBCASE("error handling")
    {
        // 无效字符
        std::vector<std::string_view> invalid_chars = {
            "0", "1", "8", "9", "!", "@", "#", " ", "\t", "\n"
        };

        for (const auto& invalid : invalid_chars)
        {
            std::vector<uint8_t> output(10);
            bool success = skr::base32_decode(
                output.data(),
                output.size(),
                _decode_str(invalid.data()),
                invalid.size()
            );
            REQUIRE_FALSE(success);
        }
    }

    SUBCASE("validation functions")
    {
        // 有效字符
        REQUIRE(skr::base32_is_valid_char('A'));
        REQUIRE(skr::base32_is_valid_char('Z'));
        REQUIRE(skr::base32_is_valid_char('2'));
        REQUIRE(skr::base32_is_valid_char('7'));
        REQUIRE(skr::base32_is_valid_char('='));

        // 无效字符
        REQUIRE_FALSE(skr::base32_is_valid_char('0'));
        REQUIRE_FALSE(skr::base32_is_valid_char('1'));
        REQUIRE_FALSE(skr::base32_is_valid_char('8'));
        REQUIRE_FALSE(skr::base32_is_valid_char('9'));
    }
}

TEST_CASE("base64")
{
    using namespace skr;

    SUBCASE("standard test vectors")
    {
        for (const auto& test_case : test_cases())
        {
            // test encode length
            {
                auto encode_len = base64_encode_length(test_case.data.size());
                auto expected_len = test_case.base64.size();
                REQUIRE_EQ(encode_len, expected_len);

                auto encode_len_no_pad = base64_encode_length(test_case.data.size(), false);
                auto expected_len_no_pad = test_case.base64_no_padding.size();
                REQUIRE_EQ(encode_len_no_pad, expected_len_no_pad);
            }

            // test decode length
            {
                // conservative length should be greater than or equal to expected length
                auto decode_len = base64_decode_length(test_case.base64.size());
                auto expected_decode_len = test_case.data.size();
                REQUIRE_GE(decode_len, expected_decode_len);

                // minimal length should be less than or equal to expected length
                auto decode_len_minimal = base64_decode_length_minimal(
                    _decode_str(test_case.base64),
                    test_case.base64.size()
                );
                REQUIRE_LE(decode_len_minimal, expected_decode_len);
            }

            // test encode
            {
                std::vector<char8_t> encoded_buffer;
                std::string_view encoded_view;
                auto expected_len = test_case.base64.size();

                // test encode
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base64_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size()
                );
                encoded_view = _to_string_view(encoded_buffer);
                REQUIRE_EQ(encoded_view, test_case.base64);

                // test encode no padding
                encoded_buffer.clear();
                encoded_buffer.resize(expected_len + 1, 0);
                base64_encode(
                    encoded_buffer.data(),
                    _encode_data(test_case.data),
                    test_case.data.size(),
                    false
                );
                encoded_view = _to_string_view(encoded_buffer);
                REQUIRE_EQ(encoded_view, test_case.base64_no_padding);
            }

            // test decode
            {
                std::vector<uint8_t> decoded_buffer;
                std::string_view encoded_view;
                auto expected_len = test_case.data.size();
                bool success;

                // test decode
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base64_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(test_case.base64),
                    test_case.base64.size()
                );
                REQUIRE(success);
                auto decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);

                // test decode no padding
                decoded_buffer.clear();
                decoded_buffer.resize(expected_len + 1, 0);
                success = base64_decode(
                    decoded_buffer.data(),
                    expected_len,
                    _decode_str(test_case.base64_no_padding),
                    test_case.base64_no_padding.size()
                );
                REQUIRE(success);
                decoded_view = _to_string_view(decoded_buffer);
                REQUIRE_EQ(decoded_view, test_case.data);
            }
        }
    }

    SUBCASE("error handling")
    {
        // 无效字符
        std::vector<std::string_view> invalid_chars = {
            "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", " ", "\t", "\n"
        };

        for (const auto& invalid : invalid_chars)
        {
            std::vector<uint8_t> output(10);
            bool success = skr::base64_decode(
                output.data(),
                output.size(),
                _decode_str(invalid.data()),
                invalid.size()
            );
            REQUIRE_FALSE(success);
        }
    }

    SUBCASE("validation functions")
    {
        // 有效字符
        REQUIRE(skr::base64_is_valid_char('A'));
        REQUIRE(skr::base64_is_valid_char('Z'));
        REQUIRE(skr::base64_is_valid_char('a'));
        REQUIRE(skr::base64_is_valid_char('z'));
        REQUIRE(skr::base64_is_valid_char('0'));
        REQUIRE(skr::base64_is_valid_char('9'));
        REQUIRE(skr::base64_is_valid_char('+'));
        REQUIRE(skr::base64_is_valid_char('/'));
        REQUIRE(skr::base64_is_valid_char('='));

        // 无效字符
        REQUIRE_FALSE(skr::base64_is_valid_char(' '));
        REQUIRE_FALSE(skr::base64_is_valid_char('-'));
    }
}
