#include "raytracing_bindings.hxx"

// 常量定义
struct RayTracingConstants {
    static constexpr uint32 WIDTH = 3200;
    static constexpr uint32 HEIGHT = 2400;
};

// 光线追踪函数
float4 trace(uint2 tid, uint2 tsize) {
    // 创建光线描述
    float3 origin = float3(
        static_cast<float>(tid.x) / static_cast<float>(tsize.x), 
        static_cast<float>(tid.y) / static_cast<float>(tsize.y), 
        100.0f
    );
    float3 direction = float3(0.0f, 0.0f, -1.0f);
    
    Ray ray(origin, direction, 0.01f, 9999.0f);

    // 创建 RayQuery（使用接受第一个命中并结束搜索的标志）
    RayQuery<RayQueryFlags::AcceptFirstAndEndSearch> query;
    query.TraceRayInline(AS, 0xff, ray);

    // 执行光线追踪
    query.Proceed();

    // 检查交点状态
    if (query.CommittedStatus() == HitStatus::HitTriangle) {
        // 返回重心坐标作为颜色
        float2 barycentrics = query.CommittedTriangleBarycentrics();
        return float4(barycentrics.x, barycentrics.y, 1.0f, 1.0f);
    } else {
        // 未命中，返回黑色
        return float4(0.0f, 0.0f, 0.0f, 1.0f); 
    }
}

// 计算着色器入口点
[[compute_shader("compute_main")]]
[[numthreads(32, 32, 1)]]
void compute_main([[builtin("ThreadID")]] uint3 tid) 
{
    uint2 tsize = uint2(RayTracingConstants::WIDTH, RayTracingConstants::HEIGHT);
    uint32 row_pitch = tsize.x;
    
    // 计算线性索引并写入结果
    uint32 index = tid.x + (tid.y * row_pitch);
    OutputColor.Store(index, trace(tid.xy, tsize));
}