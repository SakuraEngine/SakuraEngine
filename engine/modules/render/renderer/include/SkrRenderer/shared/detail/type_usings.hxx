#pragma once
#include <std/types/vec.hxx>
#include <std/types/matrix.hxx>
#include <std/resources/buffer.hxx>

#define sattr(...)

namespace skr
{
using ::float2;
using ::float3;
using ::float4;

using ::uint2;
using ::uint3;
using ::uint4;

using ::float2x2;
using ::float3x3;
using ::float4x4;

using ::ByteAddressBuffer;
using ::RWByteAddressBuffer;

using uint32 = uint;
using uint32_t = uint;
using uint64_t = uint64;
using AddressType = uint32_t;

template <class T, uint64_t N>
using gpu_array = Array<T, N>;

} // namespace skr::gpu