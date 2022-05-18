[[vk::binding(0, 0)]]
SamplerState texture_sampler : register(s0, space1);
[[vk::binding(0, 1)]]
Texture2D<float4> sampled_texture : register(t1, space0);

struct RootConstants
{
    float ColorMultiplier;
    int bFlipUVX;
    int bFlipUVY;
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> push_constants : register(b0);

struct VSOut
{
     float4 position : SV_POSITION;
     float2 uv : TEXCOORD0;
};

float4 main(VSOut psIn) : SV_TARGET
{
    float2 uv = psIn.uv;
    if(push_constants.bFlipUVX) uv.x = 1 - uv.x;
    if(push_constants.bFlipUVY) uv.y = 1 - uv.y;
    return sampled_texture.Sample(texture_sampler, uv) * push_constants.ColorMultiplier;
}