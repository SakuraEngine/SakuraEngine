[[vk::binding(0, 0)]]
Texture2D<float4> sampled_texture : register(t0, space0);

[[vk::binding(0, 1)]]
SamplerState texture_sampler : register(s0, space1);

struct RootConstants
{
    float ColorMultiplier;
    int bFlipUVX;
    int bFlipUVY;
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> root_constants : register(b0);

struct VSOut
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut psIn) : SV_TARGET
{
    float2 uv = psIn.uv;
    if(root_constants.bFlipUVX) uv.x = 1 - uv.x;
    if(root_constants.bFlipUVY) uv.y = 1 - uv.y;
    return sampled_texture.Sample(texture_sampler, uv) 
        * root_constants.ColorMultiplier
        * float4(psIn.color, 1.f);
}