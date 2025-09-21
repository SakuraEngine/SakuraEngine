#include <std/std.hxx>
#include "SkrRenderer/shared/gpu_scene.hpp"

ByteAddressBuffer GPUSceneInstances;
ByteAddressBuffer MaterialTable;
ByteAddressBuffer PrimitiveTable;

RWTexture2D output_texture;
RaytracingAccelerationStructure SceneTLAS;

// Push constants - camera parameters
struct CameraConstants 
{
    float4 cameraPos;
    float4 cameraDir;
    float2 screenSize;
};

[[push_constant]]
ConstantBuffer<CameraConstants> camera_constants;

TexelBuffer<uint> IndexBuffers[0];
ByteAddressBuffer VertexBuffers[0];
Texture2D<> MaterialTextures[0];

[[group(1)]] SamplerState tex_sampler;

// Ray tracing constants
struct RayTracingConstants {
    static constexpr uint32 MAX_BOUNCES = 1;
    static constexpr float EPSILON = 0.001f;
    static constexpr float MAX_DISTANCE = 100000.0f;
};

// Generate camera ray from screen coordinates
Ray generate_camera_ray(uint2 pixel_coord, uint2 screen_size) 
{
    // Screen coordinates to normalized device coordinates [-1, 1]
    float2 ndc = float2(
        (float(pixel_coord.x) + 0.5f) / float(screen_size.x) * 2.0f - 1.0f,
        1.0f - (float(pixel_coord.y) + 0.5f) / float(screen_size.y) * 2.0f
    );
    
    // Simplified approach: directly compute ray direction from camera
    float3 ray_origin = camera_constants.cameraPos.xyz;
    
    // Camera setup: eye=(0,500,-200), target=(0,0,-800), up=(0,1,0)
    float3 camera_forward = normalize(camera_constants.cameraDir.xyz);
    float3 camera_right = normalize(cross(camera_forward, float3(0, 1, 0)));
    float3 camera_up = cross(camera_right, camera_forward);
    
    // Simple perspective projection
    float fov_scale = tan(45.0f * 3.14159f / 180.0f / 2.0f); // 45 degree FOV
    float aspect = float(screen_size.x) / float(screen_size.y);
    
    float3 ray_direction = normalize(
        camera_forward + 
        camera_right * ndc.x * fov_scale * aspect + 
        camera_up * ndc.y * fov_scale
    );
    
    return Ray(ray_origin, ray_direction, RayTracingConstants::EPSILON, RayTracingConstants::MAX_DISTANCE);
}

// Main ray tracing function with debug info
float4 trace_scene(uint2 pixel_coord, uint2 screen_size) 
{
    // Generate camera ray for this pixel
    Ray primary_ray = generate_camera_ray(pixel_coord, screen_size);
    
    // Debug: Test if we can create ray query (this will fail if TLAS binding is broken)
    RayQuery<RayQueryFlags::None> query;
    
    // Normal ray tracing
    query.TraceRayInline(SceneTLAS, 0xff, primary_ray);
    while (query.Proceed())
    {
        if (query.CandidateStatus() == HitStatus::HitTriangle)
            query.CommitTriangle();        
    }
    
    // Check hit status
    if (query.CommittedStatus() == HitStatus::HitTriangle) {
        uint instance_id = query.CommittedInstanceIndex();
        
        // 添加实例索引边界检查
        if (instance_id == ~0u) {
            return float4(0.f, 0.f, 1.f, 1.f); // 返回蓝色表示无效实例
        }
        
        const auto instance_row = skr::gpu::Row<skr::gpu::Instance>(instance_id); 
        const auto instance = instance_row.Load(GPUSceneInstances);
        float4 color = float4(1.f, 1.f, 1.f, 1.f);
        
        uint geometry_index = query.CommittedGeometryIndex();
        // 添加几何索引边界检查
        if (geometry_index >= (instance.primitives.Start() + instance.primitives.Count())) {
            return float4(1.f, 1.f, 0.f, 1.f); // 返回黄色表示几何索引错误
        }
        
        const auto prim = instance.primitives.Load(PrimitiveTable, geometry_index);
        const auto mat = prim.material.Load(MaterialTable);
        const auto prim_idx = query.CommittedPrimitiveIndex();
        
        // 添加边界检查以防止缓冲区越界访问
        if (prim_idx >= (prim.indices.Start() + prim.indices.Count())) {
            return float4(1.f, 0.f, 0.f, 1.f); // 返回红色表示错误
        }
        
        const uint3 tri = uint3(
            prim.indices.Load(IndexBuffers, 3 * prim_idx),
            prim.indices.Load(IndexBuffers, 3 * prim_idx + 1),
            prim.indices.Load(IndexBuffers, 3 * prim_idx + 2)
        );
        
        // 检查顶点索引是否在有效范围内
        if (tri[0] >= (prim.positions.Start() + prim.positions.Count()) || 
            tri[1] >= (prim.positions.Count() + prim.positions.Count()) || 
            tri[2] >= (prim.positions.Count() + prim.positions.Count())) 
        {
            return float4(0.f, 1.f, 0.f, 1.f); // 返回绿色表示顶点索引错误
        }
        
        const auto uv_a = prim.uvs.Load(VertexBuffers, tri[0]);
        const auto uv_b = prim.uvs.Load(VertexBuffers, tri[1]);
        const auto uv_c = prim.uvs.Load(VertexBuffers, tri[2]);
        const auto uv = interpolate(query.CommittedTriangleBarycentrics(), uv_a, uv_b, uv_c);
        const auto pos_a = prim.positions.Load(VertexBuffers, tri[0]);
        if (mat.basecolor_tex != ~0)
            color = MaterialTextures[mat.basecolor_tex].Sample(tex_sampler, uv);
        else
            color = float4(1.f, 0.f, 1.f, 1.f);
        return color;
    } else {
        // Miss, return square checkerboard
        float2 uv = float2(pixel_coord) / float2(screen_size);
        float checker_size = 24.0f;
        float aspect_ratio = float(screen_size.x) / float(screen_size.y);
        float2 checker_uv = float2(uv.x * aspect_ratio, uv.y) * checker_size;
        uint2 checker_coord = uint2(checker_uv);
        bool is_even = ((checker_coord.x + checker_coord.y) % 2) == 0;
        float3 bg_color = is_even ? float3(0.18f, 0.18f, 0.18f) : float3(0.15f, 0.15f, 0.15f);
        return float4(bg_color, 1.0f);
    }
}

// Compute shader entry point
[[compute_shader("cs_main")]]
[[numthreads(16, 16, 1)]]
void compute_main([[sv_thread_id]] uint3 thread_id) 
{
    uint2 screen_size = uint2(camera_constants.screenSize);
    uint2 pixel_coord = thread_id.xy;
    
    // Boundary check
    if (any(pixel_coord >= screen_size)) {
        return;
    }
    
    // Execute ray tracing and write directly to output texture
    float4 pixel_color = trace_scene(pixel_coord, screen_size);
    output_texture.Store(pixel_coord, pixel_color);
}