#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

float4 mandelbrot(uint2 tid, uint2 tsize){
  float x = ((float)tid.x) / ((float)tsize.x);
  float y = ((float)tid.y) / ((float)tsize.y);
  float2 uv = float2(x, y);
  float n = 0.000000;
  float2 c = float2(-0.445000, 0.000000);
  c = (c + ((uv - float2(0.500000, 0.500000)) * 2.340000));
  float2 z = { 0.000000, 0.000000 };
  int M = 128;
  for (int i = 0; i < (+M); i += 1) {
    z = (float2((z.x * z.x) - (z.y * z.y), (2.000000 * z.x) * z.y) + c);
    if (dot(z, z) > 2.000000) {
        break;
      };
    n += 1.000000;
  };
  float t = n / ((float)M);
  float3 d = float3(0.300000, 0.300000, 0.500000);
  float3 e = float3(-0.200000, -0.300000, -0.500000);
  float3 f = float3(2.100000, 2.000000, 3.000000);
  float3 g = float3(0.000000, 0.100000, 0.000000);
  return float4(d + (e * cos((((f * t) + g) * 2.000000) * 3.141593)), 1.000000);
}

template <typename T>
struct RWStructuredBuffer
{
    device T* data;
    size_t size;
};

kernel void compute_main(
    uint2 tid [[thread_position_in_grid]],
    device RWStructuredBuffer<float4>& output [[buffer(0)]]
)
{
  uint2 tsize = uint2(1024, 1024);
  uint row_pitch = tsize.x;
  output.data[tid.x + (tid.y * row_pitch)] = mandelbrot(tid, tsize);
}