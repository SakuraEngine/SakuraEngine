namespace skr
{
namespace math
{

template <typename T>
FORCEINLINE constexpr Vector<T, 3>::Vector(const T x, const T y, const T z)
    : m_({ x, y, z })
{
}

template <typename T>
FORCEINLINE constexpr Vector<T, 3>::Vector(const skr::array<T, 3> v)
    : m_(v)
{
}

template<typename T>
FORCEINLINE constexpr Vector<T, 3>::Vector(const skr_float3_t v)
    :m_({v.x, v.y, v.z})
{

}

template <typename T>
FORCEINLINE span<T, 3> Vector<T, 3>::data_view()
{
    return m_.mValue;
}

template <typename T>
FORCEINLINE span<const T, 3> Vector<T, 3>::data_view() const
{
    return m_.mValue;
}

template <typename T>
FORCEINLINE constexpr Vector<T, 3> Vector<T, 3>::vector_zero()
{
    return Vector<T, 3>(0, 0, 0);
}

template <typename T>
FORCEINLINE constexpr Vector<T, 3> Vector<T, 3>::vector_one()
{
    return Vector<T, 3>(1, 1, 1);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator^(const Vector V) const
{
    return Vector(
    Y * V.Z - Z * V.Y,
    Z * V.X - X * V.Z,
    X * V.Y - Y * V.X);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::cross_product(const Vector A, const Vector B)
{
    return A ^ B;
}

template <typename T>
FORCEINLINE T Vector<T, 3>::operator|(const Vector V) const
{
    return X * V.X + Y * V.Y + Z * V.Z;
}

template <typename T>
FORCEINLINE T Vector<T, 3>::dot_product(const Vector A, const Vector B)
{
    return A | B;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator+(const Vector V) const
{
    return Vector(X + V.X, Y + V.Y, Z + V.Z);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator-(const Vector V) const
{
    return Vector(X - V.X, Y - V.Y, Z - V.Z);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator-() const
{
    return Vector(-X, -Y, -Z);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator-(T Bias) const
{
    return Vector(X - Bias, Y - Bias, Z - Bias);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator+(T Bias) const
{
    return Vector(X + Bias, Y + Bias, Z + Bias);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator+=(const Vector V)
{
    X += V.X;
    Y += V.Y;
    Z += V.Z;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator-=(const Vector V)
{
    X -= V.X;
    Y -= V.Y;
    Z -= V.Z;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator*=(const Vector V)
{
    *this = *this * V;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator*=(const T Scalar)
{
    *this = *this * Scalar;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator/=(const T Scalar)
{
    *this = *this / Scalar;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator/=(const Vector V)
{
    *this = *this / V;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator*(T Scale) const
{
    return Vector(X * Scale, Y * Scale, Z * Scale);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator/(T Scale) const
{
    const T RScale = 1.f / Scale;
    return Vector(X * RScale, Y * RScale, Z * RScale);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator*(const Vector V) const
{
    return Vector(X * V.X, Y * V.Y, Z * V.Z);
}

template <typename T>
FORCEINLINE Vector<T, 3> Vector<T, 3>::operator/(const Vector V) const
{
    return Vector(X / V.X, Y / V.Y, Z / V.Z);
}

template <typename T>
FORCEINLINE bool Vector<T, 3>::operator==(const Vector V) const
{
    return X == V.X && Y == V.Y && Z == V.Z;
}

template <typename T>
FORCEINLINE bool Vector<T, 3>::operator!=(const Vector V) const
{
    return X != V.X || Y != V.Y || Z != V.Z;
}

template <typename T>
FORCEINLINE bool Vector<T, 3>::equals(const Vector V, T Tolerance) const
{
    return math::abs(X - V.X) <= Tolerance && math::abs(Y - V.Y) <= Tolerance && math::abs(Z - V.Z) <= Tolerance;
}

template <typename T>
FORCEINLINE T Vector<T, 3>::length() const
{
    return math::sqrt(X * X + Y * Y + Z * Z);
}

template <typename T>
FORCEINLINE T Vector<T, 3>::length_squared() const
{
    return X * X + Y * Y + Z * Z;
}

template <typename T>
FORCEINLINE bool Vector<T, 3>::is_zero() const
{
    return X == 0 && Y == 0 && Z == 0;
}

template <typename T>
FORCEINLINE bool Vector<T, 3>::is_nearly_zero(T tolerance) const
{
    return math::abs(X) <= tolerance && math::abs(Y) <= tolerance && math::abs(Z) <= tolerance;
}

template <typename T>
FORCEINLINE bool Vector<T, 3>::is_normalized() const
{
    return (math::abs(1.f - length_squared()) < THRESH_VECTOR_NORMALIZED);
}

} // namespace math
} // namespace skr