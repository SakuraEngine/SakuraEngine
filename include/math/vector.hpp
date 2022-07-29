#pragma once
#include "containers/span.hpp"
#include "containers/array.hpp"
#include "scalarmath.h"
#include "utils/types.h"

namespace skr
{
namespace math
{
template <typename T, size_t N>
struct Vector {
    constexpr Vector() = default;
    constexpr Vector(const Vector& rhs) = default;
    Vector& operator=(const Vector& rhs) = default;

    constexpr Vector(const skr::array<T, N> v)
        : m_(v)
    {
    }
    skr::span<T, N> data_view()
    {
        return m_;
    }
    skr::span<const T, N> data_view() const
    {
        return m_;
    }
    FORCEINLINE const T& operator[](size_t idx) const
    {
        return data_view()[idx];
    }

    FORCEINLINE T& operator[](size_t idx)
    {
        return data_view()[idx];
    }

    T length_squared() const
    {
        T res = 0;
        // TODO: unroll with index_sequence
        for (auto i = 0u; i < N; i++)
        {
            res += m_.at(i) * m_.at(i);
        }
        return res;
    }
    T length() const
    {
        return math::sqrt(length_squared());
    }
    FORCEINLINE bool is_zero() const
    {
        return length_squared() == 0;
    }
    static constexpr Vector<T, N> vector_one();
    static constexpr Vector<T, N> vector_zero();

    skr::array<T, N> m_ = skr::create_array<T, N>(0);
};

template <typename T, size_t N>
constexpr Vector<T, N> Vector<T, N>::vector_one()
{
    return Vector<T, N>(skr::create_array<T, N>(1));
}
template <typename T, size_t N>
constexpr Vector<T, N> Vector<T, N>::vector_zero()
{
    return Vector<T, N>(skr::create_array<T, N>(0));
}

// Vector2
template <typename T>
struct Vector<T, 2> {
public:
    FORCEINLINE constexpr Vector() = default;
    FORCEINLINE constexpr Vector(const T x, const T y);
    FORCEINLINE constexpr Vector(const skr::array<T, 2> v);
    FORCEINLINE constexpr Vector(const skr_float2_t v);
    FORCEINLINE skr::span<T, 2> data_view();
    FORCEINLINE skr::span<const T, 2> data_view() const;
    FORCEINLINE static constexpr Vector<T, 2> vector_one();
    FORCEINLINE static constexpr Vector<T, 2> vector_zero();

    FORCEINLINE Vector operator+(const Vector V) const;
    FORCEINLINE Vector operator-(const Vector V) const;
    FORCEINLINE Vector operator-(T Bias) const;
    FORCEINLINE Vector operator+(T Bias) const;
    FORCEINLINE Vector operator*(T Scale) const;
    FORCEINLINE friend Vector operator*(T Scale, const Vector vec)
    {
        return vec * Scale;
    }

    FORCEINLINE const T& operator[](size_t idx) const
    {
        return data_view()[idx];
    }

    FORCEINLINE T& operator[](size_t idx)
    {
        return data_view()[idx];
    }

    Vector operator/(T Scale) const;
    bool operator==(const Vector V) const;
    bool operator!=(const Vector V) const;
    bool equals(const Vector V, T Tolerance = KINDA_SMALL_NUMBER) const;
    FORCEINLINE Vector operator-() const;
    FORCEINLINE Vector operator+=(const Vector V);
    FORCEINLINE Vector operator-=(const Vector V);
    FORCEINLINE Vector operator*=(const T Scale);
    FORCEINLINE Vector operator/=(T V);

    FORCEINLINE friend bool operator==(const Vector<float, 2> v2, const skr_float2_t f2)
    {
        return f2.x == v2.x && f2.y == v2.y;
    }
    FORCEINLINE friend bool operator==(const skr_float2_t f2, const Vector<float, 2> v2)
    {
        return f2.x == v2.x && f2.y == v2.y;
    }
    FORCEINLINE friend bool operator!=(const Vector<float, 2> v2, const skr_float2_t f2)
    {
        return !(f2 == v2);
    }
    FORCEINLINE friend bool operator!=(const skr_float2_t f2, const Vector<float, 2> v2)
    {
        return !(f2 == v2);
    }
    FORCEINLINE constexpr operator skr_float2_t() const
    {
        return {x, y};
    }

    /**
     * Gets the result of component-wise multiplication of this vector by another.
     *
     * @param V The vector to multiply with.
     * @return The result of multiplication.
     */
    FORCEINLINE Vector operator*(const Vector V) const;

    /**
     * Gets the result of component-wise division of this vector by another.
     *
     * @param V The vector to divide by.
     * @return The result of division.
     */
    FORCEINLINE Vector operator/(const Vector V) const;

    /**
     * Multiplies the vector with another vector, using component-wise multiplication.
     *
     * @param V What to multiply this vector with.
     * @return Copy of the vector after multiplication.
     */
    Vector operator*=(const Vector V);

    /**
     * Divides the vector by another vector, using component-wise division.
     *
     * @param V What to divide vector by.
     * @return Copy of the vector after division.
     */
    Vector operator/=(const Vector V);

    /**
     * Calculate the dot product between this and another vector.
     *
     * @param V The other vector.
     * @return The dot product.
     */
    FORCEINLINE T operator|(const Vector V) const;
    FORCEINLINE static T dot_product(const Vector A, const Vector B);

    FORCEINLINE T length() const;
    FORCEINLINE T length_squared() const;
    FORCEINLINE bool is_zero() const;
    FORCEINLINE bool is_nearly_zero(T tolerence = SMALL_NUMBER) const;
    FORCEINLINE bool is_normalized() const;

    union
    {
        struct {
            T X, Y;
        };
        struct {
            T x, y;
        };
        skr::array<T, 2> m_ = { 0, 0 };
    };
};
using Vector2i = Vector<int32_t, 2u>;
using Vector2f = Vector<float, 2u>;
struct Rect {
    Vector2f min;
    Vector2f max;
    bool operator==(const Rect& other) { return min == other.min && max == other.max; }
    bool operator!=(const Rect& other) { return !(*this == other); }
    bool IntersectPoint(Vector2f point)
    {
        return point.X >= min.X && point.Y >= min.Y && point.X < max.X && point.Y < max.Y;
    }
};

// Vector3
template <typename T>
struct Vector<T, 3> {
public:
    FORCEINLINE constexpr Vector() = default;
    FORCEINLINE constexpr Vector(const T x, const T y, const T z);
    FORCEINLINE constexpr Vector(const skr::array<T, 3> v);
    FORCEINLINE constexpr Vector(const skr_float3_t v);
    FORCEINLINE skr::span<T, 3> data_view();
    FORCEINLINE skr::span<const T, 3> data_view() const;
    FORCEINLINE static constexpr Vector<T, 3> vector_one();
    FORCEINLINE static constexpr Vector<T, 3> vector_zero();

    FORCEINLINE Vector operator+(const Vector V) const;
    FORCEINLINE Vector operator-(const Vector V) const;
    FORCEINLINE Vector operator-(T Bias) const;
    FORCEINLINE Vector operator+(T Bias) const;
    FORCEINLINE Vector operator*(T Scale) const;
    FORCEINLINE friend Vector operator*(T Scale, const Vector vec)
    {
        return vec * Scale;
    }

    FORCEINLINE const T& operator[](size_t idx) const
    {
        return data_view()[idx];
    }

    FORCEINLINE T& operator[](size_t idx)
    {
        return data_view()[idx];
    }

    Vector operator/(T Scale) const;
    bool operator==(const Vector V) const;
    bool operator!=(const Vector V) const;
    bool equals(const Vector V, T Tolerance = KINDA_SMALL_NUMBER) const;
    FORCEINLINE Vector operator-() const;
    FORCEINLINE Vector operator+=(const Vector V);
    FORCEINLINE Vector operator-=(const Vector V);
    FORCEINLINE Vector operator*=(const T Scale);
    FORCEINLINE Vector operator/=(T V);

    FORCEINLINE friend bool operator==(const Vector<float, 3> v3, const skr_float3_t f3)
    {
        return f3.x == v3.x && f3.y == v3.y && f3.z == v3.z;
    }
    FORCEINLINE friend bool operator==(const skr_float3_t f3, const Vector<float, 3> v3)
    {
        return f3.x == v3.x && f3.y == v3.y && f3.z == v3.z;
    }
    FORCEINLINE friend bool operator!=(const Vector<float, 3> v3, const skr_float3_t f3)
    {
        return !(f3 == v3);
    }
    FORCEINLINE friend bool operator!=(const skr_float3_t f3, const Vector<float, 3> v3)
    {
        return !(f3 == v3);
    }
    FORCEINLINE constexpr operator skr_float3_t() const
    {
        return {x, y, z};
    }

    /**
     * Gets the result of component-wise multiplication of this vector by another.
     *
     * @param V The vector to multiply with.
     * @return The result of multiplication.
     */
    FORCEINLINE Vector operator*(const Vector V) const;

    /**
     * Gets the result of component-wise division of this vector by another.
     *
     * @param V The vector to divide by.
     * @return The result of division.
     */
    FORCEINLINE Vector operator/(const Vector V) const;

    /**
     * Multiplies the vector with another vector, using component-wise multiplication.
     *
     * @param V What to multiply this vector with.
     * @return Copy of the vector after multiplication.
     */
    Vector operator*=(const Vector V);

    /**
     * Divides the vector by another vector, using component-wise division.
     *
     * @param V What to divide vector by.
     * @return Copy of the vector after division.
     */
    Vector operator/=(const Vector V);

    /**
     * Calculate cross product between this and another vector.
     *
     * @param V The other vector.
     * @return The cross product.
     */
    FORCEINLINE Vector operator^(const Vector V) const;
    FORCEINLINE static Vector cross_product(const Vector A, const Vector B);

    /**
     * Calculate the dot product between this and another vector.
     *
     * @param V The other vector.
     * @return The dot product.
     */
    FORCEINLINE T operator|(const Vector V) const;
    FORCEINLINE static T dot_product(const Vector A, const Vector B);

    FORCEINLINE T length() const;
    FORCEINLINE T length_squared() const;
    FORCEINLINE bool is_zero() const;
    FORCEINLINE bool is_nearly_zero(T tolerence = SMALL_NUMBER) const;
    FORCEINLINE bool is_normalized() const;

    union
    {
        struct {
            T X, Y, Z;
        };
        struct {
            T x, y, z;
        };
        skr::array<T, 3> m_ = { 0, 0, 0 };
    };
};
using Vector3f = Vector<float, 3>;
using Vector3lf = Vector<double, 3>;
using Vector3u = Vector<uint32_t, 3>;
using Vector3i = Vector<int32_t, 3>;

// Vector4f
template <>
struct alignas(16) Vector<float, 4> {
public:
    FORCEINLINE constexpr Vector() = default;
    FORCEINLINE constexpr Vector(float x, float y, float z, float w)
        : m_({ x, y, z, w })
    {
    }
    FORCEINLINE constexpr Vector(const skr_float4_t v)
        : m_({ v.x, v.y, v.z, v.w })
    {

    }
    FORCEINLINE constexpr Vector(const skr::array<float, 4> v)
        : m_(v)
    {
    }
    FORCEINLINE skr::span<float, 4> data_view()
    {
        return m_.mValue;
    }
    FORCEINLINE skr::span<const float, 4> data_view() const
    {
        return m_.mValue;
    }
    FORCEINLINE float length() const;
    FORCEINLINE float length_squared() const;
    FORCEINLINE static constexpr Vector<float, 4> vector_one();
    FORCEINLINE static constexpr Vector<float, 4> vector_zero();

    FORCEINLINE Vector operator+(const Vector V) const;
    FORCEINLINE Vector operator-(const Vector V) const;
    FORCEINLINE Vector operator-(float Bias) const;
    FORCEINLINE Vector operator+(float Bias) const;
    FORCEINLINE Vector operator*(float Scale) const;
    FORCEINLINE friend Vector operator*(float Scale, const Vector vec)
    {
        return vec * Scale;
    }

    Vector operator/(float Scale) const;
    bool operator==(const Vector V) const;
    FORCEINLINE friend bool operator==(const Vector<float, 4> v4, const skr_float4_t f4)
    {
        return f4.x == v4.x && f4.y == v4.y && f4.z == v4.z && f4.w == v4.w;
    }
    FORCEINLINE friend bool operator==(const skr_float4_t f4, const Vector<float, 4> v4)
    {
        return f4.x == v4.x && f4.y == v4.y && f4.z == v4.z && f4.w == v4.w;
    }
    FORCEINLINE friend bool operator!=(const Vector<float, 4> v4, const skr_float4_t f4)
    {
        return !(f4 == v4);
    }
    FORCEINLINE friend bool operator!=(const skr_float4_t f4, const Vector<float, 4> v4)
    {
        return !(f4 == v4);
    }
    FORCEINLINE constexpr operator skr_float4_t() const
    {
        return {x, y, z, w};
    }
    bool operator!=(const Vector V) const;
    bool equals(const Vector V, float Tolerance = KINDA_SMALL_NUMBER) const;
    FORCEINLINE Vector operator-() const;
    FORCEINLINE Vector operator+=(const Vector V);
    FORCEINLINE Vector operator-=(const Vector V);
    FORCEINLINE Vector operator*=(const float Scale);
    FORCEINLINE Vector operator/=(float V);

    /**
     * Gets the result of component-wise multiplication of this vector by another.
     *
     * @param V The vector to multiply with.
     * @return The result of multiplication.
     */
    FORCEINLINE Vector operator*(const Vector V) const;

    /**
     * Gets the result of component-wise division of this vector by another.
     *
     * @param V The vector to divide by.
     * @return The result of division.
     */
    FORCEINLINE Vector operator/(const Vector V) const;

    /**
     * Multiplies the vector with another vector, using component-wise multiplication.
     *
     * @param V What to multiply this vector with.
     * @return Copy of the vector after multiplication.
     */
    Vector operator*=(const Vector V);

    /**
     * Divides the vector by another vector, using component-wise division.
     *
     * @param V What to divide vector by.
     * @return Copy of the vector after division.
     */
    Vector operator/=(const Vector V);

    /**
     * Calculate the dot product between this and another vector.
     *
     * @param V The other vector.
     * @return The dot product.
     */
    FORCEINLINE float operator|(const Vector V) const;
    FORCEINLINE static float dot_product(const Vector A, const Vector B);

    FORCEINLINE bool is_zero() const;
    FORCEINLINE bool is_nearly_zero(float tolerence = SMALL_NUMBER) const;
    FORCEINLINE bool is_normalized() const;

    union
    {
        struct alignas(16) {
            float X, Y, Z, W;
        };
        struct alignas(16) {
            float x, y, z, w;
        };
        alignas(16) skr::array<float, 4> m_ = { 0.f, 0.f, 0.f, 0.f };
    };
};
using Color4u = Vector<uint8_t, 4u>;
using Color4f = Vector<float, 4u>;
using Vector4f = Vector<float, 4>;
using Vector4u = Vector<uint32_t, 4>;
using Vector4i = Vector<int32_t, 4>;

FORCEINLINE constexpr Vector4f Vector4f::vector_one()
{
    return Vector4f(1.f, 1.f, 1.f, 1.f);
}
FORCEINLINE constexpr Vector4f Vector4f::vector_zero()
{
    return Vector4f(0.f, 0.f, 0.f, 0.f);
}
using Vector4lf = Vector<double, 4>;
} // namespace math
} // namespace skr

// Common Vector Math
namespace skr
{
namespace math
{
template <typename T, size_t Dimension>
FORCEINLINE Vector<T, Dimension> normalize(const Vector<T, Dimension>& vec, const T tolerance = SMALL_NUMBER)
{
    const float SquareSum = vec.length_squared();
    if (SquareSum > tolerance)
    {
        const float Scale = math::rsqrt(SquareSum);
        return vec * Scale;
    }
    return Vector<T, Dimension>::vector_zero();
}

template <typename T>
FORCEINLINE Vector<T, 3> cross_product(const Vector<T, 3> a, const Vector<T, 3> b)
{
    return a ^ b;
}

template <typename T, size_t N>
FORCEINLINE T dot_product(const Vector<T, N> a, const Vector<T, N> b)
{
    T res = T(0);
    for (auto i = 0u; i < N; i++)
    {
        res += a.data_view()[i] * b.data_view()[i];
    }
    return res;
}

template <typename T, size_t N>
FORCEINLINE T length(const Vector<T, N> vec)
{
    return vec.length();
}

template <typename T, size_t N>
FORCEINLINE Vector<T, N> add(const Vector<T, N> a, const Vector<T, N> b)
{
    return a + b;
}

template <typename T, size_t N>
FORCEINLINE Vector<T, N> subtract(const Vector<T, N> a, const Vector<T, N> b)
{
    return a - b;
}

template <typename T, size_t N>
FORCEINLINE T distance(const Vector<T, N> a, const Vector<T, N> b)
{
    return length(subtract(a, b));
}
} // namespace math
} // namespace skr

#include "detail/Vector2Impl.inl"
#include "detail/Vector3Impl.inl"
#include "detail/Vector4fImpl.inl"

#ifdef USE_DXMATH
    #include "DirectXMath/SDXMathVector.hpp"
#else

#endif