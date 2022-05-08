#pragma once
#include "rotator.hpp"
#include "vector.hpp"

namespace skr
{
namespace math
{
struct alignas(16) Quaternion : public Vector4f {
public:
    constexpr Quaternion() = default;
    FORCEINLINE constexpr Quaternion(float x, float y, float z, float w)
        : Vector4f(x, y, z, w)
    {
    }
    FORCEINLINE Quaternion(const Vector4f& value)
        : Vector4f(value)
    {
    }
    static constexpr Quaternion identity() noexcept;

public:
    Rotator rotator() const;
    Quaternion conjugate(void) const;
};

FORCEINLINE constexpr Quaternion Quaternion::identity() noexcept
{
    return Quaternion(0, 0, 0, 1);
}

FORCEINLINE Rotator Quaternion::rotator() const
{
    assert(0 && "Unimplemented!");
    Rotator rot;
    return rot;
}

FORCEINLINE Quaternion Quaternion::conjugate() const
{
    return Quaternion(-X, -Y, -Z, -W);
}

} // namespace math
} // namespace skr

#ifdef USE_DXMATH
    #include "DirectXMath/SDXMathQuaternion.hpp"
#else

#endif