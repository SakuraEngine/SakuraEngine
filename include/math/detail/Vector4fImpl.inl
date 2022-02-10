namespace sakura
{
namespace math
{

FORCEINLINE float Vector<float, 4>::operator|(const Vector V) const
{
    return X * V.X + Y * V.Y + Z * V.Z + W * V.W;
}

FORCEINLINE float Vector<float, 4>::dot_product(const Vector A, const Vector B)
{
    return A | B;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator+(const Vector V) const
{
    return Vector(X + V.X, Y + V.Y, Z + V.Z, W + V.W);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator-(const Vector V) const
{
    return Vector(X - V.X, Y - V.Y, Z - V.Z, W - V.W);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator-() const
{
    return Vector(-X, -Y, -Z, -W);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator-(float Bias) const
{
    return Vector(X - Bias, Y - Bias, Z - Bias, W - Bias);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator+(float Bias) const
{
    return Vector(X + Bias, Y + Bias, Z + Bias, W + Bias);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator+=(const Vector V)
{
    X += V.X;
    Y += V.Y;
    Z += V.Z;
    W += V.W;
    return *this;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator-=(const Vector V)
{
    X -= V.X;
    Y -= V.Y;
    Z -= V.Z;
    W -= V.W;
    return *this;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator*=(const Vector V)
{
    *this = *this * V;
    return *this;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator*=(const float Scalar)
{
    *this = *this * Scalar;
    return *this;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator/=(const float Scalar)
{
    *this = *this / Scalar;
    return *this;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator/=(const Vector V)
{
    *this = *this / V;
    return *this;
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator*(float Scale) const
{
    return Vector(X * Scale, Y * Scale, Z * Scale, W * Scale);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator/(float Scale) const
{
    const float RScale = 1.f / Scale;
    return Vector(X * RScale, Y * RScale, Z * RScale, W * RScale);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator*(const Vector V) const
{
    return Vector(X * V.X, Y * V.Y, Z * V.Z, W * V.W);
}

FORCEINLINE Vector<float, 4> Vector<float, 4>::operator/(const Vector V) const
{
    return Vector(X / V.X, Y / V.Y, Z / V.Z, W / V.W);
}

FORCEINLINE bool Vector<float, 4>::operator==(const Vector V) const
{
    return X == V.X && Y == V.Y && Z == V.Z && W == V.W;
}

FORCEINLINE bool Vector<float, 4>::operator!=(const Vector V) const
{
    return X != V.X || Y != V.Y || Z != V.Z || W != V.W;
}

FORCEINLINE bool Vector<float, 4>::equals(const Vector V, float Tolerance) const
{
    return math::abs(X - V.X) <= Tolerance && math::abs(Y - V.Y) <= Tolerance && math::abs(Z - V.Z) <= Tolerance && math::abs(W - V.W) <= Tolerance;
}

FORCEINLINE float Vector<float, 4>::length() const
{
    return math::sqrt(X * X + Y * Y + Z * Z + W * W);
}

FORCEINLINE float Vector<float, 4>::length_squared() const
{
    return X * X + Y * Y + Z * Z + W * W;
}

FORCEINLINE bool Vector<float, 4>::is_zero() const
{
    return X == 0 && Y == 0 && Z == 0;
}

FORCEINLINE bool Vector<float, 4>::is_nearly_zero(float tolerance) const
{
    return math::abs(X) <= tolerance && math::abs(Y) <= tolerance && math::abs(Z) <= tolerance && math::abs(W) <= tolerance;
}

FORCEINLINE bool Vector<float, 4>::is_normalized() const
{
    return (math::abs(1.f - length_squared()) < THRESH_VECTOR_NORMALIZED);
}

} // namespace math
} // namespace sakura