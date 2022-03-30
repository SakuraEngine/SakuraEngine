#pragma once
#include "platform/configure.h"
#include "containers/span.hpp"
#pragma warning(push)
#pragma warning(disable : 4668)
#include <DirectXMath.h>
#pragma warning(pop)
#include <DirectXPackedVector.h>

namespace sakura
{
namespace math
{
namespace __vector
{
using VectorRegister = DirectX::XMVECTOR;
#ifdef _XM_ARM_NEON_INTRINSICS_
using VectorRegisterInt = int32x4_t;
constexpr uint32_t cElementIndex_u[4]{ 1, 2, 4, 8 };
static inline uint32_t vmaskq_u32(const uint32x4_t& CR)
{
    static const uint32x4_t Mask = vld1q_u32(cElementIndex_u);
    // extract element index bitmask from compare result.
    uint32x4_t vTemp = vandq_u32(CR, Mask);
    uint32x2_t vL = vget_low_u32(vTemp);  // get low 2 uint32
    uint32x2_t vH = vget_high_u32(vTemp); // get high 2 uint32
    vL = vorr_u32(vL, vH);
    vL = vpadd_u32(vL, vL);
    return vget_lane_u32(vL, 0);
}
constexpr int32_t cElementIndex_i[4]{ 1, 2, 4, 8 };
static inline int32_t vmaskq_i32(const int32x4_t& CR)
{
    static const int32x4_t Mask = vld1q_s32(cElementIndex_i);
    // extract element index bitmask from compare result.
    int32x4_t vTemp = vandq_s32(CR, Mask);
    int32x2_t vL = vget_low_s32(vTemp);  // get low 2 uint32
    int32x2_t vH = vget_high_s32(vTemp); // get high 2 uint32
    vL = vorr_s32(vL, vH);
    vL = vpadd_s32(vL, vL);
    return vget_lane_s32(vL, 0);
}
#else
using VectorRegisterInt = __m128i;
#endif

FORCEINLINE VectorRegister vector_register(float x, float y, float z, float w)
{
    return DirectX::XMVectorSet(x, y, z, w);
}

FORCEINLINE VectorRegister vector_register(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
    return DirectX::XMVectorSetInt(x, y, z, w);
}

static VectorRegister register_zero = DirectX::XMVectorZero();
static VectorRegister register_one = DirectX::g_XMOne.v;
static const VectorRegister float4_infinity = vector_register((uint32_t)0x7F800000, (uint32_t)0x7F800000, (uint32_t)0x7F800000, (uint32_t)0x7F800000);

FORCEINLINE VectorRegister load(const sakura::span<const float, 4> vec)
{
    return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(vec.data()));
}

FORCEINLINE VectorRegister load_uint1(const sakura::span<const uint32_t, 1> vec)
{
    return vector_register(vec[0], vec[1], vec[0], vec[1]);
}

FORCEINLINE VectorRegister load_float1(const sakura::span<const float, 1> vec)
{
    return vector_register(vec[0], vec[1], vec[0], vec[1]);
}

FORCEINLINE VectorRegister load_float2(const sakura::span<const float, 2> vec)
{
    return vector_register(vec[0], vec[1], vec[0], vec[1]);
}

FORCEINLINE VectorRegister load_uint2(const sakura::span<const uint32_t, 2> vec)
{
    return vector_register(vec[0], vec[1], vec[0], vec[1]);
}

FORCEINLINE VectorRegister load_float3_w0(const sakura::span<const float, 3> vec)
{
    return vector_register(vec[0], vec[1], vec[2], 0.f);
}

FORCEINLINE VectorRegister load_float3_w1(const sakura::span<const float, 3> vec)
{
    return vector_register(vec[0], vec[1], vec[2], 1.f);
}

FORCEINLINE VectorRegister load_uint3_w0(const sakura::span<const uint32_t, 3> vec)
{
    return vector_register(vec[0], vec[1], vec[2], 0u);
}

FORCEINLINE VectorRegister load_uint3_w1(const sakura::span<const uint32_t, 3> vec)
{
    return vector_register(vec[0], vec[1], vec[2], 1u);
}

FORCEINLINE VectorRegister load_aligned(const sakura::span<const float, 4> vec)
{
    return DirectX::XMLoadFloat4A(reinterpret_cast<const DirectX::XMFLOAT4A*>(vec.data()));
}

FORCEINLINE void store_aligned(sakura::span<float, 4> target, const VectorRegister vector)
{
    return DirectX::XMStoreFloat4A(reinterpret_cast<DirectX::XMFLOAT4A*>(target.data()), vector);
}

FORCEINLINE void store(sakura::span<float, 4> target, const VectorRegister vector)
{
    return DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(target.data()), vector);
}

FORCEINLINE void store_float3(sakura::span<float, 3> target, const VectorRegister vector)
{
    return DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(target.data()), vector);
}

FORCEINLINE void store_float1(sakura::span<float, 3> target, const VectorRegister vector)
{
    return DirectX::XMStoreFloat(static_cast<float*>(target.data()), vector);
}

FORCEINLINE VectorRegister set_w0(const VectorRegister vec)
{
    return DirectX::XMVectorPermute<0, 1, 2, 7>(vec, register_zero);
}

FORCEINLINE VectorRegister set_w1(const VectorRegister vec)
{
    return DirectX::XMVectorPermute<0, 1, 2, 7>(vec, register_one);
}

FORCEINLINE float get_component(const VectorRegister vector, uint32_t index)
{
    switch (index)
    {
        case 0:
            return DirectX::XMVectorGetX(vector);
        case 1:
            return DirectX::XMVectorGetY(vector);
        case 2:
            return DirectX::XMVectorGetZ(vector);
        case 3:
            return DirectX::XMVectorGetW(vector);
    }
    return 0.0f;
}

template <size_t X, size_t Y, size_t Z, size_t W>
FORCEINLINE VectorRegister permute(const VectorRegister vec1, const VectorRegister vec2)
{
    static_assert(X <= 7, "permute: X <=7!!");
    static_assert(Y <= 7, "permute: Y <=7!!");
    static_assert(Z <= 7, "permute: Z <=7!!");
    static_assert(W <= 7, "permute: W <=7!!");
    return DirectX::XMVectorPermute<X, Y, Z, W>(vec1, vec2);
}

template <size_t X, size_t Y, size_t Z, size_t W>
FORCEINLINE VectorRegister shuffle(const VectorRegister vec1, const VectorRegister vec2)
{
    static_assert(X <= 7, "shuffle: X <=3!!");
    static_assert(Y <= 7, "shuffle: Y <=3!!");
    static_assert(Z <= 7, "shuffle: Z <=3!!");
    static_assert(W <= 7, "shuffle: W <=3!!");
    return DirectX::XMVectorPermute<X, Y, Z + 4, W + 4>(vec1, vec2);
}

template <size_t X, size_t Y, size_t Z, size_t W>
FORCEINLINE VectorRegister swizzle(const VectorRegister vec)
{
    static_assert(X <= 3, "swizzle: X <=3!!");
    static_assert(Y <= 3, "swizzle: Y <=3!!");
    static_assert(Z <= 3, "swizzle: Z <=3!!");
    static_assert(W <= 3, "swizzle: W <=3!!");
    return DirectX::XMVectorSwizzle<X, Y, Z, W>(vec);
}

FORCEINLINE VectorRegister abs(const VectorRegister vec)
{
    return DirectX::XMVectorAbs(vec);
}

FORCEINLINE VectorRegister negate(const VectorRegister vec)
{
    return DirectX::XMVectorNegate(vec);
}

FORCEINLINE VectorRegister add(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorAdd(vec1, vec2);
}

FORCEINLINE VectorRegister subtract(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorSubtract(vec1, vec2);
}

FORCEINLINE VectorRegister multiply(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorMultiply(vec1, vec2);
}

FORCEINLINE VectorRegister divide(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorDivide(vec1, vec2);
}

FORCEINLINE VectorRegister multiply_add(const VectorRegister vec1, const VectorRegister vec2, const VectorRegister vec3)
{
    return DirectX::XMVectorMultiplyAdd(vec1, vec2, vec3);
}

FORCEINLINE VectorRegister dot2(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVector2Dot(vec1, vec2);
}

FORCEINLINE VectorRegister dot3(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVector3Dot(vec1, vec2);
}

FORCEINLINE VectorRegister dot4(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVector4Dot(vec1, vec2);
}

FORCEINLINE VectorRegister equals(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorEqual(vec1, vec2);
}

FORCEINLINE VectorRegister not_equals(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorNotEqual(vec1, vec2);
}

FORCEINLINE VectorRegister greater(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorGreater(vec1, vec2);
}

FORCEINLINE VectorRegister greater_or_equal(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorGreaterOrEqual(vec1, vec2);
}

FORCEINLINE VectorRegister less(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorLess(vec1, vec2);
}

FORCEINLINE VectorRegister less_or_equal(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorLessOrEqual(vec1, vec2);
}

FORCEINLINE VectorRegister select(const VectorRegister mask, const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorSelect(vec2, vec1, mask);
}

FORCEINLINE VectorRegister bitwise_or(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorOrInt(vec1, vec2);
}

FORCEINLINE VectorRegister bitwise_and(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorAndInt(vec1, vec2);
}

FORCEINLINE VectorRegister bitwise_xor(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorXorInt(vec1, vec2);
}

FORCEINLINE int component_mask(const VectorRegister vec1)
{
#ifdef _XM_ARM_NEON_INTRINSICS_
    return vmaskq_i32(vec1);
#else
    return _mm_movemask_ps(vec1);
#endif
}

FORCEINLINE VectorRegister cross_product(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVector3Cross(vec1, vec2);
}

FORCEINLINE VectorRegister power(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorPow(vec1, vec2);
}

FORCEINLINE VectorRegister reciprocal_sqrt(const VectorRegister vec)
{
    return DirectX::XMVectorReciprocalSqrt(vec);
}

FORCEINLINE VectorRegister reciprocal_sqrt_quick(const VectorRegister vec)
{
    return DirectX::XMVectorReciprocalSqrtEst(vec);
}

FORCEINLINE VectorRegister reciprocal(const VectorRegister vec)
{
    return DirectX::XMVectorReciprocal(vec);
}

FORCEINLINE VectorRegister reciprocal_quick(const VectorRegister vec)
{
    return DirectX::XMVectorReciprocalEst(vec);
}

FORCEINLINE VectorRegister reciprocal_length(const VectorRegister vec)
{
    return DirectX::XMVector4ReciprocalLength(vec);
}

FORCEINLINE VectorRegister reciprocal_length_quick(const VectorRegister vec)
{
    return DirectX::XMVector4ReciprocalLengthEst(vec);
}

FORCEINLINE VectorRegister normalize(const VectorRegister vec)
{
    return DirectX::XMVector4NormalizeEst(vec);
}

FORCEINLINE VectorRegister normalize_quick(const VectorRegister vec)
{
    return DirectX::XMVector4NormalizeEst(vec);
}

FORCEINLINE VectorRegister min(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorMin(vec1, vec2);
}

FORCEINLINE VectorRegister max(const VectorRegister vec1, const VectorRegister vec2)
{
    return DirectX::XMVectorMax(vec1, vec2);
}

FORCEINLINE VectorRegister quat_multiply(const VectorRegister& Quat1, const VectorRegister& Quat2)
{
    // DirectXMath uses reverse parameter order to UnrealMath
    // XMQuaternionMultiply( FXMVECTOR Q1, FXMVECTOR Q2)
    // Returns the product Q2*Q1 (which is the concatenation of a rotation Q1 followed by the rotation Q2)

    // [ (Q2.w * Q1.x) + (Q2.x * Q1.w) + (Q2.y * Q1.z) - (Q2.z * Q1.y),
    //   (Q2.w * Q1.y) - (Q2.x * Q1.z) + (Q2.y * Q1.w) + (Q2.z * Q1.x),
    //   (Q2.w * Q1.z) + (Q2.x * Q1.y) - (Q2.y * Q1.x) + (Q2.z * Q1.w),
    //   (Q2.w * Q1.w) - (Q2.x * Q1.x) - (Q2.y * Q1.y) - (Q2.z * Q1.z) ]
    return DirectX::XMQuaternionMultiply(Quat2, Quat1);
}

FORCEINLINE void quat_multiply(VectorRegister* VResult, const VectorRegister* VQuat1, const VectorRegister* VQuat2)
{
    *VResult = quat_multiply(*VQuat1, *VQuat2);
}

// Returns true if the __vector contains a component that is either NAN or +/-infinite.
FORCEINLINE bool contains_nan_or_infinite(const VectorRegister& Vec)
{
    // Mask off Exponent
    const VectorRegister ExpTest = bitwise_and(Vec, float4_infinity);
    // Compare to full exponent. If any are full exponent (not finite), the signs copied to the mask are non-zero, otherwise it's zero and finite.
    bool IsFinite = component_mask(equals(ExpTest, float4_infinity)) == 0;
    return !IsFinite;
}

FORCEINLINE VectorRegister exp2(const VectorRegister& X)
{
    return DirectX::XMVectorExp2(X);
}

FORCEINLINE VectorRegister log2(const VectorRegister& X)
{
    return DirectX::XMVectorLog2(X);
}

FORCEINLINE VectorRegister sin(const VectorRegister& X)
{
    return DirectX::XMVectorSin(X);
}

FORCEINLINE VectorRegister sin_quick(const VectorRegister& X)
{
    return DirectX::XMVectorSinEst(X);
}

FORCEINLINE VectorRegister asin(const VectorRegister& X)
{
    return DirectX::XMVectorASin(X);
}

FORCEINLINE VectorRegister asin_quick(const VectorRegister& X)
{
    return DirectX::XMVectorASinEst(X);
}

FORCEINLINE VectorRegister cos(const VectorRegister& X)
{
    return DirectX::XMVectorCos(X);
}

FORCEINLINE VectorRegister cos_quick(const VectorRegister& X)
{
    return DirectX::XMVectorCosEst(X);
}

FORCEINLINE VectorRegister acos(const VectorRegister& X)
{
    return DirectX::XMVectorACos(X);
}

FORCEINLINE VectorRegister acos_quick(const VectorRegister& X)
{
    return DirectX::XMVectorACosEst(X);
}

FORCEINLINE VectorRegister tan(const VectorRegister& X)
{
    return DirectX::XMVectorTan(X);
}

FORCEINLINE VectorRegister tan_quick(const VectorRegister& X)
{
    return DirectX::XMVectorTanEst(X);
}

FORCEINLINE VectorRegister atan(const VectorRegister& X)
{
    return DirectX::XMVectorATan(X);
}

FORCEINLINE VectorRegister atan_quick(const VectorRegister& X)
{
    return DirectX::XMVectorATanEst(X);
}

FORCEINLINE VectorRegister ceil(const VectorRegister& X)
{
    return DirectX::XMVectorCeiling(X);
}

FORCEINLINE VectorRegister floor(const VectorRegister& X)
{
    return DirectX::XMVectorFloor(X);
}

FORCEINLINE VectorRegister truncate(const VectorRegister& X)
{
    return DirectX::XMVectorTruncate(X);
}

FORCEINLINE VectorRegister fractional(const VectorRegister& X)
{
    return subtract(X, truncate(X));
}

FORCEINLINE VectorRegister mod(const VectorRegister& X, const VectorRegister& Y)
{
    return DirectX::XMVectorMod(X, Y);
}
} // namespace __vector
} // namespace math
} // namespace sakura