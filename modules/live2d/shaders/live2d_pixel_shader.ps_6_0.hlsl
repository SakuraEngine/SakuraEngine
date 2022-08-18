#include "live2d_cb.hlsli"

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

Texture2D color_texture : register(t0, space0);
SamplerState color_sampler : register(s0, space1);

float4 main(VSOut input) : SV_TARGET
{
    float4 tex_color = color_texture.Sample(color_sampler, input.uv);
    return tex_color;
    tex_color.rgb = tex_color.rgb * push_constants.multiply_color.rgb;
    const float3 _a = tex_color.rgb + push_constants.screen_color.rgb; 
    const float3 _m = tex_color.rgb * push_constants.screen_color.rgb; 
    tex_color.rgb = _a - _m;
    float4 output_color = tex_color * push_constants.base_color;
    output_color.xyz *= output_color.w;
    return output_color;
}