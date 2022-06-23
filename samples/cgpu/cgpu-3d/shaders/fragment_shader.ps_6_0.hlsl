// vk:      0|  tex  |sampler|
// d3d12:   0|  tex  |
//          1|sampler|
[[vk::binding(0, 0)]]
Texture2D<float4> sampled_texture : register(t0, space0);
[[vk::binding(1, 0)]]
SamplerState texture_sampler : register(s0, space1);

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut psIn) : SV_TARGET
{
    float2 uv = psIn.uv;
    return sampled_texture.Sample(texture_sampler, uv);
}