#pragma once
#include "platform/configure.h"

#ifdef __cplusplus
    #include <cmath>
namespace sakura
{
namespace math
{
// const values
static constexpr float PI_ = 3.1415926535897932f;
static constexpr float INV_PI_ = 0.31830988618f;
static constexpr float HALF_PI_ = 1.57079632679f;

static constexpr float SQRT_2 = 1.4142135623730950488016887242097f;
static constexpr float SQRT_3 = 1.7320508075688772935274463415059f;
static constexpr float INV_SQRT_2 = 0.70710678118654752440084436210485f;
static constexpr float INV_SQRT_3 = 0.57735026918962576450914878050196f;
static constexpr float HALF_SQRT_2 = 0.70710678118654752440084436210485f;
static constexpr float HALF_SQRT_3 = 0.86602540378443864676372317075294f;

FORCEINLINE float squire(const float v) noexcept
{
    return v * v;
}

FORCEINLINE double squire(const double v) noexcept
{
    return v * v;
}

FORCEINLINE float sqrt(const float v) noexcept
{
    return std::sqrt(v);
}

FORCEINLINE double sqrt(const double v) noexcept
{
    return std::sqrt(v);
}

FORCEINLINE double rsqrt(const double v) noexcept
{
    return 1.0 / sqrt(v);
}

FORCEINLINE float rsqrt(const float v) noexcept
{
    return 1.f / sqrt(v);
}

FORCEINLINE float abs(const float v) noexcept
{
    return std::abs(v);
}

FORCEINLINE double abs(const double v) noexcept
{
    return std::abs(v);
}

FORCEINLINE int32_t abs(const int32_t v) noexcept
{
    return std::abs(v);
}

FORCEINLINE int64_t abs(const int64_t v) noexcept
{
    return std::abs(v);
}

FORCEINLINE double tan(const double v) noexcept
{
    return std::tan(v);
}

FORCEINLINE float tan(const float v) noexcept
{
    return std::tan(v);
}

FORCEINLINE float atan(const float v) noexcept
{
    return std::atan(v);
}

FORCEINLINE double atan(const double v) noexcept
{
    return std::atan(v);
}

FORCEINLINE double atan(const double v0, const double v1) noexcept
{
    return std::atan2(v0, v1);
}

FORCEINLINE float atan2(const float v0, const float v1) noexcept
{
    return std::atan2(v0, v1);
}

FORCEINLINE float sin(const float v) noexcept
{
    return std::sin(v);
}

FORCEINLINE double sin(const double v) noexcept
{
    return std::sin(v);
}

FORCEINLINE float asin(const float v) noexcept
{
    return std::asin(v);
}

FORCEINLINE double asin(const double v) noexcept
{
    return std::asin(v);
}

FORCEINLINE float cos(const float v) noexcept
{
    return std::cos(v);
}

FORCEINLINE double cos(const double v) noexcept
{
    return std::cos(v);
}

FORCEINLINE float acos(const float v) noexcept
{
    return std::acos(v);
}

FORCEINLINE double acos(const double v) noexcept
{
    return std::cos(v);
}

FORCEINLINE void sincos(float& outSin, float& outCos, float v) noexcept
{

    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = (INV_PI_ * 0.5f) * v;
    if (v >= 0.0f)
        quotient = (float)((int)(quotient + 0.5f));
    else
        quotient = (float)((int)(quotient - 0.5f));
    float y = v - (2.0f * PI_) * quotient;

    // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
    float sign;
    if (y > HALF_PI_)
    {
        y = PI_ - y;
        sign = -1.0f;
    }
    else if (y < -HALF_PI_)
    {
        y = -PI_ - y;
        sign = -1.0f;
    }
    else
        sign = +1.0f;

    float y2 = y * y;

    // 11-degree minimax approximation
    outSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    // 10-degree minimax approximation
    float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    outCos = sign * p;
}

    // Note:  We use FASTASIN_HALF_PI instead of HALF_PI_ inside of FastASin(), since it was the value that accompanied the minimax coefficients below.
    // It is important to use exactly the same value in all places inside this function to ensure that FastASin(0.0f) == 0.0f.
    // For comparison:
    //		HALF_PI_				== 1.57079632679f == 0x3fC90FDB
    //		FASTASIN_HALF_PI	== 1.5707963050f  == 0x3fC90FDA
    #define FASTASIN_HALF_PI (1.5707963050f)
/**
 * Computes the ASin of a scalar value.
 *
 * @param Value  input angle
 * @return ASin of Value
 */
FORCEINLINE float fast_asin(float Value)
{
    // Clamp input to [-1,1].
    bool nonnegative = (Value >= 0.0f);
    float x = abs(Value);
    float omx = 1.0f - x;
    if (omx < 0.0f)
    {
        omx = 0.0f;
    }
    float root = sqrt(omx);
    // 7-degree minimax approximation
    float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x + 0.0308918810f) * x - 0.0501743046f) * x + 0.0889789874f) * x - 0.2145988016f) * x + FASTASIN_HALF_PI;
    result *= root; // acos(|x|)
    // acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
    return (nonnegative ? FASTASIN_HALF_PI - result : result - FASTASIN_HALF_PI);
}
    #undef FASTASIN_HALF_PI

constexpr FORCEINLINE int32_t trunc_to_int(float F)
{
    return (int32_t)F;
}

/**
 * Converts a float to an integer value with truncation towards zero.
 * @param F		Floating point value to convert
 * @return		Truncated integer value.
 */
constexpr FORCEINLINE float trunc_to_float(float F)
{
    return (float)trunc_to_int(F);
}

FORCEINLINE float mod(float X, float Y)
{
    if (fabsf(Y) <= 1.e-8f)
    {
        return 0.f;
    }
    const float Div = (X / Y);
    // All floats where abs(f) >= 2^23 (8388608) are whole numbers so do not need truncation, and avoid overflow in TruncToFloat as they get even larger.
    const float Quotient = fabsf(Div) < 8388608.f ? trunc_to_float(Div) : Div;
    float IntPortion = Y * Quotient;

    // Rounding and imprecision could cause IntPortion to exceed X and cause the result to be outside the expected range.
    // For example Fmod(55.8, 9.3) would result in a very small negative value!
    if (fabsf(IntPortion) > fabsf(X))
    {
        IntPortion = X;
    }

    const float Result = X - IntPortion;
    return Result;
}
} // namespace math
} // namespace sakura
#endif // __cplusplus

#define smath_round_up(value, multiple) ((((value) + (multiple)-1) / (multiple)) * (multiple))
#define smath_round_down(value, multiple) ((value) - (value) % (multiple))