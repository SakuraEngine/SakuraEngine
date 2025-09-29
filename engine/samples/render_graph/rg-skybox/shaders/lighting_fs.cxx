#include <std/std.hxx>

struct [[stage_inout]] VSOut
{
    float2 uv;
};

struct RootConstants
{
    int bFlipUVX;
    int bFlipUVY;
};

[[push_constant]]
ConstantBuffer<RootConstants> push_constants;

[[group(0)]] Texture2D gbuffer_color;
[[group(0)]] Texture2D gbuffer_normal;
[[group(0)]] Texture2D gbuffer_depth;
[[group(1)]] SamplerState texture_sampler;

[[fragment_shader("fs")]]
void fs(VSOut psIn, [[sv_render_target(0)]] float4 OutputColor)
{
    float2 uv = psIn.uv;
    if (push_constants.bFlipUVX)
        uv.x = 1 - uv.x;
    if (push_constants.bFlipUVY)
        uv.y = 1 - uv.y;

    float4 gbufferColor = gbuffer_color.Sample(texture_sampler, uv);
    float4 gbufferNormal = gbuffer_normal.Sample(texture_sampler, uv);
    // OutputColor = gbufferColor * 0.5f + abs(gbufferNormal) * 0.5f;
    // OutputColor *= gbuffer_depth.Sample(texture_sampler, uv);
    OutputColor = gbufferColor;
}