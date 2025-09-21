#pragma once
#include "array.hxx"
#include "vec.hxx"

template<>
struct [[builtin("matrix")]] matrix<2> {
    using ThisType = matrix<2>;
    using T = float;
    static constexpr uint32 N = 2;
    static constexpr uint32 X = 2;
    static constexpr uint32 Y = 2;

    constexpr matrix(float2 col0, float2 col1)
        : _v(col0, col1) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m10 = 0.f, float m11 = 0.f)
        : _v(float2(m00, m01), float2(m10, m11)) {}
    explicit constexpr matrix(matrix<3> f33);
    explicit constexpr matrix(matrix<4> f44);

    static constexpr matrix<2> identity();

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 2>, 2> _v;
};

template<>
struct [[builtin("matrix")]] matrix<3> {
    using ThisType = matrix<3>;
    using T = float;
    static constexpr uint32 N = 3;
    static constexpr uint32 X = 4;
    static constexpr uint32 Y = 3;

    constexpr matrix(float3 col0, float3 col1, float3 col2)
        : _v(float4(col0, 0), float4(col1, 0), float4(col2, 0)) {}

    constexpr matrix(
        float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, 
        float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, 
        float m20 = 0.f, float m21 = 0.f, float m22 = 0.f)
        : _v(float4(m00, m01, m02, 0),
             float4(m10, m11, m12, 0),
             float4(m20, m21, m22, 0)) {}

    explicit constexpr matrix(matrix<2> f22);
    explicit constexpr matrix(matrix<4> f44);

    static constexpr matrix<3> identity();

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 4>, 3> _v;
};

template<>
struct alignas(16) [[builtin("matrix")]] matrix<4> {
    using ThisType = matrix<4>;
    using T = float;
    static constexpr uint32 N = 4;
    static constexpr uint32 X = 4;
    static constexpr uint32 Y = 4;

    constexpr matrix(float4 col0, float4 col1, float4 col2, float4 col3)
        : _v(col0, col1, col2, col3) {}

    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m03 = 0.f,
                     float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, float m13 = 0.f,
                     float m20 = 0.f, float m21 = 0.f, float m22 = 0.f, float m23 = 0.f,
                     float m30 = 0.f, float m31 = 0.f, float m32 = 0.f, float m33 = 0.f)
        : _v(vec<float, 4>(m00, m01, m02, m03),
             vec<float, 4>(m10, m11, m12, m13),
             vec<float, 4>(m20, m21, m22, m23),
             vec<float, 4>(m30, m31, m32, m33)) {}

    explicit constexpr matrix(matrix<2> f22);
    explicit constexpr matrix(matrix<3> f33);

    static constexpr matrix<4> identity();

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 4>, 4> _v;
};

constexpr matrix<2>::matrix(matrix<3> f33)
    : _v(float2(f33.get(0, 0), f33.get(0, 1)),
         float2(f33.get(1, 0), f33.get(1, 1))) {}

constexpr matrix<2>::matrix(matrix<4> f44)
    : _v(float2(f44.get(0, 0), f44.get(0, 1)),
         float2(f44.get(1, 0), f44.get(1, 1))) {}

constexpr matrix<2> matrix<2>::identity() {
    return matrix<2>(1.f, 0.f,
                     0.f, 1.f);
}

constexpr matrix<3>::matrix(matrix<2> f22)
    : _v(float4(f22.access_(0), 0, 0),
         float4(f22.access_(1), 0, 0),
         float4(0, 0, 0, 0)) {}

constexpr matrix<3>::matrix(matrix<4> f44)
    : _v(float4(f44.access_(0)),
         float4(f44.access_(1)),
         float4(f44.access_(2))) {}

constexpr matrix<3> matrix<3>::identity() {
    return matrix<3>(
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f);
}

constexpr matrix<4>::matrix(matrix<2> f22)
    : _v(float4(f22.access_(0), 0, 0),
         float4(f22.access_(1), 0, 0),
         float4(0, 0, 0, 0),
         float4(0, 0, 0, 0)) {}

constexpr matrix<4>::matrix(matrix<3> f33)
    : _v(float4(f33.access_(0), 0),
         float4(f33.access_(1), 0),
         float4(f33.access_(2), 0),
         float4(f33.access_(3), 0)) {}

constexpr matrix<4> matrix<4>::identity() {
    return matrix<4>(
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f);
}

using float2x2 = matrix<2>;
using float3x3 = matrix<3>;
using float4x4 = matrix<4>;

[[binop("MUL")]] float2 mul(float2 v, float2x2 m);
[[binop("MUL")]] float3 mul(float3 v, float3x3 m);
[[binop("MUL")]] float4 mul(float4 v, float4x4 m);

[[binop("MUL")]] float2 mul(float2x2 m, float2 v);
[[binop("MUL")]] float3 mul(float3x3 m, float3 v);
[[binop("MUL")]] float4 mul(float4x4 m, float4 v);

[[binop("MUL")]] float2x2 mul(float2x2 m1, float2x2 m2);
[[binop("MUL")]] float3x3 mul(float3x3 m1, float3x3 m2); 
[[binop("MUL")]] float4x4 mul(float4x4 m1, float4x4 m2);
