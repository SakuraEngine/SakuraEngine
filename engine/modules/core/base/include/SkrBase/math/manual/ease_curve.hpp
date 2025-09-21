#pragma once
#include "./rtm_conv.hpp"
#include "../gen/misc/quat.hpp"

namespace skr
{
inline namespace math
{
//! NOTE: see https://easings.net/zh-cn

// sine
inline double ease_in_sine(double t)
{
    return 1 - cos((t * kPi) / 2);
}
inline double ease_out_sine(double t)
{
    return sin((t * kPi) / 2);
}
inline double ease_inout_sine(double t)
{
    return -(cos(kPi * t) - 1) / 2;
}

// quad
inline double ease_in_quad(double t)
{
    return t * t;
}
inline double ease_out_quad(double t)
{
    return 1 - (1 - t) * (1 - t);
}
inline double ease_inout_quad(double t)
{
    return t < 0.5 ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
}

// cubic
inline double ease_in_cubic(double t)
{
    return t * t * t;
}
inline double ease_out_cubic(double t)
{
    return 1 - pow(1 - t, 3);
}
inline double ease_inout_cubic(double t)
{
    return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
}

// quart
inline double ease_in_quart(double t)
{
    return t * t * t * t;
}
inline double ease_out_quart(double t)
{
    return 1 - pow(1 - t, 4);
}
inline double ease_inout_quart(double t)
{
    return t < 0.5 ? 8 * t * t * t * t : 1 - pow(-2 * t + 2, 4) / 2;
}

// quint
inline double ease_in_quint(double t)
{
    return t * t * t * t * t;
}
inline double ease_out_quint(double t)
{
    return 1 - pow(1 - t, 5);
}
inline double ease_inout_quint(double t)
{
    return t < 0.5 ? 16 * t * t * t * t * t : 1 - pow(-2 * t + 2, 5) / 2;
}

// expo
inline double ease_in_expo(double t)
{
    return t == 0 ? 0 : pow(2, 10 * t - 10);
}
inline double ease_out_expo(double t)
{
    return t == 1 ? 1 : 1 - pow(2, -10 * t);
}
inline double ease_inout_expo(double t)
{
    return t == 0 ? 0 :
        t == 1    ? 1 :
        t < 0.5   ? pow(2, 20 * t - 10) / 2 :
                    (2 - pow(2, -20 * t + 10)) / 2;
}

// circ
inline double ease_in_circ(double t)
{
    return 1 - sqrt(1 - t * t);
}
inline double ease_out_circ(double t)
{
    return sqrt(1 - pow(t - 1, 2));
}
inline double ease_inout_circ(double t)
{
    return t < 0.5 ? (1 - sqrt(1 - 4 * t * t)) / 2 :
                     (sqrt(1 - pow(-2 * t + 2, 2)) + 1) / 2;
}

// back
inline double ease_in_back(double t, double s = 1.70158)
{
    return t * t * ((s + 1) * t - s);
}
inline double ease_out_back(double t, double s = 1.70158)
{
    return 1 + (t - 1) * (t - 1) * ((s + 1) * (t - 1) + s);
}
inline double ease_inout_back(double t, double s = 1.70158)
{
    s *= 1.525;
    return t < 0.5 ? (t * t * ((s + 1) * t - s)) / 2 :
                     (1 + (t - 1) * (t - 1) * ((s + 1) * (t - 1) + s)) / 2;
}

// elastic
inline double ease_in_elastic(double t, double a = 1, double p = 0.3)
{
    if (t == 0 || t == 1) return t;
    double s = p / 4;
    t -= 1;
    return -(a * pow(2, 10 * t) * sin((t - s) * (2 * kPi) / p));
}
inline double ease_out_elastic(double t, double a = 1, double p = 0.3)
{
    if (t == 0 || t == 1) return t;
    double s = p / 4;
    return a * pow(2, -10 * t) * sin((t - s) * (2 * kPi) / p) + 1;
}
inline double ease_inout_elastic(double t, double a = 1, double p = 0.3)
{
    if (t == 0 || t == 1) return t;
    double s = p / 4;
    t -= 0.5;
    if (t < 0) return -(a * pow(2, 10 * t) * sin((t - s) * (2 * kPi) / p)) / 2;
    return a * pow(2, -10 * t) * sin((t - s) * (2 * kPi) / p) / 2 + 1;
}

// bounce
double        ease_out_bounce(double t);
inline double ease_in_bounce(double t)
{
    return 1 - ease_out_bounce(1 - t);
}
inline double ease_out_bounce(double t)
{
    const auto n1 = 7.5625;
    const auto d1 = 2.75;

    if (t < 1 / d1)
    {
        return n1 * t * t;
    }
    else if (t < 2 / d1)
    {
        t -= 1.5 / d1;
        return n1 * t * t + 0.75;
    }
    else if (t < 2.5 / d1)
    {
        t -= 2.25 / d1;
        return n1 * t * t + 0.9375;
    }
    else
    {
        t -= 2.625 / d1;
        return n1 * t * t + 0.984375;
    }
}
inline double ease_inout_bounce(double t)
{
    return t < 0.5 ? ease_in_bounce(t * 2) / 2 :
                     (ease_out_bounce(t * 2 - 1) + 1) / 2;
}

} // namespace math
} // namespace skr