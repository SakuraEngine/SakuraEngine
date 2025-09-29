#include "std/std.hpp"
#include "std/functions/math.hpp"



enum class E : uint {
    A, B, C, D, E, F, G
};

struct Method
{
    float dos(float c)
    {
        b = c;
        return b + c;
    }
    float b = 2.f;
};

float4 mandelbrot(uint2 tid, uint2 tsize) {
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
    return float4(d + (e * cos(((f * t) + g) * 2.f * pi)), 1.0f);
}

Buffer<float4> buf;

[[kernel_2d(16, 16)]]
void compute_main([[builtin("ThreadID")]] uint2 tid)
{
    const uint2 tsize = uint2(3200, 2400);
    const uint32 row_pitch = tsize.x;
    buf.store(tid.x + tid.y * row_pitch, mandelbrot(tid, tsize));
}