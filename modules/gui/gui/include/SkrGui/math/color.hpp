#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
namespace __helper
{
inline constexpr uint32_t color_parse_hex_digit(const char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f')
    {
        return 10 + c - 'a';
    }
    else if ('A' <= c && c <= 'F')
    {
        return 10 + c - 'A';
    }
    else
    {
        return 0;
    }
}

} // namespace __helper

// 线性空间色彩
struct Color {
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 1;

    // factory
    inline static constexpr Color Linear(const std::string_view str)
    {
        if (str.length() == 9)
        {
            return {
                ((__helper::color_parse_hex_digit(str[1]) << 4) + __helper::color_parse_hex_digit(str[2])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[3]) << 4) + __helper::color_parse_hex_digit(str[4])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[5]) << 4) + __helper::color_parse_hex_digit(str[6])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[7]) << 4) + __helper::color_parse_hex_digit(str[8])) / 255.0f,
            };
        }
        else if (str.length() == 7)
        {
            return {
                ((__helper::color_parse_hex_digit(str[1]) << 4) + __helper::color_parse_hex_digit(str[2])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[3]) << 4) + __helper::color_parse_hex_digit(str[4])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[5]) << 4) + __helper::color_parse_hex_digit(str[6])) / 255.0f,
                1.0f,
            };
        }
        else if (str.length() == 4)
        {
            return {
                ((__helper::color_parse_hex_digit(str[1]) << 4) + __helper::color_parse_hex_digit(str[1])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[2]) << 4) + __helper::color_parse_hex_digit(str[2])) / 255.0f,
                ((__helper::color_parse_hex_digit(str[3]) << 4) + __helper::color_parse_hex_digit(str[3])) / 255.0f,
                1.0f,
            };
        }
        else
        {
            return {};
        }
    }
    inline static Color SRGB(const std::string_view str)
    {
        return Linear(str).srgb();
    }

    // convert
    inline Color srgb() const SKR_NOEXCEPT { return linear_to_srgb(*this); }

    inline static Color srgb_to_linear(Color c) SKR_NOEXCEPT
    {
#define SRGB_TO_LINEAR(f) ((f) <= 0.04045 ? ((f) / 12.9f) : std::pow((f + 0.055f) / 1.055f, 2.4f))
        return Color{ SRGB_TO_LINEAR(c.r), SRGB_TO_LINEAR(c.g), SRGB_TO_LINEAR(c.b), c.a };
#undef SRGB_TO_LINEAR
    }
    inline static Color linear_to_srgb(Color c) SKR_NOEXCEPT
    {
#define LINEAR_TO_SRGB(f) ((f) <= 0.0031308 ? ((f)*12.92f) : (1.055f * std::pow((f), 1.0f / 2.4f) - 0.055f))
        return Color{ LINEAR_TO_SRGB(c.r), LINEAR_TO_SRGB(c.g), LINEAR_TO_SRGB(c.b), c.a };
#undef LINEAR_TO_SRGB
    }

    // compare
    inline bool operator==(const Color& rhs) const SKR_NOEXCEPT { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; }
    inline bool operator!=(const Color& rhs) const SKR_NOEXCEPT { return !(*this == rhs); }
};
} // namespace skr::gui