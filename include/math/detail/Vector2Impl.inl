namespace sakura
{
namespace math
{
template <typename T>
FORCEINLINE constexpr Vector<T, 2>::Vector(const T x, const T y)
    : m_({ x, y })
{
}

template <typename T>
FORCEINLINE constexpr Vector<T, 2>::Vector(const std::array<T, 2> v)
    : m_(v)
{
}

template <typename T>
FORCEINLINE span<T, 2> Vector<T, 2>::data_view()
{
    return m_;
}

template <typename T>
FORCEINLINE span<const T, 2> Vector<T, 2>::data_view() const
{
    return m_;
}

template <typename T>
FORCEINLINE constexpr Vector<T, 2> Vector<T, 2>::vector_zero()
{
    return Vector<T, 2>(0, 0);
}

template <typename T>
FORCEINLINE constexpr Vector<T, 2> Vector<T, 2>::vector_one()
{
    return Vector<T, 2>(1, 1);
}

template <typename T>
FORCEINLINE T Vector<T, 2>::operator|(const Vector V) const
{
    return X * V.X + Y * V.Y;
}

template <typename T>
FORCEINLINE T Vector<T, 2>::dot_product(const Vector A, const Vector B)
{
    return A | B;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator+(const Vector V) const
{
    return Vector(X + V.X, Y + V.Y);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator-(const Vector V) const
{
    return Vector(X - V.X, Y - V.Y);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator-() const
{
    return Vector(-X, -Y);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator-(T Bias) const
{
    return Vector(X - Bias, Y - Bias);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator+(T Bias) const
{
    return Vector(X + Bias, Y + Bias);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator+=(const Vector V)
{
    X += V.X;
    Y += V.Y;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator-=(const Vector V)
{
    X -= V.X;
    Y -= V.Y;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator*=(const Vector V)
{
    *this = *this * V;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator*=(const T Scalar)
{
    *this = *this * Scalar;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator/=(const T Scalar)
{
    *this = *this / Scalar;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator/=(const Vector V)
{
    *this = *this / V;
    return *this;
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator*(T Scale) const
{
    return Vector(X * Scale, Y * Scale);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator/(T Scale) const
{
    const T RScale = 1.f / Scale;
    return Vector(X * RScale, Y * RScale);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator*(const Vector V) const
{
    return Vector(X * V.X, Y * V.Y);
}

template <typename T>
FORCEINLINE Vector<T, 2> Vector<T, 2>::operator/(const Vector V) const
{
    return Vector(X / V.X, Y / V.Y);
}

template <typename T>
FORCEINLINE bool Vector<T, 2>::operator==(const Vector V) const
{
    return X == V.X && Y == V.Y;
}

template <typename T>
FORCEINLINE bool Vector<T, 2>::operator!=(const Vector V) const
{
    return X != V.X || Y != V.Y;
}

template <typename T>
FORCEINLINE bool Vector<T, 2>::equals(const Vector V, T Tolerance) const
{
    return math::abs(X - V.X) <= Tolerance && math::abs(Y - V.Y) <= Tolerance;
}

template <typename T>
FORCEINLINE T Vector<T, 2>::length() const
{
    return math::sqrt(X * X + Y * Y);
}

template <typename T>
FORCEINLINE T Vector<T, 2>::length_squared() const
{
    return X * X + Y * Y;
}

template <typename T>
FORCEINLINE bool Vector<T, 2>::is_zero() const
{
    return X == 0 && Y == 0;
}

template <typename T>
FORCEINLINE bool Vector<T, 2>::is_nearly_zero(T tolerance) const
{
    return math::abs(X) <= tolerance && math::abs(Y) <= tolerance;
}

template <typename T>
FORCEINLINE bool Vector<T, 2>::is_normalized() const
{
    return (math::abs(1.f - length_squared()) < THRESH_VECTOR_NORMALIZED);
}
} // namespace math
} // namespace sakura