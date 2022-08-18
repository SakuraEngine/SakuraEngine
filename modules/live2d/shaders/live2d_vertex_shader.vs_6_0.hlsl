#include "live2d_cb.hlsli"

struct VSIn
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

VSOut main(VSIn input)
{
    VSOut output = (VSOut)0;
    output.pos = mul(float4(input.pos, 0.f, 1.f), push_constants.projection_matrix);
    output.uv.x = input.uv.x;
    output.uv.y = 1.f - input.uv.y;
    return output;
}