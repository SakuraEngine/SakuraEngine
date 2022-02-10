#pragma once
#include "platform/configure.h"
#include "containers/span.hpp"

namespace sakura
{
namespace math
{

template <typename T, size_t R, size_t L>
struct Matrix {
public:
    sakura::span<T, R * L> data_view()
    {
        return M2;
    }
    const sakura::span<const T, R * L> data_view() const
    {
        return M2;
    }

protected:
    union
    {
        float M[R][L];
        float M2[R * L];
    };
};

template <>
struct alignas(16) Matrix<float, 4, 4> {
    FORCEINLINE Matrix() = default;
    FORCEINLINE Matrix(const float* data);

    sakura::span<float, 16> data_view()
    {
        return M16;
    }
    const sakura::span<const float, 16> data_view() const
    {
        return M16;
    }

    union
    {
        alignas(16) float M[4][4] = {
            { 1.f, 0.f, 0.f, 0.f },
            { 0.f, 1.f, 0.f, 0.f },
            { 0.f, 0.f, 1.f, 0.f },
            { 0.f, 0.f, 0.f, 1.f }
        };
        alignas(16) float M16[16];
    };
};

template <>
struct alignas(16) Matrix<float, 2, 2> {
    FORCEINLINE Matrix() = default;
    FORCEINLINE Matrix(const float* data);

    sakura::span<float, 4> data_view()
    {
        return M4;
    }
    const sakura::span<const float, 4> data_view() const
    {
        return M4;
    }

    union
    {
        alignas(16) float M[2][2] = {
            { 1.f, 0.f },
            { 0.f, 1.f }
        };
        alignas(16) float M4[4];
    };
};
using Matrix4x4 = Matrix<float, 4, 4>;
using Matrix2x2 = Matrix<float, 2, 2>;
using float4x4 = Matrix4x4;
using float2x2 = Matrix2x2;
static_assert(16 == alignof(float4x4), "matrix: alignas error.");
static_assert(sizeof(float4x4) == sizeof(float) * 16, "matrix: size error.");

FORCEINLINE Matrix<float, 4, 4>::Matrix(const float* data)
{
    M[0][0] = data[0 * 4 + 0];
    M[1][0] = data[1 * 4 + 0];
    M[2][0] = data[2 * 4 + 0];
    M[3][0] = data[3 * 4 + 0];
    M[0][1] = data[0 * 4 + 1];
    M[1][1] = data[1 * 4 + 1];
    M[2][1] = data[2 * 4 + 1];
    M[3][1] = data[3 * 4 + 1];
    M[0][2] = data[0 * 4 + 2];
    M[1][2] = data[1 * 4 + 2];
    M[2][2] = data[2 * 4 + 2];
    M[3][2] = data[3 * 4 + 2];
    M[0][3] = data[0 * 4 + 3];
    M[1][3] = data[1 * 4 + 3];
    M[2][3] = data[2 * 4 + 3];
    M[3][3] = data[3 * 4 + 3];
}

FORCEINLINE Matrix<float, 2, 2>::Matrix(const float* data)
{
    M[0][0] = data[0 * 2 + 0];
    M[1][0] = data[1 * 2 + 0];
    M[0][1] = data[0 * 2 + 1];
    M[1][1] = data[1 * 2 + 1];
}

} // namespace math
} // namespace sakura