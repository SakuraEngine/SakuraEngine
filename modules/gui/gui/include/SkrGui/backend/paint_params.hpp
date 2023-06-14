#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/color.hpp"

// TODO. remove gdi
#include "SkrGui/dev/gdi/gdi.hpp"
namespace skr::gui
{
using ITexture = ::skr::gdi::IGDITexture;
using IMaterial = ::skr::gdi::IGDIMaterial;
using CustomPaintCallback = skr_gdi_custom_vertex_painter_t;
} // namespace skr::gui

namespace skr::gui
{
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

enum class EPaintType : uint8_t
{
    Color,
    Texture,
    Material,
    __Count,
};

struct BlendMode {
    EBlendFactor src_color = EBlendFactor::SrcAlpha;
    EBlendFactor dst_color = EBlendFactor::OneMinusSrcAlpha;
    EBlendFactor src_alpha = EBlendFactor::One;
    EBlendFactor dst_alpha = EBlendFactor::OneMinusSrcAlpha;
};
} // namespace skr::gui