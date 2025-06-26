#define WIDTH 3200
#define HEIGHT 2400

[[vk::binding(0, 0)]]
RWStructuredBuffer<float4> buf : register(u0, space0);

[[vk::binding(0, 1)]]
RaytracingAccelerationStructure AS : register(t0, space0);

float4 trace(uint2 tid, uint2 tsize)
{
    RayDesc ray;
    ray.Origin = float3((float)tid.x / (float)tsize.x, (float)tid.y / (float)tsize.y, 100.0f);
    ray.Direction = float3(0, 0, -1.0f);
    ray.TMin = 0.01f;
    ray.TMax = 9999.f;

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(AS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.Proceed();

    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        return float4(q.CommittedTriangleBarycentrics(), 1, 1);
    }
    else
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f); 
    }
}

[numthreads(32, 32, 1)]
void compute_main(uint3 tid : SV_DispatchThreadID)
{
    uint2 tsize = uint2(WIDTH, HEIGHT);
    uint row_pitch = tsize.x;
    buf[tid.x + (tid.y * row_pitch)] = trace(tid.xy, tsize);
}

// ANOTHER ENTRY POINT JUST FOR XMAKE OLD SHADER COMPILE
[numthreads(32, 32, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint2 tsize = uint2(WIDTH, HEIGHT);
    uint row_pitch = tsize.x;
    buf[tid.x + (tid.y * row_pitch)] = trace(tid.xy, tsize);
}