#pragma pack_matrix(row_major)

[[vk::binding(0, 0)]]
Texture2D input_color : register(t0, space0);
[[vk::binding(0, 1)]]
SamplerState texture_sampler : register(s0, space1);

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

void main(VSOut psIn, out float4 o_color : SV_Target0) : SV_TARGET
{
    float2 uv = psIn.uv;
    o_color = input_color.Sample(texture_sampler, uv);
}