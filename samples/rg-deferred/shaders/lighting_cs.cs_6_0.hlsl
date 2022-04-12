#pragma pack_matrix(row_major)

Texture2D gbuffer_color : register(t0, space0);
Texture2D gbuffer_normal : register(t1, space0);
Texture2D gbuffer_depth : register(t2, space0);
[[vk::binding(0, 1)]]
RWTexture2D<float4> lighting_output : register(u0, space0);

struct RootConstants
{
    float2 viewportSize;
    float2 viewportOrigin;
};
[[vk::push_constant]]
ConstantBuffer<RootConstants> root_constants : register(b0);

[numthreads(16, 16, 1)]
void main(int2 threadID : SV_DispatchThreadID)
{
    if (any(threadID.xy >= int2(root_constants.viewportSize)))
        return;

    int2 pixelPosition = threadID.xy + int2(root_constants.viewportOrigin);
    float4 gbufferColor = gbuffer_color[pixelPosition];
    float4 gbufferNormal = gbuffer_normal[pixelPosition];

    lighting_output[pixelPosition] = gbufferColor * 0.5 + gbufferNormal * 0.5;
    lighting_output[pixelPosition] *= gbuffer_depth[pixelPosition].rrrr;
}