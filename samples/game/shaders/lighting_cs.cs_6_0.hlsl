#pragma pack_matrix(row_major)

Texture2D gbuffer_color;
Texture2D gbuffer_normal;
Texture2D gbuffer_depth;
RWTexture2D<float4> lighting_output;

struct RootConstants
{
    float2 viewportSize;
    float2 viewportOrigin;
};
[[vk::push_constant]]
ConstantBuffer<RootConstants> push_constants : register(b0);

[numthreads(16, 16, 1)]
void main(int2 threadID : SV_DispatchThreadID)
{
    if (any(threadID.xy >= int2(push_constants.viewportSize)))
        return;

    int2 pixelPosition = threadID.xy + int2(push_constants.viewportOrigin);
    float4 gbufferColor = gbuffer_color[pixelPosition];
    float4 gbufferNormal = gbuffer_normal[pixelPosition];

    lighting_output[pixelPosition] = gbufferColor * 0.5 + gbufferNormal * 0.5;
    lighting_output[pixelPosition] *= gbuffer_depth[pixelPosition].rrrr;
}