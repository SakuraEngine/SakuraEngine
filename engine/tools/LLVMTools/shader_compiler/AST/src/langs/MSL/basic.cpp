namespace skr::CppSL::MSL
{
const wchar_t* kMSLHeader = LR"__de___l___im__(
#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_simdgroup>
#include <metal_quadgroup>

struct CppSLCtx;
template <typename T> struct __ScalarType { using Type = T; };
template <typename T> struct __ScalarType<metal::vec<T, 2>> { using Type = T; };
template <typename T> struct __ScalarType<metal::vec<T, 3>> { using Type = T; };
template <typename T> struct __ScalarType<metal::vec<T, 4>> { using Type = T; };

template <typename T>
using Bindless = constant T*;

using uint64 = uint64_t;
using uint32 = uint32_t;
using uint = uint32_t;

// Math intrinsics
using metal::abs; using metal::min; using metal::max; using metal::all; using metal::any; using metal::select;

// Helper overloads for min/max with mixed int/uint arguments to avoid ambiguity
inline uint min(int a, uint b) { return metal::min(uint(a), b); }
inline uint min(uint a, int b) { return metal::min(a, uint(b)); }
inline uint max(int a, uint b) { return metal::max(uint(a), b); }
inline uint max(uint a, int b) { return metal::max(a, uint(b)); }

using metal::sin; using metal::sinh; using metal::cos; using metal::cosh; using metal::atan; using metal::atanh; using metal::tan; using metal::tanh;
using metal::acos; using metal::acosh; using metal::asin; using metal::asinh; using metal::exp; using metal::exp2; using metal::log; using metal::log2;
using metal::log10; using metal::exp10; using metal::sqrt; using metal::rsqrt; using metal::ceil; using metal::floor; using metal::fract; using metal::trunc;
using metal::round; using metal::length; using metal::saturate; using metal::pow;
using metal::copysign; using metal::atan2; using metal::step; using metal::fma; using metal::smoothstep; using metal::normalize; using metal::dot; using metal::cross;
using metal::faceforward; using metal::reflect; using metal::transpose; using metal::determinant;
using metal::clz; using metal::ctz; using metal::popcount;

#define reversebits metal::reverse_bits

// Function aliases
template<typename T, typename U, typename X> T lerp(T a, U b, X t) { return metal::mix(a, b, t); }
template<typename T> T ddx(T p) { return metal::dfdx(p); }
template<typename T> T ddy(T p) { return metal::dfdy(p); }
template<typename T> auto is_inf(T x) { return metal::isinf(x); }
template<typename T> auto is_nan(T x) { return metal::isnan(x); }
template<typename T> auto length_squared(T v) { return metal::dot(v, v); }

template<typename T> uint asuint(T v) { return as_type<uint>(v); }
template<typename T, uint N> metal::vec<uint, N> asuint(metal::vec<T, N> v) { return as_type<metal::vec<uint, N>>(v); }

template<typename T> float asfloat(T v) { return as_type<float>(v); }
template<typename T, uint N> metal::vec<float, N> asfloat(metal::vec<T, N> v) { return as_type<metal::vec<float, N>>(v); }

template<typename T>
T clamp(T x, T minval, T maxval) { return metal::clamp(x, minval, maxval); }
// HLSL-compatible clamp functions to resolve ambiguity
int clamp(int x, int minval, int maxval) { return metal::clamp(x, minval, maxval); }
uint clamp(uint x, uint minval, uint maxval) { return metal::clamp(x, minval, maxval); }
float clamp(float x, float minval, float maxval) { return metal::clamp(x, minval, maxval); }
uint clamp(uint x, int minval, int maxval) { return metal::clamp(x, uint(minval), uint(maxval)); }
int clamp(int x, uint minval, uint maxval) { return metal::clamp(x, int(minval), int(maxval)); }

float2 CppSLMul(metal::float2 v, float s) { return float2(v.x * s, v.y * s); }
float2 CppSLMul(metal::int2 v, float s) { return float2(v.x * s, v.y * s); }
float2 CppSLMul(metal::uint2 v, float s) { return float2(v.x * s, v.y * s); }
float2 CppSLMul(float s, metal::float2 v) { return float2(v.x * s, v.y * s); }
float2 CppSLMul(float s, metal::int2 v) { return float2(v.x * s, v.y * s); }
float2 CppSLMul(float s, metal::uint2 v) { return float2(v.x * s, v.y * s); }

float2 CppSLDiv(metal::float2 v, float s) { return float2(v.x / s, v.y / s); }
float2 CppSLDiv(metal::int2 v, float s) { return float2(v.x / s, v.y / s); }
float2 CppSLDiv(metal::uint2 v, float s) { return float2(v.x / s, v.y / s); }
float2 CppSLDiv(float s, metal::float2 v) { return float2(s / v.x, s / v.y); }

float3 CppSLMul(metal::float3 v, float s) { return float3(v.x * s, v.y * s, v.z * s); }
float3 CppSLMul(metal::int3 v, float s) { return float3(v.x * s, v.y * s, v.z * s); }
float3 CppSLMul(metal::uint3 v, float s) { return float3(v.x * s, v.y * s, v.z * s); }
float3 CppSLMul(float s, metal::float3 v) { return float3(v.x * s, v.y * s, v.z * s); }
float3 CppSLMul(float s, metal::int3 v) { return float3(v.x * s, v.y * s, v.z * s); }
float3 CppSLMul(float s, metal::uint3 v) { return float3(v.x * s, v.y * s, v.z * s); }

float3 CppSLDiv(metal::float3 v, float s) { return float3(v.x / s, v.y / s, v.z / s); }
float3 CppSLDiv(metal::int3 v, float s) { return float3(v.x / s, v.y / s, v.z / s); }
float3 CppSLDiv(metal::uint3 v, float s) { return float3(v.x / s, v.y / s, v.z / s); }
float3 CppSLDiv(float s, metal::float3 v) { return float3(s / v.x, s / v.y, s / v.z); }

float4 CppSLMul(metal::float4 v, float s) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
float4 CppSLMul(metal::int4 v, float s) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
float4 CppSLMul(metal::uint4 v, float s) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
float4 CppSLMul(float s, metal::float4 v) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
float4 CppSLMul(float s, metal::int4 v) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
float4 CppSLMul(float s, metal::uint4 v) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }

float4 CppSLDiv(metal::float4 v, float s) { return float4(v.x / s, v.y / s, v.z / s, v.w / s); }
float4 CppSLDiv(metal::int4 v, float s) { return float4(v.x / s, v.y / s, v.z / s, v.w / s); }
float4 CppSLDiv(metal::uint4 v, float s) { return float4(v.x / s, v.y / s, v.z / s, v.w / s); }
float4 CppSLDiv(float s, metal::float4 v) { return float4(s / v.x, s / v.y, s / v.z, s / v.w); }
)__de___l___im__";

} // namespace CppSL::MSL