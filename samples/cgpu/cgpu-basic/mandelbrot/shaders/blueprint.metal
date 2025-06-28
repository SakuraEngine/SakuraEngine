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
struct ConstantBuffer
{
  constant T* cgpu_buffer_data;
};

template <typename T, access a>
struct Buffer;

template <typename T>
struct Buffer<T, access::read>
{
  constant T* cgpu_buffer_data;
  uint64_t cgpu_buffer_size;
};

template <typename T>
struct Buffer<T, access::read_write>
{
  device T* cgpu_buffer_data;
  uint64_t cgpu_buffer_size;
};

template <typename T>
using RWBuffer = Buffer<T, access::read_write>;

template <typename T, access a>
struct Texture2D
{
  texture2d<T, a> cgpu_texture;
};

template <typename T>
void buffer_store(RWBuffer<T> b, uint64_t idx, T value) {
  b.cgpu_buffer_data[idx] = value;
}

// bind resources
struct SRT
{
  ConstantBuffer<float4> cbuf; 

  Buffer<float4> ro_buffer; 
  RWBuffer<float4> rw_buffer; 
  Texture2D<float, access::read_write> t;

  Buffer<float4> ro_buffer_array[5]; 
  RWBuffer<float4> rw_buffer_array[5];
  Texture2D<float, access::read_write> texture_array[5];

  constant Buffer<float4>* ro_buffers; 
  device RWBuffer<float4>* rw_buffers;
  constant Texture2D<float, access::read_write>* textures;
};
device SRT& constant srt [[buffer(0)]];

kernel void compute_main(uint2 tid [[thread_position_in_grid]])
{
  uint2 tsize = uint2(1024, 1024);
  uint row_pitch = tsize.x;

  buffer_store(srt.rw_buffer, tid.x + (tid.y * row_pitch), mandelbrot(tid, tsize));
  buffer_store(srt.rw_buffer_array[1], tid.x + (tid.y * row_pitch), mandelbrot(tid, tsize));
  buffer_store(srt.rw_buffers[114514], tid.x + (tid.y * row_pitch), mandelbrot(tid, tsize));
}