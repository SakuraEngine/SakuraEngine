#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/color.hpp"

namespace skr::gui
{
struct IImage;
struct IMaterial;

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

enum class ESwizzleChannel
{
    Disable,
    R,
    G,
    B,
    A,
    Zero,
    One,
};

struct Swizzle {
    ESwizzleChannel r = ESwizzleChannel::Disable;
    ESwizzleChannel g = ESwizzleChannel::Disable;
    ESwizzleChannel b = ESwizzleChannel::Disable;
    ESwizzleChannel a = ESwizzleChannel::Disable;
};

struct BlendMode {
    EBlendFactor src_color = EBlendFactor::One;
    EBlendFactor dst_color = EBlendFactor::OneMinusSrcAlpha;
    EBlendFactor src_alpha = EBlendFactor::One;
    EBlendFactor dst_alpha = EBlendFactor::OneMinusSrcAlpha;
};

struct PaintVertex {
    float4 position;
    float2 texcoord;
    float2 aa;
    float2 clipUV; // uv in clip-space
    float2 clipUV2;
    uint32_t     color;
};
using PaintIndex = uint16_t;
struct PaintCommand {
    IImage*    texture         = nullptr;
    IMaterial* material        = nullptr;
    size_t     index_begin     = 0;
    size_t     index_count     = 0;
    Swizzle    texture_swizzle = {};
};

} // namespace skr::gui