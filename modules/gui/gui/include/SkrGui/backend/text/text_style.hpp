#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/color.hpp"

namespace skr::gui
{
struct FontWeight {
    int32_t value = 400;

    /// Thin, the least thick
    inline static constexpr FontWeight Thin(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 100 + bias }; }

    /// Extra-light
    inline static constexpr FontWeight ExtraLight(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 200 + bias }; }

    /// Light
    inline static constexpr FontWeight Light(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 300 + bias }; }

    /// Normal / regular / plain
    inline static constexpr FontWeight Normal(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 400 + bias }; }

    /// Medium
    inline static constexpr FontWeight Medium(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 500 + bias }; }

    /// Semi-bold
    inline static constexpr FontWeight SemiBold(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 600 + bias }; }

    /// Bold
    inline static constexpr FontWeight Bold(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 700 + bias }; }

    /// Extra-bold
    inline static constexpr FontWeight ExtraBold(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 800 + bias }; }

    /// Black, the most thick
    inline static constexpr FontWeight Black(int32_t bias = 0) SKR_NOEXCEPT { return FontWeight{ 900 + bias }; }
};

enum class EFontStyle
{
    Normal,
    Italic,
};

enum class ETextBaseline
{
    /// The horizontal line used to align the bottom of glyphs for alphabetic characters.
    Alphabetic,

    /// The horizontal line used to align ideographic characters.
    Ideographic,
};

struct TextStyle {
    Color color = {};
    // Color background_color = {};
    // String font_family = {};
    // Array<String> font_family_fall_back
    float font_size = 14.0f;
    // FontWeight    font_weight = FontWeight::Normal();
    // EFontStyle    font_style = EFontStyle::Normal;
    // float         letter_spacing_offset = 0.0f;
    // float         word_spacing_offset = 0.0f;
    // ETextBaseline text_baseline = ETextBaseline::Alphabetic;
    // float         height = 1.0f;
};
} // namespace skr::gui