#pragma once
#include "../gen/gen_math.hpp"

namespace skr
{
inline namespace math
{
// #define SKR_MATH_PRECISION_DOUBLE
#ifdef SKR_MATH_PRECISION_DOUBLE
// real vector & matrix
using real    = double;
using real2   = double2;
using real3   = double3;
using real4   = double4;
using real3x3 = double3x3;
using real4x4 = double4x4;

// quat & rotator & transform
using Quat      = QuatD;
using Rotator   = RotatorD;
using Transform = TransformD;

// postion & scale
using Position = double3;
using Scale    = float3; // scale needn't be double
#else
// real vector & matrix
using real    = float;
using real2   = float2;
using real3   = float3;
using real4   = float4;
using real3x3 = float3x3;
using real4x4 = float4x4;

// quat & rotator & transform
using Quat      = QuatF;
using Rotator   = RotatorF;
using Transform = TransformF;

// postion & scale
using Position = float3;
using Scale    = float3;
#endif

} // namespace math
} // namespace skr