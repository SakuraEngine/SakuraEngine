#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrRT/math/rtm/matrix4x4f.h"
#include "SkrGui/math/geometry.hpp"
namespace skr::gui
{
// column major matrix
struct SKR_ALIGNAS(16) Matrix4 {
    // factory
    static Matrix4 Identity() SKR_NOEXCEPT
    {
        return { rtm::matrix_identity() };
    }
    static Matrix4 Translate(float x, float y, float z) SKR_NOEXCEPT
    {
        return {
            {
            rtm::vector_set(1.f, 0.f, 0.f, 0.f),
            rtm::vector_set(0.f, 1.f, 0.f, 0.f),
            rtm::vector_set(0.f, 0.f, 1.f, 0.f),
            rtm::vector_set(x, y, z, 1.f),
            }
        };
    }

    // transform
    inline Offsetf transform(Offsetf pos) const SKR_NOEXCEPT
    {
        auto vec    = rtm::vector_set(pos.x, pos.y, 0.f, 1.f);
        auto result = rtm::matrix_mul_vector(vec, _m);
        return { rtm::vector_get_x(result), rtm::vector_get_y(result) };
    }
    inline Sizef transform(Sizef size) const SKR_NOEXCEPT
    {
        auto vec    = rtm::vector_set(size.width, size.height, 0.f, 0.f);
        auto result = rtm::matrix_mul_vector(vec, _m);
        return { rtm::vector_get_x(result), rtm::vector_get_y(result) };
    }

    // invert
    inline bool try_inverse(Matrix4& result)
    {
        static rtm::matrix4x4f zero_matrix = {
            rtm::vector_set(0.f),
            rtm::vector_set(0.f),
            rtm::vector_set(0.f),
            rtm::vector_set(0.f),
        };
        static auto zero_vector = rtm::vector_set(0.f);

        result = { rtm::matrix_inverse(_m, zero_matrix) };

        return rtm::vector_all_equal(result._m.x_axis, zero_vector) &&
               rtm::vector_all_equal(result._m.y_axis, zero_vector) &&
               rtm::vector_all_equal(result._m.z_axis, zero_vector) &&
               rtm::vector_all_equal(result._m.w_axis, zero_vector);
    }

    // operators
    inline Matrix4 operator*(const Matrix4& rhs) const SKR_NOEXCEPT
    {
        return Matrix4(rtm::matrix_mul(_m, rhs._m));
    }
    inline Matrix4& operator*=(const Matrix4& rhs) SKR_NOEXCEPT
    {
        _m = rtm::matrix_mul(_m, rhs._m);
        return *this;
    }
    inline friend Sizef operator*(const Sizef& lhs, const Matrix4& rhs) SKR_NOEXCEPT
    {
        auto vec    = rtm::vector_set(lhs.width, lhs.height, 0.f, 0.f);
        auto result = rtm::matrix_mul_vector(vec, rhs._m);
        return { rtm::vector_get_x(result), rtm::vector_get_y(result) };
    }

    rtm::matrix4x4f _m;
};
} // namespace skr::gui