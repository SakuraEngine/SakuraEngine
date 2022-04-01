[[vk::binding(0, 0)]]
Texture2D<float4> gbuffer_color : register(t0, space0);
[[vk::binding(1, 0)]]
Texture2D<float4> gbuffer_normal : register(t1, space0);

[[vk::binding(0, 1)]]
SamplerState texture_sampler : register(s0, space1);

struct RootConstants
{
    int bFlipUVX;
    int bFlipUVY;
};
[[vk::push_constant]]
ConstantBuffer<RootConstants> root_constants : register(b0);

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut psIn) : SV_TARGET
{
    float2 uv = psIn.uv;
    if(root_constants.bFlipUVX) uv.x = 1 - uv.x;
    if(root_constants.bFlipUVY) uv.y = 1 - uv.y;
    return gbuffer_color.Sample(texture_sampler, uv) 
        +
        gbuffer_normal.Sample(texture_sampler, uv).xxxx;
}