#pragma once
#include "./rtm_conv.hpp"
#include "../gen/misc/quat.hpp"

namespace skr
{
inline namespace math
{
// matrix3x3 mul operator
inline float3x3 operator*(const float3x3& lhs, const float3x3& rhs)
{
    const auto rtm_lhs    = RtmConvert<float3x3>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<float3x3>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    return {
        RtmConvert<float3>::from_rtm(rtm_result.x_axis),
        RtmConvert<float3>::from_rtm(rtm_result.y_axis),
        RtmConvert<float3>::from_rtm(rtm_result.z_axis)
    };
}
inline float3x3& float3x3::operator*=(const float3x3& rhs)
{
    const auto rtm_lhs    = RtmConvert<float3x3>::to_rtm(*this);
    const auto rtm_rhs    = RtmConvert<float3x3>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    RtmConvert<float3>::store(rtm_result.x_axis, axis_x);
    RtmConvert<float3>::store(rtm_result.y_axis, axis_y);
    RtmConvert<float3>::store(rtm_result.z_axis, axis_z);
    return *this;
}
inline float3 operator*(const float3& lhs, const float3x3& rhs)
{
    const auto rtm_lhs    = RtmConvert<float3>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<float3x3>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul_vector3(rtm_lhs, rtm_rhs);
    return RtmConvert<float3>::from_rtm(rtm_result);
}
inline double3x3 operator*(const double3x3& lhs, const double3x3& rhs)
{
    const auto rtm_lhs    = RtmConvert<double3x3>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<double3x3>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    return {
        RtmConvert<double3>::from_rtm(rtm_result.x_axis),
        RtmConvert<double3>::from_rtm(rtm_result.y_axis),
        RtmConvert<double3>::from_rtm(rtm_result.z_axis)
    };
}
inline double3x3& double3x3::operator*=(const double3x3& rhs)
{
    const auto rtm_lhs    = RtmConvert<double3x3>::to_rtm(*this);
    const auto rtm_rhs    = RtmConvert<double3x3>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    RtmConvert<double3>::store(rtm_result.x_axis, axis_x);
    RtmConvert<double3>::store(rtm_result.y_axis, axis_y);
    RtmConvert<double3>::store(rtm_result.z_axis, axis_z);
    return *this;
}
inline double3 operator*(const double3& lhs, const double3x3& rhs)
{
    const auto rtm_lhs    = RtmConvert<double3>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<double3x3>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul_vector3(rtm_lhs, rtm_rhs);
    return RtmConvert<double3>::from_rtm(rtm_result);
}

// matrix4x4 mul operator
inline float4x4 operator*(const float4x4& lhs, const float4x4& rhs)
{
    const auto rtm_lhs    = RtmConvert<float4x4>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<float4x4>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    return {
        RtmConvert<float4>::from_rtm(rtm_result.x_axis),
        RtmConvert<float4>::from_rtm(rtm_result.y_axis),
        RtmConvert<float4>::from_rtm(rtm_result.z_axis),
        RtmConvert<float4>::from_rtm(rtm_result.w_axis)
    };
}
inline float4x4& float4x4::operator*=(const float4x4& rhs)
{
    const auto rtm_lhs    = RtmConvert<float4x4>::to_rtm(*this);
    const auto rtm_rhs    = RtmConvert<float4x4>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    RtmConvert<float4>::store(rtm_result.x_axis, axis_x);
    RtmConvert<float4>::store(rtm_result.y_axis, axis_y);
    RtmConvert<float4>::store(rtm_result.z_axis, axis_z);
    RtmConvert<float4>::store(rtm_result.w_axis, axis_w);
    return *this;
}
inline float4 operator*(const float4& lhs, const float4x4& rhs)
{
    const auto rtm_lhs    = RtmConvert<float4>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<float4x4>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul_vector(rtm_lhs, rtm_rhs);
    return RtmConvert<float4>::from_rtm(rtm_result);
}
inline double4x4 operator*(const double4x4& lhs, const double4x4& rhs)
{
    const auto rtm_lhs    = RtmConvert<double4x4>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<double4x4>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    return {
        RtmConvert<double4>::from_rtm(rtm_result.x_axis),
        RtmConvert<double4>::from_rtm(rtm_result.y_axis),
        RtmConvert<double4>::from_rtm(rtm_result.z_axis),
        RtmConvert<double4>::from_rtm(rtm_result.w_axis)
    };
}
inline double4x4& double4x4::operator*=(const double4x4& rhs)
{
    const auto rtm_lhs    = RtmConvert<double4x4>::to_rtm(*this);
    const auto rtm_rhs    = RtmConvert<double4x4>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul(rtm_lhs, rtm_rhs);
    RtmConvert<double4>::store(rtm_result.x_axis, axis_x);
    RtmConvert<double4>::store(rtm_result.y_axis, axis_y);
    RtmConvert<double4>::store(rtm_result.z_axis, axis_z);
    RtmConvert<double4>::store(rtm_result.w_axis, axis_w);
    return *this;
}
inline double4 operator*(const double4& lhs, const double4x4& rhs)
{
    const auto rtm_lhs    = RtmConvert<double4>::to_rtm(lhs);
    const auto rtm_rhs    = RtmConvert<double4x4>::to_rtm(rhs);
    const auto rtm_result = rtm::matrix_mul_vector(rtm_lhs, rtm_rhs);
    return RtmConvert<double4>::from_rtm(rtm_result);
}

// matrix3x3 mul free function
inline float3x3 mul(const float3x3& lhs, const float3x3& rhs)
{
    return lhs * rhs;
}
inline float3 mul(const float3& lhs, const float3x3& rhs)
{
    return lhs * rhs;
}
inline double3x3 mul(const double3x3& lhs, const double3x3& rhs)
{
    return lhs * rhs;
}
inline double3 mul(const double3& lhs, const double3x3& rhs)
{
    return lhs * rhs;
}

// matrix4x4 mul free function
inline float4x4 mul(const float4x4& lhs, const float4x4& rhs)
{
    return lhs * rhs;
}
inline float4 mul(const float4& lhs, const float4x4& rhs)
{
    return lhs * rhs;
}
inline double4x4 mul(const double4x4& lhs, const double4x4& rhs)
{
    return lhs * rhs;
}
inline double4 mul(const double4& lhs, const double4x4& rhs)
{
    return lhs * rhs;
}

// matrix4x4 mul point
inline float3 mul_point(const float3& lhs, const float4x4& rhs)
{
    return RtmConvert<float3>::from_rtm(
        rtm::matrix_mul_point3(
            RtmConvert<float3>::to_rtm(lhs),
            rtm::matrix_cast(
                RtmConvert<float4x4>::to_rtm(rhs)
            )
        )
    );
}
inline double3 mul_point(const double3& lhs, const double4x4& rhs)
{
    return RtmConvert<double3>::from_rtm(
        rtm::matrix_mul_point3(
            RtmConvert<double3>::to_rtm(lhs),
            rtm::matrix_cast(
                RtmConvert<double4x4>::to_rtm(rhs)
            )
        )
    );
}

// matrix4x4 mul vector
inline float3 mul_vector(const float3& lhs, const float4x4& rhs)
{
    return RtmConvert<float3>::from_rtm(
        rtm::matrix_mul_vector(
            RtmConvert<float3>::to_rtm(lhs),
            rtm::matrix_cast(
                RtmConvert<float4x4>::to_rtm(rhs)
            )
        )
    );
}
inline double3 mul_vector(const double3& lhs, const double4x4& rhs)
{
    return RtmConvert<double3>::from_rtm(
        rtm::matrix_mul_vector(
            RtmConvert<double3>::to_rtm(lhs),
            rtm::matrix_cast(
                RtmConvert<double4x4>::to_rtm(rhs)
            )
        )
    );
}

// equal operator
inline bool operator==(const float3x3& lhs, const float3x3& rhs)
{
    return all(lhs.axis_x == rhs.axis_x) &&
        all(lhs.axis_y == rhs.axis_y) &&
        all(lhs.axis_z == rhs.axis_z);
}
inline bool operator==(const double3x3& lhs, const double3x3& rhs)
{
    return all(lhs.axis_x == rhs.axis_x) &&
        all(lhs.axis_y == rhs.axis_y) &&
        all(lhs.axis_z == rhs.axis_z);
}
inline bool operator==(const float4x4& lhs, const float4x4& rhs)
{
    return all(lhs.axis_x == rhs.axis_x) &&
        all(lhs.axis_y == rhs.axis_y) &&
        all(lhs.axis_z == rhs.axis_z) &&
        all(lhs.axis_w == rhs.axis_w);
}
inline bool operator==(const double4x4& lhs, const double4x4& rhs)
{
    return all(lhs.axis_x == rhs.axis_x) &&
        all(lhs.axis_y == rhs.axis_y) &&
        all(lhs.axis_z == rhs.axis_z) &&
        all(lhs.axis_w == rhs.axis_w);
}

// nearly equal function
inline bool nearly_equal(const float3x3& lhs, const float3x3& rhs, float threshold = 1.e-4)
{
    return all(nearly_equal(lhs.axis_x, rhs.axis_x, threshold)) &&
        all(nearly_equal(lhs.axis_y, rhs.axis_y, threshold)) &&
        all(nearly_equal(lhs.axis_z, rhs.axis_z, threshold));
}
inline bool nearly_equal(const double3x3& lhs, const double3x3& rhs, double threshold = 1.e-4)
{
    return all(nearly_equal(lhs.axis_x, rhs.axis_x, threshold)) &&
        all(nearly_equal(lhs.axis_y, rhs.axis_y, threshold)) &&
        all(nearly_equal(lhs.axis_z, rhs.axis_z, threshold));
}
inline bool nearly_equal(const float4x4& lhs, const float4x4& rhs, float threshold = 1.e-4)
{
    return all(nearly_equal(lhs.axis_x, rhs.axis_x, threshold)) &&
        all(nearly_equal(lhs.axis_y, rhs.axis_y, threshold)) &&
        all(nearly_equal(lhs.axis_z, rhs.axis_z, threshold)) &&
        all(nearly_equal(lhs.axis_w, rhs.axis_w, threshold));
}
inline bool nearly_equal(const double4x4& lhs, const double4x4& rhs, double threshold = 1.e-4)
{
    return all(nearly_equal(lhs.axis_x, rhs.axis_x, threshold)) &&
        all(nearly_equal(lhs.axis_y, rhs.axis_y, threshold)) &&
        all(nearly_equal(lhs.axis_z, rhs.axis_z, threshold)) &&
        all(nearly_equal(lhs.axis_w, rhs.axis_w, threshold));
}

// matrix3x3 transpose
inline float3x3 transpose(const float3x3& m)
{
    const auto rtm_mtx    = RtmConvert<float3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_transpose(rtm_mtx);
    return RtmConvert<float3x3>::from_rtm(rtm_result);
}
inline double3x3 transpose(const double3x3& m)
{
    const auto rtm_mtx    = RtmConvert<double3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_transpose(rtm_mtx);
    return RtmConvert<double3x3>::from_rtm(rtm_result);
}

// matrix4x4 transpose
inline float4x4 transpose(const float4x4& m)
{
    const auto rtm_mtx    = RtmConvert<float4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_transpose(rtm_mtx);
    return RtmConvert<float4x4>::from_rtm(rtm_result);
}
inline double4x4 transpose(const double4x4& m)
{
    const auto rtm_mtx    = RtmConvert<double4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_transpose(rtm_mtx);
    return RtmConvert<double4x4>::from_rtm(rtm_result);
}

// matrix3x3 inverse
inline float3x3 inverse(const float3x3& mtx)
{
    const auto rtm_mtx    = RtmConvert<float3x3>::to_rtm(mtx);
    const auto rtm_result = rtm::matrix_inverse(rtm_mtx);
    return RtmConvert<float3x3>::from_rtm(rtm_result);
}
inline float3x3 inverse(const float3x3& mtx, const float3x3& fallback)
{
    const auto rtm_mtx      = RtmConvert<float3x3>::to_rtm(mtx);
    const auto rtm_fallback = RtmConvert<float3x3>::to_rtm(fallback);
    const auto rtm_result   = rtm::matrix_inverse(rtm_mtx, rtm_fallback);
    return RtmConvert<float3x3>::from_rtm(rtm_result);
}
inline double3x3 inverse(const double3x3& mtx)
{
    const auto rtm_mtx    = RtmConvert<double3x3>::to_rtm(mtx);
    const auto rtm_result = rtm::matrix_inverse(rtm_mtx);
    return RtmConvert<double3x3>::from_rtm(rtm_result);
}
inline double3x3 inverse(const double3x3& mtx, const double3x3& fallback)
{
    const auto rtm_mtx      = RtmConvert<double3x3>::to_rtm(mtx);
    const auto rtm_fallback = RtmConvert<double3x3>::to_rtm(fallback);
    const auto rtm_result   = rtm::matrix_inverse(rtm_mtx, rtm_fallback);
    return RtmConvert<double3x3>::from_rtm(rtm_result);
}

// matrix4x4 inverse
inline float4x4 inverse(const float4x4& mtx)
{
    const auto rtm_matrix = RtmConvert<float4x4>::to_rtm(mtx);
    const auto rtm_result = rtm::matrix_inverse(rtm_matrix);
    return RtmConvert<float4x4>::from_rtm(rtm_result);
}
inline float4x4 inverse(const float4x4& mtx, const float4x4& fallback)
{
    const auto rtm_matrix   = RtmConvert<float4x4>::to_rtm(mtx);
    const auto rtm_fallback = RtmConvert<float4x4>::to_rtm(fallback);
    const auto rtm_result   = rtm::matrix_inverse(rtm_matrix, rtm_fallback);
    return RtmConvert<float4x4>::from_rtm(rtm_result);
}
inline double4x4 inverse(const double4x4& mtx)
{
    const auto rtm_matrix = RtmConvert<double4x4>::to_rtm(mtx);
    const auto rtm_result = rtm::matrix_inverse(rtm_matrix);
    return RtmConvert<double4x4>::from_rtm(rtm_result);
}
inline double4x4 inverse(const double4x4& mtx, const double4x4& fallback)
{
    const auto rtm_matrix   = RtmConvert<double4x4>::to_rtm(mtx);
    const auto rtm_fallback = RtmConvert<double4x4>::to_rtm(fallback);
    const auto rtm_result   = rtm::matrix_inverse(rtm_matrix, rtm_fallback);
    return RtmConvert<double4x4>::from_rtm(rtm_result);
}

// matrix3x3 determinant
inline float determinant(const float3x3& m)
{
    const auto rtm_matrix = RtmConvert<float3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_determinant(rtm_matrix);
    return rtm::scalar_cast(rtm_result);
}
inline double determinant(const double3x3& m)
{
    const auto rtm_matrix = RtmConvert<double3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_determinant(rtm_matrix);
    return rtm::scalar_cast(rtm_result);
}

// matrix4x4 determinant
inline float determinant(const float4x4& m)
{
    const auto rtm_matrix = RtmConvert<float4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_determinant(rtm_matrix);
    return rtm::scalar_cast(rtm_result);
}
inline double determinant(const double4x4& m)
{
    const auto rtm_matrix = RtmConvert<double4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_determinant(rtm_matrix);
    return rtm::scalar_cast(rtm_result);
}

// matrix3x3 minor
inline float minor(const float3x3& m, EAxis3 row, EAxis3 col)
{
    const auto rtm_matrix = RtmConvert<float3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_minor(
        rtm_matrix,
        static_cast<rtm::axis3>(row),
        static_cast<rtm::axis3>(col)
    );
    return rtm::scalar_cast(rtm_result);
}
inline double minor(const double3x3& m, EAxis3 row, EAxis3 col)
{
    const auto rtm_matrix = RtmConvert<double3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_minor(
        rtm_matrix,
        static_cast<rtm::axis3>(row),
        static_cast<rtm::axis3>(col)
    );
    return rtm::scalar_cast(rtm_result);
}

// matrix4x4 minor
inline float minor(const float4x4& m, EAxis4 row, EAxis4 col)
{
    const auto rtm_matrix = RtmConvert<float4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_minor(
        rtm_matrix,
        static_cast<rtm::axis4>(row),
        static_cast<rtm::axis4>(col)
    );
    return rtm::scalar_cast(rtm_result);
}
inline double minor(const double4x4& m, EAxis4 row, EAxis4 col)
{
    const auto rtm_matrix = RtmConvert<double4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_minor(
        rtm_matrix,
        static_cast<rtm::axis4>(row),
        static_cast<rtm::axis4>(col)
    );
    return rtm::scalar_cast(rtm_result);
}

// matrix3x3 cofactor
inline float3x3 cofactor(const float3x3& m)
{
    const auto rtm_matrix = RtmConvert<float3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_cofactor(rtm_matrix);
    return RtmConvert<float3x3>::from_rtm(rtm_result);
}
inline double3x3 cofactor(const double3x3& m)
{
    const auto rtm_matrix = RtmConvert<double3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_cofactor(rtm_matrix);
    return RtmConvert<double3x3>::from_rtm(rtm_result);
}

// matrix4x4 cofactor
inline float4x4 cofactor(const float4x4& m)
{
    const auto rtm_matrix = RtmConvert<float4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_cofactor(rtm_matrix);
    return RtmConvert<float4x4>::from_rtm(rtm_result);
}
inline double4x4 cofactor(const double4x4& m)
{
    const auto rtm_matrix = RtmConvert<double4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_cofactor(rtm_matrix);
    return RtmConvert<double4x4>::from_rtm(rtm_result);
}

// matrix3x3 adjugate
inline float3x3 adjugate(const float3x3& m)
{
    const auto rtm_matrix = RtmConvert<float3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_adjugate(rtm_matrix);
    return RtmConvert<float3x3>::from_rtm(rtm_result);
}
inline double3x3 adjugate(const double3x3& m)
{
    const auto rtm_matrix = RtmConvert<double3x3>::to_rtm(m);
    const auto rtm_result = rtm::matrix_adjugate(rtm_matrix);
    return RtmConvert<double3x3>::from_rtm(rtm_result);
}

// matrix4x4 adjugate
inline float4x4 adjugate(const float4x4& m)
{
    const auto rtm_matrix = RtmConvert<float4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_adjugate(rtm_matrix);
    return RtmConvert<float4x4>::from_rtm(rtm_result);
}
inline double4x4 adjugate(const double4x4& m)
{
    const auto rtm_matrix = RtmConvert<double4x4>::to_rtm(m);
    const auto rtm_result = rtm::matrix_adjugate(rtm_matrix);
    return RtmConvert<double4x4>::from_rtm(rtm_result);
}

// matrix3x3 relative
inline float3x3 relative(const float3x3& from, const float3x3& to)
{
    const auto rtm_from   = RtmConvert<float3x3>::to_rtm(from);
    const auto rtm_to     = RtmConvert<float3x3>::to_rtm(to);
    const auto rtm_result = rtm::matrix_mul(
        rtm_to,
        rtm::matrix_inverse(rtm_from)
    );
    return RtmConvert<float3x3>::from_rtm(rtm_result);
}
inline double3x3 relative(const double3x3& from, const double3x3& to)
{
    const auto rtm_from   = RtmConvert<double3x3>::to_rtm(from);
    const auto rtm_to     = RtmConvert<double3x3>::to_rtm(to);
    const auto rtm_result = rtm::matrix_mul(
        rtm_to,
        rtm::matrix_inverse(rtm_from)
    );
    return RtmConvert<double3x3>::from_rtm(rtm_result);
}

// matrix4x4 relative
inline float4x4 relative(const float4x4& parent, const float4x4& world)
{
    const auto rtm_parent = RtmConvert<float4x4>::to_rtm(parent);
    const auto rtm_world  = RtmConvert<float4x4>::to_rtm(world);
    const auto rtm_result = rtm::matrix_mul(
        rtm_world,
        rtm::matrix_inverse(rtm_parent)
    );
    return RtmConvert<float4x4>::from_rtm(rtm_result);
}
inline double4x4 relative(const double4x4& parent, const double4x4& world)
{
    const auto rtm_parent = RtmConvert<double4x4>::to_rtm(parent);
    const auto rtm_world  = RtmConvert<double4x4>::to_rtm(world);
    const auto rtm_result = rtm::matrix_mul(
        rtm_world,
        rtm::matrix_inverse(rtm_parent)
    );
    return RtmConvert<double4x4>::from_rtm(rtm_result);
}
} // namespace math
} // namespace skr