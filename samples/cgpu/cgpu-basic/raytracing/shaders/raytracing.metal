#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template <typename T, access a>
struct Buffer;

template <typename T>
struct Buffer<T, access::read>
{
  constant T* cgpu_buffer_data;
  uint32_t cgpu_buffer_size;
};

template <typename T>
struct Buffer<T, access::read_write>
{
  void Store(uint32_t index, T v) {
    cgpu_buffer_data[index] = v;
  }

  device T* cgpu_buffer_data;
  uint32_t cgpu_buffer_size;
};

template <typename T>
using RWBuffer = Buffer<T, access::read_write>;

struct AccelerationStructure
{
  metal::raytracing::instance_acceleration_structure as;
};

struct SRT
{
  RWBuffer<float4> buf;
  AccelerationStructure AS;
};
constant SRT& constant srt [[buffer(0)]];

float4 trace(uint2 tid, uint2 tsize){
  using namespace metal::raytracing;

  ray r;
  r.origin = float3((float)tid.x / (float)tsize.x, (float)tid.y / (float)tsize.y, 100.0f);
  r.direction = float3(0, 0, -100.0f);

  intersector<triangle_data, instancing> intersector;
  intersection_result<triangle_data, instancing> intersection;

  intersection = intersector.intersect(r, srt.AS.as);

  if (intersection.type == intersection_type::triangle) {
      return float4(intersection.triangle_barycentric_coord, 1, 1);
  } else {
      return float4(0);
  }
}

kernel void compute_main(
    uint2 tid [[thread_position_in_grid]],
    constant SRT& _srt [[buffer(0)]]
)
{
  uint2 tsize = uint2(3200, 2400);
  uint row_pitch = tsize.x;
  ((SRT)srt).buf.Store(tid.x + (tid.y * row_pitch), trace(tid, tsize));
}