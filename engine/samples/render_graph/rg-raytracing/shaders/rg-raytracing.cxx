#include <std/std.hxx>

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

// Colorful sphere shading based on instance ID
float4 shade_sphere_hit(RayQuery<RayQueryFlags::AcceptFirstAndEndSearch>& query) 
{
    // Get instance ID (corresponding to sphere index)
    uint instance_id = query.CommittedInstanceIndex();
    
    // Generate colors uniformly distributed in RGB space
    uint hash = instance_id * 12345u + 67890u;
    uint hash2 = hash * 54321u + 98765u;
    uint hash3 = hash2 * 13579u + 24681u;
    
    // Extract RGB components from different parts of the hash
    float r = float((hash >> 8) & 0xFF) / 255.0f;
    float g = float((hash2 >> 8) & 0xFF) / 255.0f;
    float b = float((hash3 >> 8) & 0xFF) / 255.0f;
    
    // Create pure, saturated colors like in profiler
    float3 base_color = float3(r, g, b);
    
    // Make colors more saturated and bright like profiler
    base_color = normalize(base_color) * (0.8f + 0.4f * length(base_color)); // Boost saturation
    
    // Adjust brightness - not too bright, not too dark
    base_color *= 0.8f; // Moderate brightness
    
    return float4(base_color, 1.0f);
}

// Main ray tracing function with debug info
float4 trace_scene(uint2 pixel_coord, uint2 screen_size) 
{
    // Generate camera ray for this pixel
    Ray primary_ray = generate_camera_ray(pixel_coord, screen_size);
    
    // Debug: Test if we can create ray query (this will fail if TLAS binding is broken)
    RayQuery<RayQueryFlags::AcceptFirstAndEndSearch> query;
    
    // Add debug marker in top-left corner to show TLAS binding status
    if (pixel_coord.x < 50 && pixel_coord.y < 50) {
        // Try a simple ray straight down from above origin to test TLAS binding
        Ray debug_ray = Ray(float3(0.0f, 1000.0f, 0.0f), float3(0.0f, -1.0f, 0.0f), 0.1f, 2000.0f);
        query.TraceRayInline(SceneTLAS, 0xff, debug_ray);
        query.Proceed();
        
        if (query.CommittedStatus() == HitStatus::HitTriangle) {
            // TLAS working - green corner
            return float4(0.0f, 1.0f, 0.0f, 1.0f);
        } else {
            // TLAS not working - red corner  
            return float4(1.0f, 0.0f, 0.0f, 1.0f);
        }
    }
    
    // Normal ray tracing
    query.TraceRayInline(SceneTLAS, 0xff, primary_ray);
    query.Proceed();
    
    // Check hit status
    if (query.CommittedStatus() == HitStatus::HitTriangle) {
        // Hit sphere, perform shading
        return shade_sphere_hit(query);
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