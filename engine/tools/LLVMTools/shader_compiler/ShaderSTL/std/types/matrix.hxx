#pragma once
#include "array.hxx"
#include "vec.hxx"

template<>
struct [[builtin("matrix")]] matrix<2, 2> {
    using ThisType = matrix<2, 2>;
    using T = float;
    static constexpr uint32 X = 2;
    static constexpr uint32 Y = 2;

    constexpr matrix(float2 col0, float2 col1)
        : _v(col0, col1) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m10 = 0.f, float m11 = 0.f)
        : _v(float2(m00, m01), float2(m10, m11)) {}

    static constexpr matrix<2, 2> identity() {
        // clang-format off
        return matrix<2, 2>(
            1.f, 0.f,  // col 0
            0.f, 1.f  // col 1
        );
        // clang-format on
    }

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 2>, 2> _v;
};

template<>
struct [[builtin("matrix")]] matrix<2, 3> {
    using ThisType = matrix<2, 3>;
    using T = float;
    static constexpr uint32 X = 2;
    static constexpr uint32 Y = 3;

    constexpr matrix(float3 col0, float3 col1)
        : _v(col0, col1) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m10 = 0.f, float m11 = 0.f, float m12 = 0.f)
        : _v(float3(m00, m01, m02), float3(m10, m11, m12)) {}

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 3>, 2> _v;
};

template<>
struct [[builtin("matrix")]] matrix<2, 4> {
    using ThisType = matrix<2, 4>;
    using T = float;
    static constexpr uint32 X = 2;
    static constexpr uint32 Y = 4;

    constexpr matrix(float4 col0, float4 col1)
        : _v(col0, col1) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m03 = 0.f, float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, float m13 = 0.f)
        : _v(float4(m00, m01, m02, m03), float4(m10, m11, m12, m13)) {}

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 4>, 2> _v;
};

template<>
struct [[builtin("matrix")]] matrix<3, 2> {
    using ThisType = matrix<3, 2>;
    using T = float;
    static constexpr uint32 X = 3;
    static constexpr uint32 Y = 2;

    constexpr matrix(float2 col0, float2 col1, float2 col2)
        : _v(col0, col1, col2) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m10 = 0.f, float m11 = 0.f, float m20 = 0.f, float m21 = 0.f)
        : _v(float2(m00, m01), float2(m10, m11), float2(m20, m21)) {}

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 2>, 3> _v;
};

template<>
struct [[builtin("matrix")]] matrix<3, 3> {
    using ThisType = matrix<3, 3>;
    using T = float;
    static constexpr uint32 X = 3;
    static constexpr uint32 Y = 3;

    constexpr matrix(float3 col0, float3 col1, float3 col2)
        : _v(col0, col1, col2) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, float m20 = 0.f, float m21 = 0.f, float m22 = 0.f)
        : _v(float3(m00, m01, m02), float3(m10, m11, m12), float3(m20, m21, m22)) {}

    static constexpr matrix<3, 3> identity() {
        // clang-format off
        return matrix<3, 3>(
            1.f, 0.f, 0.f,  // col 0
            0.f, 1.f, 0.f,  // col 1
            0.f, 0.f, 1.f  // col 2
        );
        // clang-format on
    }

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 3>, 3> _v;
};

template<>
struct [[builtin("matrix")]] matrix<3, 4> {
    using ThisType = matrix<3, 4>;
    using T = float;
    static constexpr uint32 X = 3;
    static constexpr uint32 Y = 4;

    constexpr matrix(float4 col0, float4 col1, float4 col2)
        : _v(col0, col1, col2) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m03 = 0.f, float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, float m13 = 0.f, float m20 = 0.f, float m21 = 0.f, float m22 = 0.f, float m23 = 0.f)
        : _v(float4(m00, m01, m02, m03), float4(m10, m11, m12, m13), float4(m20, m21, m22, m23)) {}

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 4>, 3> _v;
};

template<>
struct [[builtin("matrix")]] matrix<4, 2> {
    using ThisType = matrix<4, 2>;
    using T = float;
    static constexpr uint32 X = 4;
    static constexpr uint32 Y = 2;

    constexpr matrix(float2 col0, float2 col1, float2 col2, float2 col3)
        : _v(col0, col1, col2, col3) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m10 = 0.f, float m11 = 0.f, float m20 = 0.f, float m21 = 0.f, float m30 = 0.f, float m31 = 0.f)
        : _v(float2(m00, m01), float2(m10, m11), float2(m20, m21), float2(m30, m31)) {}

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 2>, 4> _v;
};

template<>
struct [[builtin("matrix")]] matrix<4, 3> {
    using ThisType = matrix<4, 3>;
    using T = float;
    static constexpr uint32 X = 4;
    static constexpr uint32 Y = 3;

    constexpr matrix(float3 col0, float3 col1, float3 col2, float3 col3)
        : _v(col0, col1, col2, col3) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, float m20 = 0.f, float m21 = 0.f, float m22 = 0.f, float m30 = 0.f, float m31 = 0.f, float m32 = 0.f)
        : _v(float3(m00, m01, m02), float3(m10, m11, m12), float3(m20, m21, m22), float3(m30, m31, m32)) {}

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 3>, 4> _v;
};

template<>
struct [[builtin("matrix")]] matrix<4, 4> {
    using ThisType = matrix<4, 4>;
    using T = float;
    static constexpr uint32 X = 4;
    static constexpr uint32 Y = 4;

    constexpr matrix(float4 col0, float4 col1, float4 col2, float4 col3)
        : _v(col0, col1, col2, col3) {}
    constexpr matrix(float m00 = 0.f, float m01 = 0.f, float m02 = 0.f, float m03 = 0.f, float m10 = 0.f, float m11 = 0.f, float m12 = 0.f, float m13 = 0.f, float m20 = 0.f, float m21 = 0.f, float m22 = 0.f, float m23 = 0.f, float m30 = 0.f, float m31 = 0.f, float m32 = 0.f, float m33 = 0.f)
        : _v(float4(m00, m01, m02, m03), float4(m10, m11, m12, m13), float4(m20, m21, m22, m23), float4(m30, m31, m32, m33)) {}

    static constexpr matrix<4, 4> identity() {
        // clang-format off
        return matrix<4, 4>(
            1.f, 0.f, 0.f, 0.f,  // col 0
            0.f, 1.f, 0.f, 0.f,  // col 1
            0.f, 0.f, 1.f, 0.f,  // col 2
            0.f, 0.f, 0.f, 1.f  // col 3
        );
        // clang-format on
    }

#include "ops/mat_ops.inl"

private:
    // DONT EDIT THIS FIELD LAYOUT
    Array<vec<float, 4>, 4> _v;
};

using float2x2 = matrix<2, 2>;
using float2x3 = matrix<2, 3>;
using float2x4 = matrix<2, 4>;

using float3x2 = matrix<3, 2>;
using float3x3 = matrix<3, 3>;
using float3x4 = matrix<3, 4>;

using float4x2 = matrix<4, 2>;
using float4x3 = matrix<4, 3>;
using float4x4 = matrix<4, 4>;

[[binop("MUL")]] float2 mul(float2 v, float2x2 m);
[[binop("MUL")]] float3 mul(float3 v, float3x3 m);
[[binop("MUL")]] float4 mul(float4 v, float4x4 m);

[[binop("MUL")]] float2 mul(float2x2 m, float2 v);
[[binop("MUL")]] float3 mul(float3x3 m, float3 v);
[[binop("MUL")]] float4 mul(float4x4 m, float4 v);

[[binop("MUL")]] float2x2 mul(float2x2 m1, float2x2 m2);
[[binop("MUL")]] float3x3 mul(float3x3 m1, float3x3 m2);
[[binop("MUL")]] float4x4 mul(float4x4 m1, float4x4 m2);
