#pragma once
#include "./rtm_conv.hpp"
#include "../gen/misc/quat.hpp"

namespace skr
{
inline namespace math
{
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
struct CoordinateSystem
{
    static_assert(kCrossAxis != kUpAxis && kCrossAxis != kForwardAxis && kUpAxis != kForwardAxis, "Axes must be distinct.");
    static_assert(kCrossAxis >= EAxis3::X && kCrossAxis <= EAxis3::Z, "Invalid cross axis.");
    static_assert(kUpAxis >= EAxis3::X && kUpAxis <= EAxis3::Z, "Invalid up axis.");
    static_assert(kForwardAxis >= EAxis3::X && kForwardAxis <= EAxis3::Z, "Invalid forward axis.");

    inline static constexpr float  kCrossSign        = (kCoordHand == ECoordinateSystemHand::LeftHand) ? 1.0f : -1.0f;
    inline static constexpr size_t kCrossAxisIndex   = static_cast<size_t>(kCrossAxis);
    inline static constexpr size_t kUpAxisIndex      = static_cast<size_t>(kUpAxis);
    inline static constexpr size_t kForwardAxisIndex = static_cast<size_t>(kForwardAxis);

    // convert vector3
    static float3  to_skr(const float3& vec);
    static float3  from_skr(const float3& vec);
    static double3 to_skr(const double3& vec);
    static double3 from_skr(const double3& vec);

    // convert vector3 no cross flip
    static float3  to_skr_no_cross_flip(const float3& vec);
    static float3  from_skr_no_cross_flip(const float3& vec);
    static double3 to_skr_no_cross_flip(const double3& vec);
    static double3 from_skr_no_cross_flip(const double3& vec);

    // convert rotator
    static RotatorF to_skr(const RotatorF& rot);
    static RotatorF from_skr(const RotatorF& rot);
    static RotatorD to_skr(const RotatorD& rot);
    static RotatorD from_skr(const RotatorD& rot);

    // convert quaternion (by translate to rotator)
    static QuatF to_skr(const QuatF& quat);
    static QuatF from_skr(const QuatF& quat);
    static QuatD to_skr(const QuatD& quat);
    static QuatD from_skr(const QuatD& quat);

    // convert transform
    static TransformF to_skr(const TransformF& transform);
    static TransformF from_skr(const TransformF& transform);
    static TransformD to_skr(const TransformD& transform);
    static TransformD from_skr(const TransformD& transform);

private:
    // helpers
    template <typename T>
    static float* _as_float_ptr(T* ptr);
    template <typename T>
    static const float* _as_float_ptr(const T* ptr);
    template <typename T>
    static double* _as_double_ptr(T* ptr);
    template <typename T>
    static const double* _as_double_ptr(const T* ptr);
};

//======================utility coordinate system======================
using CoordinateSystem_LeftHand_YUp = CoordinateSystem<
    ECoordinateSystemHand::LeftHand,
    EAxis3::X, /* cross */
    EAxis3::Y, /* up */
    EAxis3::Z  /* forward */
    >;
using CoordinateSystem_LeftHand_ZUp = CoordinateSystem<
    ECoordinateSystemHand::LeftHand,
    EAxis3::Y, /* cross */
    EAxis3::Z, /* up */
    EAxis3::X  /* forward */
    >;
using CoordinateSystem_RightHand_YUp = CoordinateSystem<
    ECoordinateSystemHand::RightHand,
    EAxis3::X, /* cross */
    EAxis3::Y, /* up */
    EAxis3::Z  /* forward */
    >;
using CoordinateSystem_RightHand_ZUp = CoordinateSystem<
    ECoordinateSystemHand::RightHand,
    EAxis3::Y, /* cross */
    EAxis3::Z, /* up */
    EAxis3::X  /* forward */
    >;

//======================some dcc & engine coordinate system======================

// sakura engine coordinate system
using CoordinateSystem_Skr = CoordinateSystem_LeftHand_YUp;

// left hand && y-up coordinate system
using CoordinateSystem_Unity     = CoordinateSystem_LeftHand_YUp;
using CoordinateSystem_LightWave = CoordinateSystem_LeftHand_YUp;
using CoordinateSystem_ZBrush    = CoordinateSystem_LeftHand_YUp;
using CoordinateSystem_Cinema4D  = CoordinateSystem_LeftHand_YUp;

// left hand && z-up coordinate system
using CoordinateSystem_Unreal = CoordinateSystem_LeftHand_ZUp;

// right hand && y-up coordinate system
using CoordinateSystem_Maya             = CoordinateSystem_RightHand_YUp;
using CoordinateSystem_MoDo             = CoordinateSystem_RightHand_YUp;
using CoordinateSystem_SubstancePainter = CoordinateSystem_RightHand_YUp;
using CoordinateSystem_Houdini          = CoordinateSystem_RightHand_YUp;
using CoordinateSystem_Godot            = CoordinateSystem_RightHand_YUp;
using CoordinateSystem_Roblox           = CoordinateSystem_RightHand_YUp;

// right hand && z-up coordinate system
using CoordinateSystem_3DSMax   = CoordinateSystem_RightHand_ZUp;
using CoordinateSystem_Blender  = CoordinateSystem_RightHand_ZUp;
using CoordinateSystem_SketchUp = CoordinateSystem_RightHand_ZUp;
using CoordinateSystem_Source   = CoordinateSystem_RightHand_ZUp;
using CoordinateSystem_AutoCAD  = CoordinateSystem_RightHand_ZUp;

} // namespace math
} // namespace skr

namespace skr
{
inline namespace math
{
// helpers
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
template <typename T>
inline float* CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::_as_float_ptr(T* ptr)
{
    return reinterpret_cast<float*>(ptr);
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
template <typename T>
inline const float* CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::_as_float_ptr(const T* ptr)
{
    return reinterpret_cast<const float*>(ptr);
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
template <typename T>
inline double* CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::_as_double_ptr(T* ptr)
{
    return reinterpret_cast<double*>(ptr);
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
template <typename T>
inline const double* CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::_as_double_ptr(const T* ptr)
{
    return reinterpret_cast<const double*>(ptr);
}

// convert vector3
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline float3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const float3& vec)
{
    auto p_float = _as_float_ptr(&vec);
    return float3(
        kCrossSign * p_float[kCrossAxisIndex],
        p_float[kUpAxisIndex],
        p_float[kForwardAxisIndex]
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline float3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const float3& vec)
{
    float3 result;
    auto   p_float_result             = _as_float_ptr(&result);
    p_float_result[kCrossAxisIndex]   = kCrossSign * vec.x;
    p_float_result[kUpAxisIndex]      = vec.y;
    p_float_result[kForwardAxisIndex] = vec.z;
    return result;
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline double3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const double3& vec)
{
    auto p_double = _as_double_ptr(&vec);
    return double3(
        kCrossSign * p_double[kCrossAxisIndex],
        p_double[kUpAxisIndex],
        p_double[kForwardAxisIndex]
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline double3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const double3& vec)
{
    double3 result;
    auto    p_double_result            = _as_double_ptr(&result);
    p_double_result[kCrossAxisIndex]   = kCrossSign * vec.x;
    p_double_result[kUpAxisIndex]      = vec.y;
    p_double_result[kForwardAxisIndex] = vec.z;
    return result;
}

// convert vector3 no cross flip
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline float3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr_no_cross_flip(const float3& vec)
{
    auto p_float = _as_float_ptr(&vec);
    return float3(
        p_float[kCrossAxisIndex],
        p_float[kUpAxisIndex],
        p_float[kForwardAxisIndex]
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline float3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr_no_cross_flip(const float3& vec)
{
    float3 result;
    auto   p_float_result             = _as_float_ptr(&result);
    p_float_result[kCrossAxisIndex]   = vec.x;
    p_float_result[kUpAxisIndex]      = vec.y;
    p_float_result[kForwardAxisIndex] = vec.z;
    return result;
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline double3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr_no_cross_flip(const double3& vec)
{
    auto p_double = _as_double_ptr(&vec);
    return double3(
        p_double[kCrossAxisIndex],
        p_double[kUpAxisIndex],
        p_double[kForwardAxisIndex]
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline double3 CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr_no_cross_flip(const double3& vec)
{
    double3 result;
    auto    p_double_result            = _as_double_ptr(&result);
    p_double_result[kCrossAxisIndex]   = vec.x;
    p_double_result[kUpAxisIndex]      = vec.y;
    p_double_result[kForwardAxisIndex] = vec.z;
    return result;
}

// convert rotator
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline RotatorF CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const RotatorF& rot)
{
    auto p_float = _as_float_ptr(&rot);
    return RotatorF(
        kCrossSign * p_float[kCrossAxisIndex],
        p_float[kUpAxisIndex],
        p_float[kForwardAxisIndex]
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline RotatorF CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const RotatorF& rot)
{
    RotatorF result;
    auto     p_float_result           = _as_float_ptr(&result);
    p_float_result[kCrossAxisIndex]   = kCrossSign * rot.pitch;
    p_float_result[kUpAxisIndex]      = rot.yaw;
    p_float_result[kForwardAxisIndex] = rot.roll;
    return result;
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline RotatorD CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const RotatorD& rot)
{
    auto p_double = _as_double_ptr(&rot);
    return RotatorD(
        kCrossSign * p_double[kCrossAxisIndex],
        p_double[kUpAxisIndex],
        p_double[kForwardAxisIndex]
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline RotatorD CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const RotatorD& rot)
{
    RotatorD result;
    auto     p_double_result           = _as_double_ptr(&result);
    p_double_result[kCrossAxisIndex]   = kCrossSign * rot.pitch;
    p_double_result[kUpAxisIndex]      = rot.yaw;
    p_double_result[kForwardAxisIndex] = rot.roll;
    return result;
}

// convert quaternion (by translate to rotator)
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline QuatF CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const QuatF& quat)
{
    return QuatF(to_skr(RotatorF(quat)));
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline QuatF CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const QuatF& quat)
{
    return QuatF(from_skr(RotatorF(quat)));
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline QuatD CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const QuatD& quat)
{
    return QuatD(to_skr(RotatorD(quat)));
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline QuatD CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const QuatD& quat)
{
    return QuatD(from_skr(RotatorD(quat)));
}

// convert transform
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline TransformF CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const TransformF& transform)
{
    return TransformF(
        to_skr(transform.rotation),
        to_skr(transform.position),
        to_skr_no_cross_flip(transform.scale)
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline TransformF CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const TransformF& transform)
{
    return TransformF(
        from_skr(transform.rotation),
        from_skr(transform.position),
        to_skr_no_cross_flip(transform.scale)
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline TransformD CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::to_skr(const TransformD& transform)
{
    return TransformD(
        to_skr(transform.rotation),
        to_skr(transform.position),
        to_skr_no_cross_flip(transform.scale)
    );
}
template <ECoordinateSystemHand kCoordHand, EAxis3 kCrossAxis, EAxis3 kUpAxis, EAxis3 kForwardAxis>
inline TransformD CoordinateSystem<kCoordHand, kCrossAxis, kUpAxis, kForwardAxis>::from_skr(const TransformD& transform)
{
    return TransformD(
        from_skr(transform.rotation),
        from_skr(transform.position),
        to_skr_no_cross_flip(transform.scale)
    );
}
} // namespace math
} // namespace skr