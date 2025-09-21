#pragma once
// skr types
#include "../gen/gen_math.hpp"

// rtm types
#include <rtm/scalarf.h>
#include <rtm/scalard.h>
#include <rtm/quatf.h>
#include <rtm/quatd.h>
#include <rtm/vector4f.h>
#include <rtm/vector4d.h>
#include <rtm/qvvf.h>
#include <rtm/qvvd.h>
#include <rtm/matrix3x3f.h>
#include <rtm/matrix3x3d.h>
#include <rtm/matrix3x4f.h>
#include <rtm/matrix3x4d.h>
#include <rtm/matrix4x4f.h>
#include <rtm/matrix4x4d.h>

namespace skr
{
inline namespace math
{
template <typename T>
struct RtmConvert
{
};

// vector2
template <>
struct RtmConvert<float2>
{
    inline static rtm::vector4f to_rtm(const float2& v)
    {
        return rtm::vector_load2(&v.x);
    }
    inline static void store(const rtm::vector4f& v, float2& out)
    {
        rtm::vector_store2(v, &out.x);
    }
    inline static float2 from_rtm(const rtm::vector4f& v)
    {
        float2 result{ kMathNoInit };
        store(v, result);
        return result;
    }
};
template <>
struct RtmConvert<double2>
{
    inline static rtm::vector4d to_rtm(const double2& v)
    {
        return rtm::vector_load2(&v.x);
    }
    inline static void store(const rtm::vector4d& v, double2& out)
    {
        rtm::vector_store2(v, &out.x);
    }
    inline static double2 from_rtm(const rtm::vector4d& v)
    {
        double2 result{ kMathNoInit };
        store(v, result);
        return result;
    }
};

// vector3
template <>
struct RtmConvert<float3>
{
    inline static rtm::vector4f to_rtm(const float3& v)
    {
        return rtm::vector_load3(&v.x);
    }
    inline static void store(const rtm::vector4f& v, float3& out)
    {
        rtm::vector_store3(v, &out.x);
    }
    inline static float3 from_rtm(const rtm::vector4f& v)
    {
        float3 result{ kMathNoInit };
        store(v, result);
        return result;
    }
};
template <>
struct RtmConvert<double3>
{
    inline static rtm::vector4d to_rtm(const double3& v)
    {
        return rtm::vector_load3(&v.x);
    }
    inline static void store(const rtm::vector4d& v, double3& out)
    {
        rtm::vector_store3(v, &out.x);
    }
    inline static double3 from_rtm(const rtm::vector4d& v)
    {
        double3 result{ kMathNoInit };
        store(v, result);
        return result;
    }
};

// vector4
template <>
struct RtmConvert<float4>
{
    inline static rtm::vector4f to_rtm(const float4& v)
    {
        return rtm::vector_load(&v.x);
    }
    inline static void store(const rtm::vector4f& v, float4& out)
    {
        rtm::vector_store(v, &out.x);
    }
    inline static float4 from_rtm(const rtm::vector4f& v)
    {
        float4 result{ kMathNoInit };
        store(v, result);
        return result;
    }
};
template <>
struct RtmConvert<double4>
{
    inline static rtm::vector4d to_rtm(const double4& v)
    {
        return rtm::vector_load(&v.x);
    }
    inline static void store(const rtm::vector4d& v, double4& out)
    {
        rtm::vector_store(v, &out.x);
    }
    inline static double4 from_rtm(const rtm::vector4d& v)
    {
        double4 result{ kMathNoInit };
        store(v, result);
        return result;
    }
};

// matrix3x3
template <>
struct RtmConvert<float3x3>
{
    inline static rtm::matrix3x3f to_rtm(const float3x3& m)
    {
        return {
            RtmConvert<float3>::to_rtm(m.axis_x),
            RtmConvert<float3>::to_rtm(m.axis_y),
            RtmConvert<float3>::to_rtm(m.axis_z),
        };
    }
    inline static void store(const rtm::matrix3x3f& m, float3x3& out)
    {
        RtmConvert<float3>::store(m.x_axis, out.axis_x);
        RtmConvert<float3>::store(m.y_axis, out.axis_y);
        RtmConvert<float3>::store(m.z_axis, out.axis_z);
    }
    inline static float3x3 from_rtm(const rtm::matrix3x3f& m)
    {
        float3x3 result{ kMathNoInit };
        store(m, result);
        return result;
    }
};
template <>
struct RtmConvert<double3x3>
{
    inline static rtm::matrix3x3d to_rtm(const double3x3& m)
    {
        return {
            RtmConvert<double3>::to_rtm(m.axis_x),
            RtmConvert<double3>::to_rtm(m.axis_y),
            RtmConvert<double3>::to_rtm(m.axis_z),
        };
    }
    inline static void store(const rtm::matrix3x3d& m, double3x3& out)
    {
        RtmConvert<double3>::store(m.x_axis, out.axis_x);
        RtmConvert<double3>::store(m.y_axis, out.axis_y);
        RtmConvert<double3>::store(m.z_axis, out.axis_z);
    }
    inline static double3x3 from_rtm(const rtm::matrix3x3d& m)
    {
        double3x3 result{ kMathNoInit };
        store(m, result);
        return result;
    }
};

// matrix4x4
template <>
struct RtmConvert<float4x4>
{
    inline static rtm::matrix4x4f to_rtm(const float4x4& m)
    {
        return {
            RtmConvert<float4>::to_rtm(m.axis_x),
            RtmConvert<float4>::to_rtm(m.axis_y),
            RtmConvert<float4>::to_rtm(m.axis_z),
            RtmConvert<float4>::to_rtm(m.axis_w),
        };
    }
    inline static void store(const rtm::matrix4x4f& m, float4x4& out)
    {
        RtmConvert<float4>::store(m.x_axis, out.axis_x);
        RtmConvert<float4>::store(m.y_axis, out.axis_y);
        RtmConvert<float4>::store(m.z_axis, out.axis_z);
        RtmConvert<float4>::store(m.w_axis, out.axis_w);
    }
    inline static float4x4 from_rtm(const rtm::matrix4x4f& m)
    {
        float4x4 result{ kMathNoInit };
        store(m, result);
        return result;
    }
};
template <>
struct RtmConvert<double4x4>
{
    inline static rtm::matrix4x4d to_rtm(const double4x4& m)
    {
        return {
            RtmConvert<double4>::to_rtm(m.axis_x),
            RtmConvert<double4>::to_rtm(m.axis_y),
            RtmConvert<double4>::to_rtm(m.axis_z),
            RtmConvert<double4>::to_rtm(m.axis_w),
        };
    }
    inline static void store(const rtm::matrix4x4d& m, double4x4& out)
    {
        RtmConvert<double4>::store(m.x_axis, out.axis_x);
        RtmConvert<double4>::store(m.y_axis, out.axis_y);
        RtmConvert<double4>::store(m.z_axis, out.axis_z);
        RtmConvert<double4>::store(m.w_axis, out.axis_w);
    }
    inline static double4x4 from_rtm(const rtm::matrix4x4d& m)
    {
        double4x4 result;
        store(m, result);
        return result;
    }
};

// quat
template <>
struct RtmConvert<QuatF>
{
    inline static rtm::quatf to_rtm(const QuatF& q)
    {
        return rtm::quat_load(&q.x);
    }
    inline static void store(const rtm::quatf& q, QuatF& out)
    {
        rtm::quat_store(q, &out.x);
    }
    inline static QuatF from_rtm(const rtm::quatf& q)
    {
        QuatF result{ kMathNoInit };
        store(q, result);
        return result;
    }
};
template <>
struct RtmConvert<QuatD>
{
    inline static rtm::quatd to_rtm(const QuatD& q)
    {
        return rtm::quat_load(&q.x);
    }
    inline static void store(const rtm::quatd& q, QuatD& out)
    {
        rtm::quat_store(q, &out.x);
    }
    inline static QuatD from_rtm(const rtm::quatd& q)
    {
        QuatD result{ kMathNoInit };
        store(q, result);
        return result;
    }
};

// transform
template <>
struct RtmConvert<TransformF>
{
    inline static rtm::qvvf to_rtm(const TransformF& t)
    {
        return rtm::qvvf(
            RtmConvert<QuatF>::to_rtm(t.rotation),
            RtmConvert<float3>::to_rtm(t.position),
            RtmConvert<float3>::to_rtm(t.scale)
        );
    }
    inline static void store(const rtm::qvvf& t, TransformF& out)
    {
        RtmConvert<QuatF>::store(t.rotation, out.rotation);
        RtmConvert<float3>::store(t.translation, out.position);
        RtmConvert<float3>::store(t.scale, out.scale);
    }
    inline static TransformF from_rtm(const rtm::qvvf& t)
    {
        TransformF result{ kMathNoInit };
        store(t, result);
        return result;
    }
};
template <>
struct RtmConvert<TransformD>
{
    inline static rtm::qvvd to_rtm(const TransformD& t)
    {
        return rtm::qvvd(
            RtmConvert<QuatD>::to_rtm(t.rotation),
            RtmConvert<double3>::to_rtm(t.position),
            RtmConvert<double3>::to_rtm(t.scale)
        );
    }
    inline static void store(const rtm::qvvd& t, TransformD& out)
    {
        RtmConvert<QuatD>::store(t.rotation, out.rotation);
        RtmConvert<double3>::store(t.translation, out.position);
        RtmConvert<double3>::store(t.scale, out.scale);
    }
    inline static TransformD from_rtm(const rtm::qvvd& t)
    {
        TransformD result{ kMathNoInit };
        store(t, result);
        return result;
    }
};

} // namespace math
} // namespace skr