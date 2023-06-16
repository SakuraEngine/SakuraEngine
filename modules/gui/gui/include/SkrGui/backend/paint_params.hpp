#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/color.hpp"

// TODO. remove gdi
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gui
{
using ITexture = IGDITexture;
using IMaterial = IGDIMaterial;
using CustomPaintCallback = CustomVertexPainter;

enum class EPaintType : uint8_t
{
    Color,
    Texture,
    Material,
    __Count,
};

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
    __Count,
};

enum class ESwizzleChannel
{
    R,
    G,
    B,
    A,
    Zero,
    One,
    __Count,
};

struct Swizzle {
    ESwizzleChannel r = ESwizzleChannel::R;
    ESwizzleChannel g = ESwizzleChannel::G;
    ESwizzleChannel b = ESwizzleChannel::B;
    ESwizzleChannel a = ESwizzleChannel::A;
};

struct BlendMode {
    EBlendFactor src_color = EBlendFactor::One;
    EBlendFactor dst_color = EBlendFactor::OneMinusSrcAlpha;
    EBlendFactor src_alpha = EBlendFactor::One;
    EBlendFactor dst_alpha = EBlendFactor::OneMinusSrcAlpha;
};
} // namespace skr::gui