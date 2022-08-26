#include "live2d_cb.hlsli"

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 clip_pos  : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

Texture2D color_texture : register(t0, space0);
Texture2D mask_texture : register(t0, space1);
SamplerState color_sampler : register(s0, space2);

float4 main(VSOut input) : SV_TARGET
{
    float4 tex_color = color_texture.Sample(color_sampler, input.uv);
    tex_color.rgb = tex_color.rgb * push_constants.multiply_color.rgb;
    const float3 _a = tex_color.rgb + push_constants.screen_color.rgb; 
    const float3 _m = tex_color.rgb * push_constants.screen_color.rgb; 
    tex_color.rgb = _a - _m;
    float4 output_color = tex_color * push_constants.base_color;
    output_color.xyz *= output_color.w;

    if (push_constants.use_mask)
    {
        float2 mask_uv = input.clip_pos.xy / input.clip_pos.w;
        
        float4 clip_mask = (1.0 - mask_texture.Sample(color_sampler, mask_uv)) * push_constants.channel_flag;
        float mask_value = clip_mask.r + clip_mask.g + clip_mask.b + clip_mask.a;
        output_color = output_color * mask_value;
    }

    return output_color;
}