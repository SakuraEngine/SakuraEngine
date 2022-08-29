#pragma once
#include "SDXMathVector.hpp"
#include "SDXMathQuaternion.hpp"

namespace skr
{
namespace math
{
namespace __matrix
{
using MatrixRegister = DirectX::XMMATRIX;
using VectorRegister = DirectX::XMVECTOR;

FORCEINLINE void store_aligned(skr::span<float, 16> target, const MatrixRegister matrix)
{
    return DirectX::XMStoreFloat4x4A(reinterpret_cast<DirectX::XMFLOAT4X4A*>(target.data()), matrix);
}

FORCEINLINE MatrixRegister load_aligned(const skr::span<const float, 16> target)
{
    return DirectX::XMLoadFloat4x4A(reinterpret_cast<const DirectX::XMFLOAT4X4A*>(target.data()));
}

FORCEINLINE MatrixRegister inverse(
const MatrixRegister a)
{
    return DirectX::XMMatrixInverse(nullptr, a);
}

FORCEINLINE MatrixRegister transpose(
const MatrixRegister a)
{
    return DirectX::XMMatrixTranspose(a);
}

FORCEINLINE VectorRegister multiply(
const VectorRegister a,
const MatrixRegister b)
{
    return DirectX::XMVector4Transform(a, b);
}

FORCEINLINE MatrixRegister multiply(
const MatrixRegister a,
const MatrixRegister b)
{
    return DirectX::XMMatrixMultiply(a, b);
}

FORCEINLINE MatrixRegister make_transform_trs(
const Vector3f translation,
const Vector3f scale,
const Quaternion quaternion)
{
    using namespace DirectX;
    return DirectX::XMMatrixTransformation(
        g_XMZero,
        g_XMIdentityR3,
        __vector::load_float3_w0(scale.data_view()),
        g_XMZero,
        __vector::load_aligned(quaternion.data_view()),
        __vector::load_float3_w0(translation.data_view()));
}

FORCEINLINE MatrixRegister make_transform_t(
const Vector3f translation)
{
    using namespace DirectX;
    return XMMatrixTranslationFromVector(__vector::load_float3_w0(translation.data_view()));
}

FORCEINLINE MatrixRegister look_at(
    const Vector3f Eye,
    const Vector3f At,
    const Vector3f Up = Vector3f(0.f, 1.f, 0.f) )
{
    return DirectX::XMMatrixLookAtRH(
        __vector::load_float3_w1(Eye.data_view()),
        __vector::load_float3_w1(At.data_view()),
        __vector::load_float3_w0(Up.data_view()));
}

FORCEINLINE MatrixRegister perspective_fov(
float FovAngleY,
float AspectRatio,
float NearZ,
float FarZ)
{
    return DirectX::XMMatrixPerspectiveFovRH(
    FovAngleY, AspectRatio, NearZ, FarZ);
}

FORCEINLINE MatrixRegister ortho_projection(
float ViewWidth,
float ViewHeight,
float NearZ,
float FarZ)
{
    return DirectX::XMMatrixOrthographicRH(
    ViewWidth, ViewHeight, NearZ, FarZ);
}
} // namespace __matrix
} // namespace math
} // namespace skr