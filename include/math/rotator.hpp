#pragma once
#include "vector.hpp"

namespace skr
{
namespace math
{
struct Rotator : public Vector3f {
    Rotator() = default;
    Rotator(const Vector3f& value)
        : Vector3f(value)
    {
    }
    Rotator(const float pitch, const float yaw, const float roll)
        : Vector3f(pitch, yaw, roll)
    {
    }
    FORCEINLINE Rotator& operator=(const Vector3f value)
    {
        X = value.data_view()[0];
        Y = value.data_view()[1];
        Z = value.data_view()[2];
        return *this;
    }

    FORCEINLINE float pitch() const
    {
        return data_view()[0];
    }
    FORCEINLINE void pitch(float value)
    {
        m_[0] = value;
    }

    FORCEINLINE float yaw() const
    {
        return data_view()[1];
    }
    FORCEINLINE void yaw(float value)
    {
        m_[1] = value;
    }

    FORCEINLINE float roll() const
    {
        return data_view()[2];
    }
    FORCEINLINE void roll(float value)
    {
        m_[2] = value;
    }

    FORCEINLINE static float clamp_axis(float Angle)
    {
        // returns Angle in the range (-360,360)
        Angle = skr::math::mod(Angle, 360.f);

        if (Angle < 0.f)
        {
            // shift to [0,360) range
            Angle += 360.f;
        }

        return Angle;
    }

    FORCEINLINE static float normalize_axis(float Angle)
    {
        // returns Angle in the range [0,360)
        Angle = clamp_axis(Angle);

        if (Angle > 180.f)
        {
            // shift to (-180,180]
            Angle -= 360.f;
        }

        return Angle;
    }
};
} // namespace math
} // namespace skr