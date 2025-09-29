#include <std/std.hxx>

RWStructuredBuffer<float4> buf;

[[numthreads(32, 32, 1)]]
void compute_main([[sv_thread_id]] uint2 tid)
{
    const uint2 tsize = uint2(3200, 2400);
    const uint32 row_pitch = tsize.x;
    auto mandelbrot = [&](){
        const float x = float(tid.x) / (float)tsize.x;
        const float y = float(tid.y) / (float)tsize.y;
        const float2 uv = float2(x, y);
        float n = 0.0f;
        float2 c = float2(-0.445f, 0.0f);
        c = c + (uv - float2(0.5f, 0.5f)) * 2.34f;
        float2 z = float2(0.f, 0.f);
        const int M = 128;
        for (int i = 0; i < M; i++) {
            z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
            if (dot(z, z) > 2.0f) {
                break;
            }
            n += 1.0f;
        }
        // we use a simple cosine palette to determine color:
        // http://iquilezles.org/www/articles/palettes/palettes.htm
        const float t = float(n) / float(M);
        const float3 d = float3(0.3f, 0.3f, 0.5f);
        const float3 e = float3(-0.2f, -0.3f, -0.5f);
        const float3 f = float3(2.1f, 2.0f, 3.0f);
        const float3 g = float3(0.0f, 0.1f, 0.0f);
        return float4(d + (e * cos(((f * t) + g) * 2.f * 3.14159265358979323846f)), 1.0f);
    };
    buf.Store(tid.x + tid.y * row_pitch, mandelbrot());
}