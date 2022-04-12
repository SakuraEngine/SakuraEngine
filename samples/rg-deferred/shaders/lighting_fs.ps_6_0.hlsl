#pragma pack_matrix(row_major)

Texture2D<float4> gbuffer_color : register(t0, space0);
Texture2D<float4> gbuffer_normal : register(t1, space0);
Texture2D<float> gbuffer_depth : register(t2, space0);

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

void main(VSOut psIn, out float4 out_color : SV_TARGET0) : SV_TARGET
{
    float2 uv = psIn.uv;
    if(root_constants.bFlipUVX) uv.x = 1 - uv.x;
    if(root_constants.bFlipUVY) uv.y = 1 - uv.y;
    float4 gbufferColor = gbuffer_color.Sample(texture_sampler, uv);
    float4 gbufferNormal = gbuffer_normal.Sample(texture_sampler, uv);
    out_color = gbufferColor * 0.5 + gbufferNormal * 0.5;
    out_color *= gbuffer_depth.Sample(texture_sampler, uv);
}