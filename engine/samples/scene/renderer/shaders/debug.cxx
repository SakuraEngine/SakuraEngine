#include <std/std.hxx>

struct [[stage_inout]] VSIn
{
    float3 position;
    float2 uv0;
    float2 uv1;
    float3 normal;
};

struct [[stage_inout]] VSOut
{
    float2 uv;
};

struct RootConstants
{
    float4x4 model;
    float4x4 view_proj; // model-view-projection matrix
    bool use_base_color_texture;
};

[[group(0)]] Texture2D color_texture;
[[group(1)]] SamplerState color_sampler;

[[push_constant]]
ConstantBuffer<RootConstants> push_constants;

[[vertex_shader("vs")]]
VSOut vs(const VSIn input, [[sv_position]] float4& position)
{
    VSOut output;
    position = float4(input.position, 1.0f) * push_constants.model * push_constants.view_proj;
    output.uv = input.uv0;
    return output;
}

[[fragment_shader("fs")]]
void fs(const VSOut input, [[sv_render_target(0)]] float4& color)
{
    float4 sampled = color_texture.Sample(color_sampler, input.uv);

    if (push_constants.use_base_color_texture)
    {
        color = sampled;
    }
    else
    {
        color = float4(float(input.uv.x), float(input.uv.y), 0.5f, 1.0f);
    }
}