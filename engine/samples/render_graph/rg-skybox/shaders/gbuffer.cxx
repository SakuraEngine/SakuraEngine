#include <std/std.hxx>

struct [[stage_inout]] VSIn
{
    float3 position;
    float2 uv;
    [[centroid]] float4 normal;
    [[centroid]] float4 tangent;
    [[nointerpolation]] float4 model0;
    [[nointerpolation]] float4 model1;
    [[nointerpolation]] float4 model2;
    [[nointerpolation]] float4 model3;
};

struct [[stage_inout]] VSOut
{
    float2 uv;
    [[centroid]] float4 normal;
    [[centroid]] float4 tangent;
};

struct RootConstants
{
    float4x4 view_proj;
};

TextureCube<> skybox;
[[group(1)]] SamplerState sky_sampler;

[[push_constant]]
ConstantBuffer<RootConstants> push_constants;

[[vertex_shader("vs")]]
VSOut vertex(const VSIn input, [[sv_position]] float4& position)
{
    VSOut output;
    float4 posW = float4(input.position, 1.0f) * float4x4(input.model0, input.model1, input.model2, input.model3);
    float4 posH = posW * push_constants.view_proj;
    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;
    position = posH;
    return output;
}

[[fragment_shader("fs")]]
void fragment(const VSOut input, [[sv_render_target(0)]] float4& color, [[sv_render_target(1)]] float4& normal)
{
    float2 uv = input.uv;
    // color = float4(1.f, 1.f, 1.f, 1.f);
    // color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    color = skybox.Sample(sky_sampler, input.normal.xyz);
    normal = input.normal;
}