#include <std/std.hxx>

struct Constants
{
    float4x4 projection_matrix;
    float4x4 clip_matrix;
    float4 base_color;
    float4 multiply_color;
    float4 screen_color;
    float4 channel_flag;
    float use_mask;
    float pad0;
    float pad1;
    float pad2;
};

[[push_constant]]
ConstantBuffer<Constants> push_constants;

[[group(0)]] Texture2D color_texture;
[[group(0)]] Texture2D mask_texture;
[[group(1)]] SamplerState color_sampler;

struct [[stage_inout]] VSIn
{
    float2 pos;
    float2 uv;
};

struct [[stage_inout]] VSOut
{
    float4 clip_pos;
    float2 uv;
};

[[vertex_shader("vertex_shader")]]
VSOut vertex(VSIn input, [[sv_position]] float4& sv_pos)
{
    VSOut output;
    sv_pos = float4(input.pos, 0.f, 1.f) * push_constants.projection_matrix;
    output.clip_pos = float4(input.pos, 0.f, 1.f) * push_constants.clip_matrix;
    output.uv.x = input.uv.x;
    output.uv.y = 1.f - input.uv.y;
    return output;
}

[[fragment_shader("mask_fs")]]
float4 mask_fs(VSOut input)
{
    const float4 base_color = push_constants.base_color;
    float isInside = step(base_color.x, input.clip_pos.x / input.clip_pos.w)
        * step(base_color.y, input.clip_pos.y / input.clip_pos.w)
        * step(input.clip_pos.x / input.clip_pos.w, base_color.z)
        * step(input.clip_pos.y / input.clip_pos.w, base_color.w);

    return push_constants.channel_flag * color_texture.Sample(color_sampler, input.uv).a * isInside;
}

[[fragment_shader("model_fs")]]
void model_fs(VSOut input, [[sv_render_target(0)]] float4& output_color)
{
    float4 tex_color = color_texture.Sample(color_sampler, input.uv);
    tex_color.rgb = tex_color.rgb * push_constants.multiply_color.rgb;
    const float3 _a = tex_color.rgb + push_constants.screen_color.rgb; 
    const float3 _m = tex_color.rgb * push_constants.screen_color.rgb; 
    tex_color.rgb = _a - _m;
    output_color = tex_color * push_constants.base_color;
    output_color.xyz *= output_color.w;

    if (push_constants.use_mask)
    {
        float2 mask_uv = input.clip_pos.xy / input.clip_pos.w;
        
        float4 clip_mask = (float4(1.f, 1.f, 1.f, 1.f) - mask_texture.Sample(color_sampler, mask_uv)) * push_constants.channel_flag;
        float mask_value = clip_mask.r + clip_mask.g + clip_mask.b + clip_mask.a;
        output_color = output_color * mask_value;
    }
}