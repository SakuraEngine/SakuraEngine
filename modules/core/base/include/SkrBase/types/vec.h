#pragma once
#include "SkrBase/config.h"

#ifdef __cplusplus
    #define SKR_DECLARE_VEC2_BODY(TT, NAME)                                                          \
        TT                    x, y;                                                                  \
        SKR_FORCEINLINE bool  operator==(const NAME& vec) const { return x == vec.x && y == vec.y; } \
        SKR_FORCEINLINE bool  operator!=(const NAME& vec) const { return x != vec.x || y != vec.y; } \
        SKR_FORCEINLINE       NAME()                 = default;                                      \
        SKR_FORCEINLINE       NAME(const NAME&)      = default;                                      \
        SKR_FORCEINLINE       NAME(NAME&&)           = default;                                      \
        SKR_FORCEINLINE NAME& operator=(const NAME&) = default;                                      \
        SKR_FORCEINLINE NAME& operator=(NAME&&)      = default;                                      \
        SKR_FORCEINLINE       NAME(TT X, TT Y)                                                       \
            : x(X)                                                                                   \
            , y(Y)                                                                                   \
        {                                                                                            \
        }                                                                                            \
        SKR_FORCEINLINE NAME(TT t)                                                                   \
            : x(t)                                                                                   \
            , y(t)                                                                                   \
        {                                                                                            \
        }
#else
    #define SKR_DECLARE_VEC2_BODY(TT, NAME) TT x, y;
#endif

typedef struct skr_uint64x2_t {
    SKR_DECLARE_VEC2_BODY(uint64_t, skr_uint64x2_t)
} skr_uint64x2_t;

typedef struct skr_uint32x2_t {
    SKR_DECLARE_VEC2_BODY(uint32_t, skr_uint32x2_t)
} skr_uint32x2_t;

typedef struct skr_float2_t {
    SKR_DECLARE_VEC2_BODY(float, skr_float2_t)
} skr_float2_t;

typedef struct skr_double2_t {
    SKR_DECLARE_VEC2_BODY(double, skr_double2_t)
} skr_double2_t;

typedef struct skr_float3_t {
    float x SKR_IF_CPP(= 0.f);
    float y SKR_IF_CPP(= 0.f);
    float z SKR_IF_CPP(= 0.f);
} skr_float3_t;

typedef struct SKR_ALIGNAS(16) skr_float4_t {
    float x SKR_IF_CPP(= 0.f);
    float y SKR_IF_CPP(= 0.f);
    float z SKR_IF_CPP(= 0.f);
    float w SKR_IF_CPP(= 0.f);
} skr_float4_t;

typedef struct skr_rotator_t {
    float pitch SKR_IF_CPP(= 0.f);
    float yaw   SKR_IF_CPP(= 0.f);
    float roll  SKR_IF_CPP(= 0.f);
} skr_rotator_t;

typedef struct SKR_ALIGNAS(16) skr_quaternion_t {
    float x SKR_IF_CPP(= 0.f);
    float y SKR_IF_CPP(= 0.f);
    float z SKR_IF_CPP(= 0.f);
    float w SKR_IF_CPP(= 0.f);
} skr_quaternion_t;

typedef struct SKR_ALIGNAS(16) skr_transform_t {
    skr_rotator_t rotation   SKR_IF_CPP(= { 0.f, 0.f, 0.f });
    skr_float3_t translation SKR_IF_CPP(= { 0.f, 0.f, 0.f });
    skr_float3_t scale       SKR_IF_CPP(= { 1.f, 1.f, 1.f });
} skr_transform_t;

typedef struct SKR_ALIGNAS(16) skr_float4x4_t {
    float M[4][4];
} skr_float4x4_t;

#ifdef __cplusplus
namespace skr
{

using float2 = skr_float2_t;
using float3 = skr_float3_t;
using float4 = skr_float4_t;
using float4x4 = skr_float4x4_t;

using Rotator = skr_rotator_t;
using Quaternion = skr_quaternion_t;
using Transform = skr_transform_t;

namespace scalar_math
{
inline skr_float3_t operator*(float s, skr_float3_t v) { return skr_float3_t{ s * v.x, s * v.y, s * v.z }; }
inline skr_float3_t operator*(skr_float3_t v, float s) { return skr_float3_t{ s * v.x, s * v.y, s * v.z }; }
inline skr_float3_t operator+(skr_float3_t a, skr_float3_t b) { return skr_float3_t{ a.x + b.x, a.y + b.y, a.z + b.z }; }
} // namespace scalar_math
} // namespace skr

inline static SKR_CONSTEXPR bool operator==(skr_uint32x2_t l, uint32_t r)
{
    return (l.x == r) && (l.y == r);
}
inline static SKR_CONSTEXPR bool operator!=(skr_uint32x2_t l, uint32_t r)
{
    return (l.x != r) || (l.y != r);
}
inline static SKR_CONSTEXPR bool operator==(uint32_t l, skr_uint32x2_t r)
{
    return (l == r.x) && (l == r.y);
}
inline static SKR_CONSTEXPR bool operator!=(uint32_t l, skr_uint32x2_t r)
{
    return (l != r.x) || (l != r.y);
}
inline static SKR_CONSTEXPR bool operator==(skr_float3_t l, skr_float3_t r)
{
    return (l.x == r.x) && (l.y == r.y) && (l.z == r.z);
}
inline static SKR_CONSTEXPR bool operator!=(skr_float3_t l, skr_float3_t r)
{
    return (l.x != r.x) || (l.y != r.y) || (l.z != r.z);
}
inline static SKR_CONSTEXPR bool operator==(skr_float4_t l, skr_float4_t r)
{
    return (l.x == r.x) && (l.y == r.y) && (l.z == r.z) && (l.w == r.w);
}
inline static SKR_CONSTEXPR bool operator!=(skr_float4_t l, skr_float4_t r)
{
    return (l.x != r.x) || (l.y != r.y) || (l.z != r.z) || (l.w != r.w);
}
inline static SKR_CONSTEXPR bool operator==(skr_rotator_t l, skr_rotator_t r)
{
    return (l.pitch == r.pitch) && (l.yaw == r.yaw) && (l.roll == r.roll);
}
inline static SKR_CONSTEXPR bool operator!=(skr_rotator_t l, skr_rotator_t r)
{
    return (l.pitch != r.pitch) || (l.yaw != r.yaw) || (l.roll != r.roll);
}
inline bool operator==(skr_md5_t a, skr_md5_t b)
{
    const skr_md5_u32x4_view_t* va     = (skr_md5_u32x4_view_t*)&a;
    const skr_md5_u32x4_view_t* vb     = (skr_md5_u32x4_view_t*)&b;
    int                         result = true;
    result &= (va->a == vb->a);
    result &= (va->b == vb->b);
    result &= (va->c == vb->c);
    result &= (va->d == vb->d);
    return result;
}
#endif
