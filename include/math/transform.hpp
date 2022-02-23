#pragma once
#include "quaternion.hpp"
#include "vector.hpp"

namespace sakura
{
namespace math
{
struct Translation {
    Vector3f translation_ = Vector3f::vector_zero();
};

struct Rotation {
    Vector3f rotation_ = Vector3f::vector_zero();
};

struct Scale {
    Vector3f scale_ = Vector3f::vector_one();
};

// RH.
struct alignas(16) Transform {
    using VectorStorage = __vector::VectorRegister;

public:
    FORCEINLINE Transform(
        const Quaternion& rotation, const Vector3f& translation, const Vector3f& scale = Vector3f::vector_one())
    {
        rotation_ = __vector::load_aligned(rotation.data_view());
        translation_ = __vector::load_float3_w0(translation.data_view());
        scale_ = __vector::load_float3_w0(scale.data_view());
    }

    FORCEINLINE Transform(
        const VectorStorage& rotation, const VectorStorage& translation, const VectorStorage& scale)
        : rotation_(rotation)
        , translation_(translation)
        , scale_(scale)
    {
    }

    sakura::span<float, 16> data_view()
    {
        return M16;
    }
    const sakura::span<const float, 16> data_view() const
    {
        return M16;
    }

    FORCEINLINE Transform& operator=(const Transform& Other)
    {
        this->rotation_ = Other.rotation_;
        this->translation_ = Other.translation_;
        this->scale_ = Other.scale_;
        return *this;
    }

protected:
    union
    {
        struct
        {
            VectorStorage rotation_;
            VectorStorage translation_;
            VectorStorage scale_;
        };
        alignas(16) float M16[16] = {
            0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
            1.f, 1.f, 1.f, 1.f
        };
    };
};

} // namespace math
} // namespace sakura

#ifdef USE_DXMATH
    #include "DirectXMath/SDXMathTransform.hpp"
#else

#endif