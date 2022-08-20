#include "live2d_cb.hlsli"

struct VSIn
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 clip_pos  : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

VSOut main(VSIn input)
{
    VSOut output = (VSOut)0;
    output.pos = mul(float4(input.pos, 0.f, 1.f), push_constants.projection_matrix);
    output.clip_pos = mul(float4(input.pos, 0.f, 1.f), push_constants.clip_matrix);
    output.uv.x = input.uv.x;
    output.uv.y = 1.f - input.uv.y;
    return output;
}