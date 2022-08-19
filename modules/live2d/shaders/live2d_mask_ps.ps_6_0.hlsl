#include "live2d_cb.hlsli"

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 clip_pos  : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

Texture2D color_texture : register(t0, space0);
SamplerState color_sampler : register(s0, space1);

float4 main(VSOut input) : SV_TARGET
{
    const float4 base_color = push_constants.base_color;
    float isInside = step(base_color.x, input.clip_pos.x / input.clip_pos.w)
        * step(base_color.y, input.clip_pos.y / input.clip_pos.w)
        * step(input.clip_pos.x / input.clip_pos.w, base_color.z)
        * step(input.clip_pos.y / input.clip_pos.w, base_color.w);
    return push_constants.channel_flag * color_texture.Sample(color_sampler, input.uv).a * isInside;
}