#pragma once
#include "./rtm_conv.hpp"
#include "../gen/misc/quat.hpp"
#include "./quat.hpp"

namespace skr
{
inline namespace math
{
// transform mul transform
inline TransformF mul(const TransformF& lhs, const TransformF& rhs)
{
    auto result = rtm::qvv_mul(
        RtmConvert<TransformF>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs)
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD mul(const TransformD& lhs, const TransformD& rhs)
{
    auto result = rtm::qvv_mul(
        RtmConvert<TransformD>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs)
    );
    return RtmConvert<TransformD>::from_rtm(result);
}
inline TransformF mul_no_scale(const TransformF& lhs, const TransformF& rhs)
{
    auto result = rtm::qvv_mul_no_scale(
        RtmConvert<TransformF>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs)
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD mul_no_scale(const TransformD& lhs, const TransformD& rhs)
{
    auto result = rtm::qvv_mul_no_scale(
        RtmConvert<TransformD>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs)
    );
    return RtmConvert<TransformD>::from_rtm(result);
}
inline TransformF operator*(const TransformF& lhs, const TransformF& rhs)
{
    return mul(lhs, rhs);
}
inline TransformD operator*(const TransformD& lhs, const TransformD& rhs)
{
    return mul(lhs, rhs);
}

// mul point
inline float3 mul_point(const float3& lhs, const TransformF& rhs)
{
    auto result = rtm::qvv_mul_point3(
        RtmConvert<float3>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs)
    );
    return RtmConvert<float3>::from_rtm(result);
}
inline double3 mul_point(const double3& lhs, const TransformD& rhs)
{
    auto result = rtm::qvv_mul_point3(
        RtmConvert<double3>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs)
    );
    return RtmConvert<double3>::from_rtm(result);
}

// mul point no scale
inline float3 mul_point_no_scale(const float3& lhs, const TransformF& rhs)
{
    auto result = rtm::qvv_mul_point3_no_scale(
        RtmConvert<float3>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs)
    );
    return RtmConvert<float3>::from_rtm(result);
}
inline double3 mul_point_no_scale(const double3& lhs, const TransformD& rhs)
{
    auto result = rtm::qvv_mul_point3_no_scale(
        RtmConvert<double3>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs)
    );
    return RtmConvert<double3>::from_rtm(result);
}

// mul vector
inline float3 mul_vector(const float3& lhs, const TransformF& rhs)
{
    auto rtm_lhs      = RtmConvert<float3>::to_rtm(lhs);
    auto rtm_rotation = RtmConvert<QuatF>::to_rtm(rhs.rotation);
    auto rtm_scale    = RtmConvert<float3>::to_rtm(rhs.scale);
    auto result       = rtm::quat_mul_vector3(
        rtm::vector_mul(rtm_scale, rtm_lhs),
        rtm_rotation
    );
    return RtmConvert<float3>::from_rtm(result);
}
inline double3 mul_vector(const double3& lhs, const TransformD& rhs)
{
    auto rtm_lhs      = RtmConvert<double3>::to_rtm(lhs);
    auto rtm_rotation = RtmConvert<QuatD>::to_rtm(rhs.rotation);
    auto rtm_scale    = RtmConvert<double3>::to_rtm(rhs.scale);
    auto result       = rtm::quat_mul_vector3(
        rtm::vector_mul(rtm_scale, rtm_lhs),
        rtm_rotation
    );
    return RtmConvert<double3>::from_rtm(result);
}

// mul vector4, the position apply by w component
inline float4 mul(const float4& lhs, const TransformF& rhs)
{
    auto rtm_lhs      = RtmConvert<float4>::to_rtm(lhs);
    auto rtm_rotation = RtmConvert<QuatF>::to_rtm(rhs.rotation);
    auto rtm_scale    = RtmConvert<float3>::to_rtm(rhs.scale);
    auto rtm_position = RtmConvert<float3>::to_rtm(rhs.position);
    auto result       = rtm::vector_add(
        rtm::quat_mul_vector3(
            rtm::vector_mul(rtm_scale, rtm_lhs),
            rtm_rotation
        ),
        rtm::vector_mul(rtm_position, rtm::vector_broadcast(&lhs.w))
    );
    return RtmConvert<float4>::from_rtm(result);
}
inline double4 mul(const double4& lhs, const TransformD& rhs)
{
    auto rtm_lhs      = RtmConvert<double4>::to_rtm(lhs);
    auto rtm_rotation = RtmConvert<QuatD>::to_rtm(rhs.rotation);
    auto rtm_scale    = RtmConvert<double3>::to_rtm(rhs.scale);
    auto rtm_position = RtmConvert<double3>::to_rtm(rhs.position);
    auto result       = rtm::vector_add(
        rtm::quat_mul_vector3(
            rtm::vector_mul(rtm_scale, rtm_lhs),
            rtm_rotation
        ),
        rtm::vector_mul(rtm_position, rtm::vector_broadcast(&lhs.w))
    );
    return RtmConvert<double4>::from_rtm(result);
}
inline float4 operator*(const float4& lhs, const TransformF& rhs)
{
    return mul(lhs, rhs);
}
inline double4 operator*(const double4& lhs, const TransformD& rhs)
{
    return mul(lhs, rhs);
}

// mul vector4 no scale, the position apply by w component
inline float4 mul_no_scale(float4 lhs, const TransformF& rhs)
{
    auto rtm_lhs      = RtmConvert<float4>::to_rtm(lhs);
    auto rtm_rotation = RtmConvert<QuatF>::to_rtm(rhs.rotation);
    auto rtm_position = RtmConvert<float3>::to_rtm(rhs.position);
    auto result       = rtm::vector_add(
        rtm::quat_mul_vector3(
            rtm_lhs,
            rtm_rotation
        ),
        rtm::vector_mul(rtm_position, rtm::vector_broadcast(&lhs.w))
    );
    return RtmConvert<float4>::from_rtm(result);
}
inline double4 mul_no_scale(double4 lhs, const TransformD& rhs)
{
    auto rtm_lhs      = RtmConvert<double4>::to_rtm(lhs);
    auto rtm_rotation = RtmConvert<QuatD>::to_rtm(rhs.rotation);
    auto rtm_position = RtmConvert<double3>::to_rtm(rhs.position);
    auto result       = rtm::vector_add(
        rtm::quat_mul_vector3(
            rtm_lhs,
            rtm_rotation
        ),
        rtm::vector_mul(rtm_position, rtm::vector_broadcast(&lhs.w))
    );
    return RtmConvert<double4>::from_rtm(result);
}

// mul vector no scale
inline float3 mul_vector_no_scale(const float3& lhs, const TransformF& rhs)
{
    return mul(lhs, rhs.rotation);
}
inline double3 mul_vector_no_scale(const double3& lhs, const TransformD& rhs)
{
    return mul(lhs, rhs.rotation);
}

// inverse
inline TransformF inverse(const TransformF& transform)
{
    auto result = rtm::qvv_inverse(
        RtmConvert<TransformF>::to_rtm(transform)
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformF inverse(const TransformF& transform, float3 fallback_scale = float3(1.0f), float threshold = float(0.00001f))
{
    auto result = rtm::qvv_inverse(
        RtmConvert<TransformF>::to_rtm(transform),
        RtmConvert<float3>::to_rtm(fallback_scale),
        threshold
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD inverse(const TransformD& transform)
{
    auto result = rtm::qvv_inverse(
        RtmConvert<TransformD>::to_rtm(transform)
    );
    return RtmConvert<TransformD>::from_rtm(result);
}
inline TransformD inverse(const TransformD& transform, double3 fallback_scale = double3(1.0), double threshold = double(0.00001))
{
    auto result = rtm::qvv_inverse(
        RtmConvert<TransformD>::to_rtm(transform),
        RtmConvert<double3>::to_rtm(fallback_scale),
        threshold
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// inverse no scale
inline TransformF inverse_no_scale(const TransformF& transform)
{
    auto result = rtm::qvv_inverse_no_scale(
        RtmConvert<TransformF>::to_rtm(transform)
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD inverse_no_scale(const TransformD& transform)
{
    auto result = rtm::qvv_inverse_no_scale(
        RtmConvert<TransformD>::to_rtm(transform)
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// normalize
inline TransformF normalize(const TransformF& transform)
{
    auto result = rtm::qvv_normalize(
        RtmConvert<TransformF>::to_rtm(transform)
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD normalize(const TransformD& transform)
{
    auto result = rtm::qvv_normalize(
        RtmConvert<TransformD>::to_rtm(transform)
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// lerp
inline TransformF lerp(const TransformF& lhs, const TransformF& rhs, float t)
{
    auto result = rtm::qvv_lerp(
        RtmConvert<TransformF>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD lerp(const TransformD& lhs, const TransformD& rhs, double t)
{
    auto result = rtm::qvv_lerp(
        RtmConvert<TransformD>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// lerp no scale
inline TransformF lerp_no_scale(const TransformF& lhs, const TransformF& rhs, float t)
{
    auto result = rtm::qvv_lerp_no_scale(
        RtmConvert<TransformF>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD lerp_no_scale(const TransformD& lhs, const TransformD& rhs, double t)
{
    auto result = rtm::qvv_lerp_no_scale(
        RtmConvert<TransformD>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// slerp
inline TransformF slerp(const TransformF& lhs, const TransformF& rhs, float t)
{
    auto result = rtm::qvv_slerp(
        RtmConvert<TransformF>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD slerp(const TransformD& lhs, const TransformD& rhs, double t)
{
    auto result = rtm::qvv_slerp(
        RtmConvert<TransformD>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// slerp no scale
inline TransformF slerp_no_scale(const TransformF& lhs, const TransformF& rhs, float t)
{
    auto result = rtm::qvv_slerp_no_scale(
        RtmConvert<TransformF>::to_rtm(lhs),
        RtmConvert<TransformF>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformF>::from_rtm(result);
}
inline TransformD slerp_no_scale(const TransformD& lhs, const TransformD& rhs, double t)
{
    auto result = rtm::qvv_slerp_no_scale(
        RtmConvert<TransformD>::to_rtm(lhs),
        RtmConvert<TransformD>::to_rtm(rhs),
        t
    );
    return RtmConvert<TransformD>::from_rtm(result);
}

// is finite
inline bool is_finite(const TransformF& transform)
{
    return rtm::qvv_is_finite(
        RtmConvert<TransformF>::to_rtm(transform)
    );
}
inline bool is_finite(const TransformD& transform)
{
    return rtm::qvv_is_finite(
        RtmConvert<TransformD>::to_rtm(transform)
    );
}

// equal
inline bool operator==(const TransformF& lhs, const TransformF& rhs)
{
    return all(lhs.rotation.as_vector() == rhs.rotation.as_vector()) &&
        all(lhs.position == rhs.position) &&
        all(lhs.scale == rhs.scale);
}
inline bool operator==(const TransformD& lhs, const TransformD& rhs)
{
    return all(lhs.rotation.as_vector() == rhs.rotation.as_vector()) &&
        all(lhs.position == rhs.position) &&
        all(lhs.scale == rhs.scale);
}

// nearly equal
inline bool nearly_equal(const TransformF& lhs, const TransformF& rhs, float threshold = float(0.00001))
{
    return all(nearly_equal(lhs.rotation, rhs.rotation, threshold)) &&
        all(nearly_equal(lhs.position, rhs.position, threshold)) &&
        all(nearly_equal(lhs.scale, rhs.scale, threshold));
}
inline bool nearly_equal(const TransformD& lhs, const TransformD& rhs, double threshold = double(0.00001))
{
    return all(nearly_equal(lhs.rotation, rhs.rotation, threshold)) &&
        all(nearly_equal(lhs.position, rhs.position, threshold)) &&
        all(nearly_equal(lhs.scale, rhs.scale, threshold));
}

// mul assign operator
inline TransformF& TransformF::operator*=(const TransformF& rhs)
{
    auto result = rtm::qvv_mul(
        RtmConvert<TransformF>::to_rtm(*this),
        RtmConvert<TransformF>::to_rtm(rhs)
    );
    RtmConvert<TransformF>::store(result, *this);
    return *this;
}
inline TransformD& TransformD::operator*=(const TransformD& rhs)
{
    auto result = rtm::qvv_mul(
        RtmConvert<TransformD>::to_rtm(*this),
        RtmConvert<TransformD>::to_rtm(rhs)
    );
    RtmConvert<TransformD>::store(result, *this);
    return *this;
}

// is identity
inline bool TransformF::is_identity() const
{
    return rotation.is_identity() &&
        all(position == float3(0)) &&
        all(scale == float3(1));
}
inline bool TransformD::is_identity() const
{
    return rotation.is_identity() &&
        all(position == double3(0)) &&
        all(scale == double3(1));
}

// is nearly identity
inline bool TransformF::is_nearly_identity(float threshold) const
{
    return rotation.is_nearly_identity(threshold) &&
        all(nearly_equal(position, float3(0), threshold)) &&
        all(nearly_equal(scale, float3(1), threshold));
}
inline bool TransformD::is_nearly_identity(double threshold) const
{
    return rotation.is_nearly_identity(threshold) &&
        all(nearly_equal(position, double3(0), threshold)) &&
        all(nearly_equal(scale, double3(1), threshold));
}

// to matrix
inline TransformF::operator float4x4() const
{
    return to_matrix();
}
inline float4x4 TransformF::to_matrix() const
{
    return RtmConvert<float4x4>::from_rtm(
        rtm::matrix_cast(
            rtm::matrix_from_qvv(
                RtmConvert<TransformF>::to_rtm(*this)
            )
        )
    );
}
inline float4x4 TransformF::to_matrix_no_scale() const
{
    return RtmConvert<float4x4>::from_rtm(
        rtm::matrix_cast(
            rtm::matrix_from_qvv(
                RtmConvert<QuatF>::to_rtm(rotation),
                RtmConvert<float3>::to_rtm(position),
                rtm::vector_set(1.f, 1.f, 1.f, 0.f)
            )
        )
    );
}
inline TransformD::operator double4x4() const
{
    return to_matrix();
}
inline double4x4 TransformD::to_matrix() const
{
    return RtmConvert<double4x4>::from_rtm(
        rtm::matrix_cast(
            rtm::matrix_from_qvv(
                RtmConvert<TransformD>::to_rtm(*this)
            )
        )
    );
}
inline double4x4 TransformD::to_matrix_no_scale() const
{
    return RtmConvert<double4x4>::from_rtm(
        rtm::matrix_cast(
            rtm::matrix_from_qvv(
                RtmConvert<QuatD>::to_rtm(rotation),
                RtmConvert<double3>::to_rtm(position),
                rtm::vector_set(1.0, 1.0, 1.0, 0.0)
            )
        )
    );
}

// from matrix ctor
inline TransformF::TransformF(const float3x3& mat)
{
    auto result = rtm::qvv_from_matrix(
        RtmConvert<float3x3>::to_rtm(mat)
    );
    RtmConvert<QuatF>::store(result.rotation, rotation);
    RtmConvert<float3>::store(result.translation, position);
    RtmConvert<float3>::store(result.scale, scale);
}
inline TransformF::TransformF(const float4x4& mat)
{
    auto result = rtm::qvv_from_matrix(
        (rtm::matrix3x4f)rtm::matrix_cast(
            RtmConvert<float4x4>::to_rtm(mat)
        )
    );
    RtmConvert<QuatF>::store(result.rotation, rotation);
    RtmConvert<float3>::store(result.translation, position);
    RtmConvert<float3>::store(result.scale, scale);
}
inline TransformD::TransformD(const double3x3& mat)
{
    auto result = rtm::qvv_from_matrix(
        RtmConvert<double3x3>::to_rtm(mat)
    );
    rotation = RtmConvert<QuatD>::from_rtm(result.rotation);
    position = RtmConvert<double3>::from_rtm(result.translation);
    scale    = RtmConvert<double3>::from_rtm(result.scale);
}
inline TransformD::TransformD(const double4x4& mat)
{
    auto result = rtm::qvv_from_matrix(
        (rtm::matrix3x4d)rtm::matrix_cast(
            RtmConvert<double4x4>::to_rtm(mat)
        )
    );
    rotation = RtmConvert<QuatD>::from_rtm(result.rotation);
    position = RtmConvert<double3>::from_rtm(result.translation);
    scale    = RtmConvert<double3>::from_rtm(result.scale);
}

// from matrix factory
inline TransformF TransformF::FromMatrix(const float3x3& mat)
{
    return TransformF(mat);
}
inline TransformF TransformF::FromMatrix(const float4x4& mat)
{
    return TransformF(mat);
}
inline TransformD TransformD::FromMatrix(const double3x3& mat)
{
    return TransformD(mat);
}
inline TransformD TransformD::FromMatrix(const double4x4& mat)
{
    return TransformD(mat);
}

// directions
inline float3 TransformF::right() const
{
    return mul_vector_no_scale(float3::right(), *this);
}
inline float3 TransformF::up() const
{
    return mul_vector_no_scale(float3::up(), *this);
}
inline float3 TransformF::forward() const
{
    return mul_vector_no_scale(float3::forward(), *this);
}
inline float3 TransformF::left() const
{
    return mul_vector_no_scale(float3::left(), *this);
}
inline float3 TransformF::down() const
{
    return mul_vector_no_scale(float3::down(), *this);
}
inline float3 TransformF::back() const
{
    return mul_vector_no_scale(float3::back(), *this);
}
inline double3 TransformD::right() const
{
    return mul_vector_no_scale(double3::right(), *this);
}
inline double3 TransformD::up() const
{
    return mul_vector_no_scale(double3::up(), *this);
}
inline double3 TransformD::forward() const
{
    return mul_vector_no_scale(double3::forward(), *this);
}
inline double3 TransformD::left() const
{
    return mul_vector_no_scale(double3::left(), *this);
}
inline double3 TransformD::down() const
{
    return mul_vector_no_scale(double3::down(), *this);
}
inline double3 TransformD::back() const
{
    return mul_vector_no_scale(double3::back(), *this);
}

// relative
inline TransformF relative(const TransformF& parent, const TransformF& world)
{
    auto      rtm_parent = RtmConvert<TransformF>::to_rtm(parent);
    auto      rtm_world  = RtmConvert<TransformF>::to_rtm(world);
    rtm::qvvf rtm_result;

    auto _safe_rcp = +[](rtm::vector4f v) {
        auto rcp = rtm::vector_reciprocal(v);
        return rtm::vector_select(
            rtm::vector_greater_than(rtm::vector_abs(v), rtm::vector_set(1.e-8f)),
            rcp,
            rtm::vector_zero()
        );
    };

    if (rtm::vector_any_less_than3(
            rtm::vector_min(rtm_parent.scale, rtm_world.scale),
            rtm::vector_zero()
        ))
    { // any neg scale, use matrix
        auto parent_mat       = rtm::matrix_from_qvv(rtm_parent);
        auto world_mat        = rtm::matrix_from_qvv(rtm_world);
        auto parent_scale_rcp = _safe_rcp(rtm_parent.scale);

        // get scale
        auto result_scale = rtm::vector_mul(
            rtm_world.scale,
            parent_scale_rcp
        );
        rtm_result.scale = result_scale;

        auto result_mat = rtm::matrix_mul(
            world_mat,
            rtm::matrix_inverse(parent_mat)
        );

        // get translation
        rtm_result.translation = result_mat.w_axis;

        // get rotation
        result_mat        = rtm::matrix_remove_scale(result_mat);
        auto scale_sign   = rtm::vector_sign(result_scale);
        result_mat.x_axis = rtm::vector_mul(
            rtm::vector_dup_x(scale_sign),
            result_mat.x_axis
        );
        result_mat.y_axis = rtm::vector_mul(
            rtm::vector_dup_y(scale_sign),
            result_mat.y_axis
        );
        result_mat.z_axis = rtm::vector_mul(
            rtm::vector_dup_z(scale_sign),
            result_mat.z_axis
        );
        rtm_result.rotation = rtm::quat_from_matrix(result_mat);
    }
    else
    {
        // scale = S(world) / S(parent)
        auto parent_scale_rcp = _safe_rcp(rtm_parent.scale);
        rtm_result.scale      = rtm::vector_mul(rtm_world.scale, parent_scale_rcp);

        // -R(parent)
        auto parent_rotation_inv = rtm::quat_conjugate(rtm_parent.rotation);

        // translation = (T(world) - T(parent)) * -Q(parent) / S(parent)
        auto direct_translation = rtm::vector_sub(
            rtm_world.translation,
            rtm_parent.translation
        );
        auto rotated_translation = rtm::quat_mul_vector3(
            direct_translation,
            parent_rotation_inv
        );
        rtm_result.translation = rtm::vector_mul(
            rotated_translation,
            parent_scale_rcp
        );

        // rotation = Q(world) * -Q(parent)
        rtm_result.rotation = rtm::quat_mul(
            rtm_world.rotation,
            parent_rotation_inv
        );
    }
    return RtmConvert<TransformF>::from_rtm(rtm_result);
}
inline TransformD relative(const TransformD& parent, const TransformD& world)
{
    auto      rtm_parent = RtmConvert<TransformD>::to_rtm(parent);
    auto      rtm_world  = RtmConvert<TransformD>::to_rtm(world);
    rtm::qvvd rtm_result;

    auto _safe_rcp = +[](rtm::vector4d v) {
        auto rcp = rtm::vector_reciprocal(v);
        return rtm::vector_select(
            rtm::vector_greater_than(rtm::vector_abs(v), rtm::vector_set(1.e-8)),
            rcp,
            rtm::vector_zero()
        );
    };

    if (rtm::vector_any_less_than3(
            rtm::vector_min(rtm_parent.scale, rtm_world.scale),
            rtm::vector_zero()
        ))
    { // any neg scale, use matrix
        auto parent_mat       = rtm::matrix_from_qvv(rtm_parent);
        auto world_mat        = rtm::matrix_from_qvv(rtm_world);
        auto parent_scale_rcp = _safe_rcp(rtm_parent.scale);

        // get scale
        auto result_scale = rtm::vector_mul(
            rtm_world.scale,
            parent_scale_rcp
        );
        rtm_result.scale = result_scale;

        auto result_mat = rtm::matrix_mul(
            world_mat,
            rtm::matrix_inverse(parent_mat)
        );

        // get translation
        rtm_result.translation = result_mat.w_axis;

        // get rotation
        result_mat        = rtm::matrix_remove_scale(result_mat);
        auto scale_sign   = rtm::vector_sign(result_scale);
        result_mat.x_axis = rtm::vector_mul(
            rtm::vector_dup_x(scale_sign),
            result_mat.x_axis
        );
        result_mat.y_axis = rtm::vector_mul(
            rtm::vector_dup_y(scale_sign),
            result_mat.y_axis
        );
        result_mat.z_axis = rtm::vector_mul(
            rtm::vector_dup_z(scale_sign),
            result_mat.z_axis
        );
        rtm_result.rotation = rtm::quat_from_matrix(result_mat);
    }
    else
    {
        // scale = S(world) / S(parent)
        auto parent_scale_rcp = _safe_rcp(rtm_parent.scale);
        rtm_result.scale      = rtm::vector_mul(rtm_world.scale, parent_scale_rcp);

        // -R(parent)
        auto parent_rotation_inv = rtm::quat_conjugate(rtm_parent.rotation);

        // translation = (T(world) - T(parent)) * -Q(parent) / S(parent)
        auto direct_translation = rtm::vector_sub(
            rtm_world.translation,
            rtm_parent.translation
        );
        auto rotated_translation = rtm::quat_mul_vector3(
            direct_translation,
            parent_rotation_inv
        );
        rtm_result.translation = rtm::vector_mul(
            rotated_translation,
            parent_scale_rcp
        );

        // rotation = Q(world) * -Q(parent)
        rtm_result.rotation = rtm::quat_mul(
            rtm_world.rotation,
            parent_rotation_inv
        );
    }
    return RtmConvert<TransformD>::from_rtm(rtm_result);
}
} // namespace math
} // namespace skr