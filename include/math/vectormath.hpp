#pragma once
#include "vector.hpp"
#include "matrix.hpp"
#include "rotator.hpp"
#include "quaternion.hpp"
#include "transform.hpp"

namespace sakura
{
namespace math
{
// Implementations
FORCEINLINE float4x4 make_transform(
    const Vector3f translation,
    const Vector3f scale = Vector3f::vector_one(),
    const Quaternion quaternion = Quaternion::identity())
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(), __matrix::make_transform_trs(translation, scale, quaternion));
    return res;
}

FORCEINLINE float4x4 make_transform_t(
    const Vector3f translation)
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(), __matrix::make_transform_t(translation));
    return res;
}

FORCEINLINE float4x4 perspective_fov(
    float FovAngleY,
    float AspectRatio,
    float NearZ,
    float FarZ)
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(),
        __matrix::inverse(__matrix::perspective_fov(FovAngleY, AspectRatio, NearZ, FarZ)));
    return res;
}

FORCEINLINE float4x4 ortho_projection(
    float ViewWidth,
    float ViewHeight,
    float NearZ,
    float FarZ)
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(),
        __matrix::inverse(__matrix::ortho_projection(ViewWidth, ViewHeight, NearZ, FarZ)));
    return res;
}

FORCEINLINE float4x4 look_at_matrix(
    const Vector3f Eye,
    const Vector3f At)
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(), __matrix::look_at(Eye, At));
    return res;
}

FORCEINLINE float4x4 multiply(
    const float4x4 a,
    const float4x4 b)
{
    float4x4 res;
    __matrix::store_aligned(
        res.data_view(),
        __matrix::multiply(__matrix::load_aligned(a.data_view()), __matrix::load_aligned(b.data_view())));
    return res;
}

FORCEINLINE Vector4f multiply(
    const Vector4f a,
    const float4x4 b)
{
    Vector4f res;
    __vector::store_aligned(
        res.data_view(),
        __matrix::multiply(__vector::load_aligned(a.data_view()), __matrix::load_aligned(b.data_view())));
    return res;
}

FORCEINLINE float4x4 inverse(
    const float4x4 a)
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(),
        __matrix::inverse(__matrix::load_aligned(a.data_view())));
    return res;
}

FORCEINLINE float4x4 transpose(
    const float4x4 a)
{
    float4x4 res;
    __matrix::store_aligned(res.data_view(),
        __matrix::transpose(__matrix::load_aligned(a.data_view())));
    return res;
}

// Wrappers

FORCEINLINE Quaternion quaternion_from_rotator(
    const Rotator rot)
{
    Quaternion res;
    __vector::store_aligned(res.data_view(),
        __quaternion::quaternion_from_euler(rot.pitch(), rot.yaw(), rot.roll()));
    return res;
}

FORCEINLINE Quaternion quaternion_from_euler(
    const float pitch, const float yaw, const float roll)
{
    Quaternion res;
    __vector::store_aligned(res.data_view(), __quaternion::quaternion_from_euler(pitch, yaw, roll));
    return res;
}

FORCEINLINE Quaternion quaternion_from_rotation(
    const float4x4 rotation)
{
    Quaternion res;
    __vector::store_aligned(res.data_view(), __quaternion::quaternion_from_rotation(rotation));
    return res;
}

FORCEINLINE Quaternion look_at_quaternion(
    const Vector3f direction)
{
    return quaternion_from_rotation(look_at_matrix(Vector3f::vector_zero(), direction));
}

FORCEINLINE Quaternion quaternion_from_axis(
    const Vector3f axis, const float angle)
{
    Quaternion res;
    __vector::store_aligned(res.data_view(), __quaternion::quaternion_from_axis(__vector::load_float3_w0(axis.data_view()), angle));
    return res;
}

FORCEINLINE float4x4 make_transform_2d(
    Vector2f pos2d, float rot2d, Vector2f scale2d)
{
    Vector3f pos{ pos2d.X, pos2d.Y, 0 };
    // TODO: rotate pivot is left bottom now
    Quaternion rot = quaternion_from_axis(Vector3f{ 0.f, 0.f, 1.f }, rot2d);
    Vector3f scale{ scale2d.X, scale2d.Y, 1 };
    return make_transform(pos, scale, rot);
}

FORCEINLINE float2x2 multiply(
    float2x2 a, float2x2 b)
{
    auto va = a.data_view(), vb = b.data_view();
    float result[] = { va[0] * vb[0] + va[1] * vb[2], va[0] * vb[1] + va[1] * vb[3], va[2] * vb[0] + va[3] * vb[2], va[2] * vb[1] + va[3] * vb[3] };
    return float2x2{ result };
}

FORCEINLINE Vector2f multiply(
    float2x2 a, Vector2f b)
{
    return Vector2f{ b.x * a.M[0][0] + b.y * a.M[1][0], b.x * a.M[0][1] + b.y * a.M[1][1] };
}
} // namespace math
} // namespace sakura