#include <std/std.hxx>



struct Constants
{
    float4x4 ProjectionMatrix;
};

struct [[stage_inout]] VSIn
{
    float2 pos;
    float2 uv;
    float4 color;
};

struct [[stage_inout]] VSOut
{
    float4 color;
    float2 uv;
};

ConstantBuffer<Constants> Constants;
Texture2D texture0;
Sampler sampler0;

[[vertex_shader("vs")]]
VSOut vs(VSIn input, [[sv_position]] float4& position)
{
    VSOut output;
    output.color = input.color;
    output.uv = input.uv;
    position = Constants.ProjectionMatrix * float4(input.pos, 0.0, 1.0);
    return output;
}

[[fragment_shader("fs")]]
float4 fs(VSOut input)
{
    float4 font_color = texture0.Sample(sampler0, input.uv);
    return font_color * input.color;
}