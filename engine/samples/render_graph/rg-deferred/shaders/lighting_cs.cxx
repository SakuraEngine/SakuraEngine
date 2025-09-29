#include <std/std.hxx>



Texture2D gbuffer_color;
Texture2D gbuffer_normal;
Texture2D gbuffer_depth;
RWTexture2D lighting_output;

struct RootConstants
{
    float2 viewportSize;
    float2 viewportOrigin;
};
[[push_constant]]
ConstantBuffer<RootConstants> push_constants;

[[numthreads(16, 16, 1)]]
void cs([[sv_thread_id]] uint2 threadID)
{
    if (any(threadID.xy >= uint2(push_constants.viewportSize)))
        return;

    int2 pixelPosition = threadID.xy + uint2(push_constants.viewportOrigin);
    float4 gbufferColor = gbuffer_color.Load(pixelPosition);
    float4 gbufferNormal = gbuffer_normal.Load(pixelPosition);
    float4 depth = gbuffer_depth.Load(pixelPosition);

    float4 OutputColor = gbufferColor * 0.5f + abs(gbufferNormal) * 0.5f;
    OutputColor *= depth.rrrr;
    lighting_output.Store(pixelPosition, OutputColor);
}