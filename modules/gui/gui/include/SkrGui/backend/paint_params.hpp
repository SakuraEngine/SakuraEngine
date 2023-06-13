#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/color.hpp"

namespace skr::gui
{
struct ITexture;
struct IMaterial;

enum class EPaintStyle : uint8_t
{
    Fill,
    Stroke,
    __Count,
};

enum class EStrokeCap : uint8_t
{
    Butt,
    Round,
    Square,
    __Count,
};

enum class EStrokeJoin : uint8_t
{
    Miter,
    Round,
    Bevel,
    __Count,
};

enum class EBlendFactor : uint8_t
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    SrcAlphaSaturate,
};

struct BlendMode {
    EBlendFactor src_color = EBlendFactor::SrcAlpha;
    EBlendFactor dst_color = EBlendFactor::OneMinusSrcAlpha;
    EBlendFactor src_alpha = EBlendFactor::One;
    EBlendFactor dst_alpha = EBlendFactor::OneMinusSrcAlpha;
};

struct PaintParams {
    //==> draw state
    Color       color;
    EPaintStyle paint_style = EPaintStyle::Fill;
    EStrokeCap  stroke_cap = EStrokeCap::Butt;
    EStrokeJoin stroke_join = EStrokeJoin::Miter;
    bool        anti_alias = true;
    float       stroke_width = 0.0f;
    float       stroke_miter_limit = 4.0f;

    //==> resource TODO. use Surface class
    ITexture*    texture = nullptr;
    IMaterial*   material = nullptr;
    EBlendFactor blend = {};
    // - mask filter (impl by material)
    // - filter quality (impl by material pls)
    // - color filter (material pls sir)
    // - image filter (material pls sir)
    // - invert colors (material pls sir)
    // - _dither (material pls sir)
};
} // namespace skr::gui