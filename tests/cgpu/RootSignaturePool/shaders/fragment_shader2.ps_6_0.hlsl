[[vk::binding(0, 0)]]
Texture2D<float4> sampled_texture2 : register(t0, space0);

[[vk::binding(0, 1)]]
SamplerState texture_sampler2 : register(s0, space1);

struct RootConstants
{
    float ColorMultiplier;
    float ColorMultiplier1;
    float ColorMultiplier2;
    float ColorMultiplier3;
    float ColorMultiplier4;
    float ColorMultiplier5;
    float ColorMultiplier6;
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
    return sampled_texture2.Sample(texture_sampler2, uv) * push_constants.ColorMultiplier;
}