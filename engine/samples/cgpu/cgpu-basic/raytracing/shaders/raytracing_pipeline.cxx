#include "raytracing_bindings.hxx"

// 常量定义
struct RayTracingConstants
{
    static constexpr uint32 WIDTH = 3200;
    static constexpr uint32 HEIGHT = 2400;
};

struct PathPayload
{
    float4 color;
};

struct BuiltInTriangleIntersectionAttributes
{
    float2 barycentrics;
};

[[raygen_shader("raygen")]]
void RayGen()
{
    uint3 tid = RayPipeline::DispatchRaysIndex();
    uint2 tsize = uint2(RayTracingConstants::WIDTH, RayTracingConstants::HEIGHT);
    uint32 row_pitch = tsize.x;

    float3 origin = float3(
        static_cast<float>(tid.x) / static_cast<float>(tsize.x), 
        static_cast<float>(tid.y) / static_cast<float>(tsize.y), 
        100.0f
    );
    float3 direction = float3(0.0f, 0.0f, -1.0f);
    
    Ray ray(origin, direction, 0.01f, 9999.0f);
    PathPayload payload;
    RayPipeline::TraceRay(AS, RayQueryFlags::None, 0xff, 0, 0, 0, ray, payload);

    uint32 index = tid.x + (tid.y * row_pitch);
    OutputColor.Store(index, payload.color);
}

[[closesthit_shader("closesthit")]]
void ClosestHit([[sv_payload]] PathPayload& payload, const BuiltInTriangleIntersectionAttributes& attrib)
{
    payload.color = float4(attrib.barycentrics, 1.f, 1.f);
}

[[miss_shader("miss")]]
void Miss([[sv_payload]] PathPayload& payload)
{
    payload.color = float4(0.f, 0.f, 0.f, 1.f);
}

[[anyhit_shader("anyhit")]]
void AnyHit([[sv_payload]] PathPayload& payload, const BuiltInTriangleIntersectionAttributes& attrib)
{
    
}