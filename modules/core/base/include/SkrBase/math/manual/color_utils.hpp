#pragma once
#include "./rtm_conv.hpp"
#include "./matrix.hpp"
#include "../gen/misc/quat.hpp"

namespace skr
{
inline namespace math
{
enum class EWhitePoint : uint8_t
{
    CIE1931_D65,
    ACES_D60,
    DCI_CalibrationWhite,
    E,
};
enum class EChromaticAdaptationMethod : uint8_t
{
    XYZScaling,
    Bradford,
    CAT02,
    VonKries,
};
enum class EColorSpace : uint8_t
{
    CIEXYZ,
    AdobeRGB,
    Rec709, // unreal default color space
    Rec2020,
    ACES_AP0,
    ACES_AP1,
    P3DCI,
    P3D65,
    REDWideGamut,
    SonySGamut3,
    SonySGamut3Cine,
    AlexaWideGamut,
    CanonCinemaGamut,
    GoProProtuneNative,
    PanasonicVGamut,
    PLASA_E1_54,
};
enum class EColorEncodeMethod : uint8_t
{
    Linear,
    sRGB,
    ST2084,
    Gamma22,
    BT1886,
    Gamma26,
    Cineon,
    REDLog,
    REDLog3G10,
    SLog1,
    SLog2,
    SLog3,
    AlexaV3LogC,
    CanonLog,
    ProTune,
    VLog,
};

// TODO. color encoding TransferFunctions.h

// chromaticities in xy coordinate
struct Chromaticities {
    Chromaticities(EColorSpace color_space);
    Chromaticities(
        const double2& red,
        const double2& green,
        const double2& blue,
        const double2& white_point
    );

    // xyY <=> XYZ
    static double3 xyY_to_XYZ(const double3& xyY);
    static double3 XYZ_to_xyY(const double3& XYZ);

    // xyY <=> LMS
    static double3 XYZ_to_LMS(const double3& XYZ, EChromaticAdaptationMethod method = EChromaticAdaptationMethod::Bradford);
    static double3 LMS_to_XYZ(const double3& LMS, EChromaticAdaptationMethod method = EChromaticAdaptationMethod::Bradford);

    // white point getter
    static double2 get_white_point(EWhitePoint white_point);

    // chromatic adaptation matrix
    // see: http://brucelindbloom.com/index.html?Eqn_ChromAdapt.html
    static double3x3 chromatic_adaptation_matrix(EChromaticAdaptationMethod method);
    static double3x3 chromatic_adaptation_matrix(
        EChromaticAdaptationMethod method,
        const double3&             from_white_in_XYZ,
        const double3&             to_white_in_XYZ
    );

    // Accurate for 1000K < Temp < 15000K
    // [Krystek 1985, "An algorithm to calculate correlated colour temperature"]
    static double2 cct_to_xy_planckian_locus(double cct);

    // Accurate for 4000K < Temp < 25000K
    // in: correlated color temperature
    // out: CIE 1931 chromaticity
    static double2 cct_to_xy_daylight(double cct);

    // cct/duv to xy
    static double2 cct_duv_to_xy(
        double cct,
        double duv
    );

    // white balance matrix
    static double3x3 white_balance_matrix(
        double2                    reference_white,
        double2                    target_white_xy,
        bool                       is_white_balance_mode,
        EChromaticAdaptationMethod method = EChromaticAdaptationMethod::Bradford
    );

    // get planckian isothermal offset
    static float2 planckian_isothermal(float cct, float duv);

    // setup by color space
    void setup_by_color_space(EColorSpace color_space);

    // calc to_xyz matrix
    double3x3 matrix_to_xyz();

    double2 red;
    double2 green;
    double2 blue;
    double2 white_point;
};

// color space
struct ColorSpace {
    ColorSpace(EColorSpace color_space);
    ColorSpace(
        const double2& red,
        const double2& green,
        const double2& blue,
        const double2& white_point
    );

    // convert matrix
    static double3x3 convert_matrix(
        const ColorSpace&          from,
        const ColorSpace&          to,
        EChromaticAdaptationMethod method = EChromaticAdaptationMethod::Bradford
    );

    // convert color
    double3 color_to_XYZ(const double3& color) const;
    double3 color_from_XYZ(const double3& XYZ) const;

    Chromaticities chromaticities;
    double3x3      to_xyz;
    double3x3      from_xyz;
};

// color encoding
struct ColorEncode {
    // srgb
    inline static float SRGB_encode(float v)
    {
        if (v <= 0.04045f / 12.92f)
        {
            return v * 12.92f;
        }
        else
        {
            return pow(v, (1.0f / 2.4f)) * 1.055f - 0.055f;
        }
    }
    inline static float SRGB_decode(float v)
    {
        if (v <= 0.04045f)
        {
            return v / 12.92f;
        }
        else
        {
            return pow((v + 0.055f) / 1.055f, 2.4f);
        }
    }

    // st2084
    inline static float ST2084_encode(float v)
    {
        const float Lp = 10000.0f;
        const float m1 = 2610 / 4096.0f * (1.0f / 4.0f);
        const float m2 = 2523 / 4096.0f * 128.0f;
        const float c1 = 3424 / 4096.0f;
        const float c2 = 2413 / 4096.0f * 32.f;
        const float c3 = 2392 / 4096.0f * 32.f;

        v = pow(v / Lp, m1);
        return pow((c1 + c2 * v) / (c3 * v + 1), m2);
    }
    inline static float ST2084_decode(float v)
    {
        const float Lp = 10000.0f;
        const float m1 = 2610 / 4096.0f * (1.0f / 4.0f);
        const float m2 = 2523 / 4096.0f * 128.0f;
        const float c1 = 3424 / 4096.0f;
        const float c2 = 2413 / 4096.0f * 32.f;
        const float c3 = 2392 / 4096.0f * 32.f;

        const float Vp = pow(v, 1.0f / m2);
        v              = max(0.0f, Vp - c1);
        return pow((v / (c2 - c3 * Vp)), 1.0f / m1) * Lp;
    }

    // gamma 2.2
    inline static float Gamma22_encode(float v)
    {
        return pow(v, 1.0f / 2.2f);
    }
    inline static float Gamma22_decode(float v)
    {
        return pow(v, 2.2f);
    }

    // bt1886
    inline static float BT1886_encode(float v)
    {
        const float L_B      = 0;
        const float L_W      = 1;
        float       Gamma    = 2.40f;
        float       GammaInv = 1.0f / Gamma;
        float       N        = pow(L_W, GammaInv) - pow(L_B, GammaInv);
        float       A        = pow(N, Gamma);
        float       B        = pow(L_B, GammaInv) / N;
        return pow(v / A, GammaInv) - B;
    }
    inline static float BT1886_decode(float Value)
    {
        const float L_B      = 0;
        const float L_W      = 1;
        float       Gamma    = 2.40f;
        float       GammaInv = 1.0f / Gamma;
        float       N        = pow(L_W, GammaInv) - pow(L_B, GammaInv);
        float       A        = pow(N, Gamma);
        float       B        = pow(L_B, GammaInv) / N;
        return A * pow(max(Value + B, 0.0f), Gamma);
    }

    // gamma 2.6
    inline static float Gamma26_encode(float v)
    {
        return pow(v, 1.0f / 2.6f);
    }
    inline static float Gamma26_decode(float v)
    {
        return pow(v, 2.6f);
    }

    // cineon
    inline static float Cineon_encode(float v)
    {
        const float BlackOffset = pow(10.0f, (95.0f - 685.0f) / 300.0f);
        return (685.0f + 300.0f * log10(v * (1.0f - BlackOffset) + BlackOffset)) / 1023.0f;
    }
    inline static float Cineon_decode(float v)
    {
        const float BlackOffset = pow(10.0f, (95.0f - 685.0f) / 300.0f);
        return (pow(10.0f, (1023.0f * v - 685.0f) / 300.0f) - BlackOffset) / (1.0f - BlackOffset);
    }

    // REDLog
    inline static float REDLog_encode(float v)
    {
        const float BlackOffset = pow(10.0f, (0.0f - 1023.0f) / 511.0f);
        return (1023.0f + 511.0f * log10(v * (1.0f - BlackOffset) + BlackOffset)) / 1023.0f;
    }
    inline static float REDLog_decode(float v)
    {
        const float BlackOffset = pow(10.0f, (0.0f - 1023.0f) / 511.0f);
        return (pow(10.0f, (1023.0f * v - 1023.0f) / 511.0f) - BlackOffset) / (1.0f - BlackOffset);
    }

    // REDLog3G10
    inline static float REDLog3G10_encode(float v)
    {
        const float A = 0.224282f;
        const float B = 155.975327f;
        const float C = 0.01f;
        const float G = 15.1927f;

        v += C;

        if (v < 0.0f)
        {
            return v * G;
        }
        else
        {
            return sign(v) * A * log10(B * abs(v) + 1.0f);
        }
    }
    inline static float REDLog3G10_decode(float v)
    {
        const float A = 0.224282f;
        const float B = 155.975327f;
        const float C = 0.01f;
        const float G = 15.1927f;

        if (v < 0.0f)
        {
            v /= G;
        }
        else
        {
            v = sign(v) * (pow(10.0f, abs(v) / A) - 1.0f) / B;
        }

        return v - C;
    }

    // SLog1
    inline static float SLog1_encode(float v)
    {
        v /= 0.9f;
        v = 0.432699f * log10(v + 0.037584f) + 0.616596f + 0.03f;
        return (v * 219.0f + 16.0f) * 4.0f / 1023.0f;
    }
    inline static float SLog1_decode(float v)
    {
        v = ((v * 1023.f) / 4.0f - 16.0f) / 219.0f;
        v = pow(10.0f, (v - 0.616596f - 0.03f) / 0.432699f) - 0.037584f;
        return v * 0.9f;
    }

    // SLog2
    inline static float SLog2_encode(float v)
    {
        if (v >= 0.0f)
        {
            return (64.0f + 876.0f * (0.432699f * log10(155.0f * v / 197.1f + 0.037584f) + 0.646596f)) / 1023.f;
        }
        else
        {
            return (64.0f + 876.0f * (v * 3.53881278538813f / 0.9f) + 0.646596f + 0.030001222851889303f) / 1023.f;
        }
    }
    inline static float SLog2_decode(float v)
    {
        if (v >= (64.f + 0.030001222851889303f * 876.f) / 1023.f)
        {
            return 197.1f * (pow(10.0f, ((v * 1023.f - 64.f) / 876.f - 0.646596f) / 0.432699f) - 0.037584f) / 155.f;
        }
        else
        {
            return 0.9f * ((v * 1023.f - 64.f) / 876.f - 0.030001222851889303f) / 3.53881278538813f;
        }
    }

    // SLog3
    inline static float SLog3_encode(float v)
    {
        if (v >= 0.01125000f)
        {
            return (420.0f + log10((v + 0.01f) / 0.19f) * 261.5f) / 1023.0f;
        }
        else
        {
            return (v * 76.2102946929f / 0.01125f + 95.0f) / 1023.0f;
        }
    }
    inline static float SLog3_decode(float v)
    {
        if (v >= 171.2102946929f / 1023.0f)
        {
            return (pow(10.0f, (v * 1023.0f - 420.f) / 261.5f)) * 0.19f - 0.01f;
        }
        else
        {
            return (v * 1023.0f - 95.0f) * 0.01125000f / (171.2102946929f - 95.0f);
        }
    }

    // AlexaV3LogC
    inline static float ArriAlexaV3LogC_encode(float v)
    {
        const float cut = 0.010591f;
        const float a   = 5.555556f;
        const float b   = 0.052272f;
        const float c   = 0.247190f;
        const float d   = 0.385537f;
        const float e   = 5.367655f;
        const float f   = 0.092809f;

        if (v > cut)
        {
            return c * log10(a * v + b) + d;
        }
        else
        {
            return e * v + f;
        }
    }
    inline static float ArriAlexaV3LogC_decode(float v)
    {
        const float cut = 0.010591f;
        const float a   = 5.555556f;
        const float b   = 0.052272f;
        const float c   = 0.247190f;
        const float d   = 0.385537f;
        const float e   = 5.367655f;
        const float f   = 0.092809f;

        if (v > e * cut + f)
        {
            return (pow(10.0f, (v - d) / c) - b) / a;
        }
        else
        {
            return (v - f) / e;
        }
    }

    // CanonLog
    inline static float CanonLog_encode(float v)
    {
        if (v < 0.0f)
        {
            return -(0.529136f * (log10(-v * 10.1596f + 1.0f)) - 0.0730597f);
        }
        else
        {
            return 0.529136f * log10(10.1596f * v + 1.0f) + 0.0730597f;
        }
    }
    inline static float CanonLog_decode(float v)
    {
        if (v < 0.0730597f)
        {
            return -(pow(10.0f, (0.0730597f - v) / 0.529136f) - 1.0f) / 10.1596f;
        }
        else
        {
            return (pow(10.0f, (v - 0.0730597f) / 0.529136f) - 1.0f) / 10.1596f;
        }
    }

    // ProTune
    inline static float GoProProTune_encode(float v)
    {
        return log(v * 112.f + 1.0f) / log(113.0f);
    }
    inline static float GoProProTune_decode(float v)
    {
        return (pow(113.f, v) - 1.0f) / 112.f;
    }

    // VLog
    inline static float PanasonicVLog_encode(float v)
    {
        const float b = 0.00873f;
        const float c = 0.241514f;
        const float d = 0.598206f;

        if (v < 0.01f)
        {
            return 5.6f * v + 0.125f;
        }
        else
        {
            return c * log10(v + b) + d;
        }
    }
    inline static float PanasonicVLog_decode(float v)
    {
        const float b = 0.00873f;
        const float c = 0.241514f;
        const float d = 0.598206f;

        if (v < 0.181f)
        {
            return (v - 0.125f) / 5.6f;
        }
        else
        {
            return pow(10.0f, (v - d) / c) - b;
        }
    }

    // encode & decode
    inline static float encode(float v, EColorEncodeMethod method)
    {
        switch (method)
        {
        case EColorEncodeMethod::Linear:
            return v;
        case EColorEncodeMethod::sRGB:
            return SRGB_encode(v);
        case EColorEncodeMethod::ST2084:
            return ST2084_encode(v);
        case EColorEncodeMethod::Gamma22:
            return Gamma22_encode(v);
        case EColorEncodeMethod::BT1886:
            return BT1886_encode(v);
        case EColorEncodeMethod::Gamma26:
            return Gamma26_encode(v);
        case EColorEncodeMethod::Cineon:
            return Cineon_encode(v);
        case EColorEncodeMethod::REDLog:
            return REDLog_encode(v);
        case EColorEncodeMethod::REDLog3G10:
            return REDLog3G10_encode(v);
        case EColorEncodeMethod::SLog1:
            return SLog1_encode(v);
        case EColorEncodeMethod::SLog2:
            return SLog2_encode(v);
        case EColorEncodeMethod::SLog3:
            return SLog3_encode(v);
        case EColorEncodeMethod::AlexaV3LogC:
            return ArriAlexaV3LogC_encode(v);
        case EColorEncodeMethod::CanonLog:
            return CanonLog_encode(v);
        case EColorEncodeMethod::ProTune:
            return GoProProTune_encode(v);
        case EColorEncodeMethod::VLog:
            return PanasonicVLog_encode(v);
        default:
            SKR_UNREACHABLE_CODE()
            return 0.0f;
        }
    }
    inline static float decode(float v, EColorEncodeMethod method)
    {
        switch (method)
        {
        case skr::EColorEncodeMethod::Linear:
            return v;
        case EColorEncodeMethod::sRGB:
            return SRGB_decode(v);
        case EColorEncodeMethod::ST2084:
            return ST2084_decode(v);
        case EColorEncodeMethod::Gamma22:
            return Gamma22_decode(v);
        case EColorEncodeMethod::BT1886:
            return BT1886_decode(v);
        case EColorEncodeMethod::Gamma26:
            return Gamma26_decode(v);
        case EColorEncodeMethod::Cineon:
            return Cineon_decode(v);
        case EColorEncodeMethod::REDLog:
            return REDLog_decode(v);
        case EColorEncodeMethod::REDLog3G10:
            return REDLog3G10_decode(v);
        case EColorEncodeMethod::SLog1:
            return SLog1_decode(v);
        case EColorEncodeMethod::SLog2:
            return SLog2_decode(v);
        case EColorEncodeMethod::SLog3:
            return SLog3_decode(v);
        case EColorEncodeMethod::AlexaV3LogC:
            return ArriAlexaV3LogC_decode(v);
        case EColorEncodeMethod::CanonLog:
            return CanonLog_decode(v);
        case EColorEncodeMethod::ProTune:
            return GoProProTune_decode(v);
        case EColorEncodeMethod::VLog:
            return PanasonicVLog_decode(v);
        default:
            SKR_UNREACHABLE_CODE()
            return 0.0f;
        }
    }

    // encode & decode vector
    inline static double3 encode(const double3& v, EColorEncodeMethod method)
    {
        return { encode(v.x, method), encode(v.y, method), encode(v.z, method) };
    }
    inline static double3 decode(const double3& v, EColorEncodeMethod method)
    {
        return { decode(v.x, method), decode(v.y, method), decode(v.z, method) };
    }
    inline static double4 encode(const double4& v, EColorEncodeMethod method)
    {
        return { encode(v.x, method), encode(v.y, method), encode(v.z, method), v.w };
    }
    inline static double4 decode(const double4& v, EColorEncodeMethod method)
    {
        return { decode(v.x, method), decode(v.y, method), decode(v.z, method), v.w };
    }
    inline static float3 encode(const float3& v, EColorEncodeMethod method)
    {
        return { encode(v.x, method), encode(v.y, method), encode(v.z, method) };
    }
    inline static float3 decode(const float3& v, EColorEncodeMethod method)
    {
        return { decode(v.x, method), decode(v.y, method), decode(v.z, method) };
    }
    inline static float4 encode(const float4& v, EColorEncodeMethod method)
    {
        return { encode(v.x, method), encode(v.y, method), encode(v.z, method), v.w };
    }
    inline static float4 decode(const float4& v, EColorEncodeMethod method)
    {
        return { decode(v.x, method), decode(v.y, method), decode(v.z, method), v.w };
    }
};

// luminance
inline float luminance(const float3& color, const float3& factor)
{
    return dot(color, factor);
}
inline float luminance(const float3& color)
{
    return dot(color, float3{ 0.299f, 0.587f, 0.114f });
}
inline double luminance(const double3& color, const double3& factor)
{
    return dot(color, factor);
}
inline double luminance(const double3& color)
{
    return dot(color, double3{ 0.299, 0.587, 0.114 });
}

// rgb <=> hsv
inline double3 rgb_to_hsv(double3 rgb)
{
    double r = rgb.x;
    double g = rgb.y;
    double b = rgb.z;

    double K = 0.f;
    if (g < b)
    {
        ::std::swap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        ::std::swap(r, g);
        K = -2.f / 6.f - K;
    }

    const double chroma = r - (g < b ? g : b);
    double       out_h  = abs(K + (g - b) / (6.f * chroma + 1e-20f));
    double       out_s  = chroma / (r + 1e-20f);
    double       out_v  = r;

    return { out_h, out_s, out_v };
}
inline double3 hsv_to_rgb(const double3& hsv)
{
    double h = hsv.x;
    double s = hsv.y;
    double v = hsv.z;

    if (s == 0.0f)
    {
        return { v };
    }

    h        = fmod(h, 1.0) / (60.0 / 360.0);
    int    i = (int)h;
    double f = h - (double)i;
    double p = v * (1.0f - s);
    double q = v * (1.0f - s * f);
    double t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0:
        return { v, t, p };
    case 1:
        return { q, v, p };
    case 2:
        return { p, v, t };
    case 3:
        return { p, q, v };
    case 4:
        return { t, p, v };
    case 5:
    default:
        return { v, p, q };
    }
}
inline float3 rgb_to_hsv(float3 rgb)
{
    return (float3)rgb_to_hsv((double3)rgb);
}
inline float3 hsv_to_rgb(const float3& hsv)
{
    return (float3)hsv_to_rgb((double3)hsv);
}

} // namespace math
} // namespace skr

namespace skr
{
inline namespace math
{
inline Chromaticities::Chromaticities(EColorSpace color_space)
    : red(kMathNoInit)
    , green(kMathNoInit)
    , blue(kMathNoInit)
    , white_point(kMathNoInit)
{
    setup_by_color_space(color_space);
}
inline Chromaticities::Chromaticities(
    const double2& red,
    const double2& green,
    const double2& blue,
    const double2& white_point
)
    : red(red)
    , green(green)
    , blue(blue)
    , white_point(white_point)
{
}

// xyY <=> XYZ
inline double3 Chromaticities::xyY_to_XYZ(const double3& xyY)
{
    double divisor = max(xyY.y, 1e-10);

    return {
        xyY.x * xyY.z / divisor,
        xyY.z,
        (1.0 - xyY.x - xyY.y) * xyY.z / divisor
    };
}
inline double3 Chromaticities::XYZ_to_xyY(const double3& XYZ)
{
    double divisor = XYZ.x + XYZ.y + XYZ.z;
    if (divisor == 0.0)
    {
        divisor = 1e-10;
    }

    return {
        XYZ.x / divisor,
        XYZ.y / divisor,
        XYZ.y
    };
}

// xyY <=> LMS
inline double3 Chromaticities::XYZ_to_LMS(const double3& XYZ, EChromaticAdaptationMethod method)
{
    return XYZ * chromatic_adaptation_matrix(method);
}
inline double3 Chromaticities::LMS_to_XYZ(const double3& LMS, EChromaticAdaptationMethod method)
{
    return LMS * inverse(chromatic_adaptation_matrix(method));
}

inline double2 Chromaticities::get_white_point(EWhitePoint white_point)
{
    switch (white_point)
    {
    case EWhitePoint::CIE1931_D65:
        return { 0.3127, 0.3290 };
    case EWhitePoint::ACES_D60:
        return { 0.32168, 0.33767 };
    case EWhitePoint::DCI_CalibrationWhite:
        return { 0.314, 0.351 };
    case EWhitePoint::E:
        return { 1.0 / 3.0, 1.0 / 3.0 };
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}

inline double3x3 Chromaticities::chromatic_adaptation_matrix(EChromaticAdaptationMethod method)
{
    switch (method)
    {
    case EChromaticAdaptationMethod::XYZScaling:
        return double3x3::identity();
    case EChromaticAdaptationMethod::Bradford:
        return double3x3::transposed(
            0.8951, 0.2664, -0.1614,
            -0.7502, 1.7135, 0.0367,
            0.0389, -0.0685, 1.0296
        );
    case EChromaticAdaptationMethod::CAT02:
        return double3x3::transposed(
            0.7328, 0.4296, -0.1624,
            -0.7036, 1.6975, 0.0061,
            0.0030, 0.0136, 0.9834
        );
    case EChromaticAdaptationMethod::VonKries:
        return double3x3::transposed(
            0.4002, 0.7076, -0.0808,
            -0.2263, 1.1653, 0.0457,
            0.0000, 0.0000, 0.8252
        );
    default:
        SKR_UNREACHABLE_CODE();
        return double3x3::identity();
    }
}
inline double3x3 Chromaticities::chromatic_adaptation_matrix(
    EChromaticAdaptationMethod method,
    const double3&             from_white_in_XYZ,
    const double3&             to_white_in_XYZ
)
{
    double3x3 to_cone_response = chromatic_adaptation_matrix(method);

    auto from_white_cone_response = from_white_in_XYZ * to_cone_response;
    auto to_white_cone_response   = to_white_in_XYZ * to_cone_response;

    auto scale = to_white_cone_response / from_white_cone_response;

    auto scale_matrix = double3x3::from_scale(scale);

    return to_cone_response * scale_matrix * inverse(to_cone_response);
}

// calc xy from temperature
inline double2 Chromaticities::cct_to_xy_planckian_locus(double cct)
{
    float u = (0.860117757f + 1.54118254e-4f * cct + 1.28641212e-7f * cct * cct) / (1.0f + 8.42420235e-4f * cct + 7.08145163e-7f * cct * cct);
    float v = (0.317398726f + 4.22806245e-5f * cct + 4.20481691e-8f * cct * cct) / (1.0f - 2.89741816e-5f * cct + 1.61456053e-7f * cct * cct);

    float x = 3 * u / (2 * u - 8 * v + 4);
    float y = 2 * v / (2 * u - 8 * v + 4);

    return float2(x, y);
}
inline double2 Chromaticities::cct_to_xy_daylight(double cct)
{
    // Correct for revision of Plank's law
    // This makes 6500 == D65
    cct *= 1.4388 / 1.438;
    float rcp_cct = 1.0 / cct;
    float x       = cct <= 7000 ?
                        0.244063 + (0.09911e3 + (2.9678e6 - 4.6070e9 * rcp_cct) * rcp_cct) * rcp_cct :
                        0.237040 + (0.24748e3 + (1.9018e6 - 2.0064e9 * rcp_cct) * rcp_cct) * rcp_cct;

    float y = -3 * x * x + 2.87 * x - 0.275;

    return float2(x, y);
}
inline float2 Chromaticities::planckian_isothermal(float cct, float duv)
{
    float u = (0.860117757f + 1.54118254e-4f * cct + 1.28641212e-7f * cct * cct) / (1.0f + 8.42420235e-4f * cct + 7.08145163e-7f * cct * cct);
    float v = (0.317398726f + 4.22806245e-5f * cct + 4.20481691e-8f * cct * cct) / (1.0f - 2.89741816e-5f * cct + 1.61456053e-7f * cct * cct);

    float ud = (-1.13758118e9f - 1.91615621e6f * cct - 1.53177f * cct * cct) / square(1.41213984e6f + 1189.62f * cct + cct * cct);
    float vd = (1.97471536e9f - 705674.0f * cct - 308.607f * cct * cct) / square(6.19363586e6f - 179.456f * cct + cct * cct);

    float2 uvd = normalize(float2(ud, vd));

    // Correlated color temperature is meaningful within +/- 0.05
    u += uvd.y * duv * 0.05;
    v += -uvd.x * duv * 0.05;

    float x = 3 * u / (2 * u - 8 * v + 4);
    float y = 2 * v / (2 * u - 8 * v + 4);

    return float2(x, y);
}
inline double2 Chromaticities::cct_duv_to_xy(
    double cct,
    double duv
)
{
    double2 white_point_daylight        = cct_to_xy_daylight(cct);
    double2 white_point_planckian       = cct_to_xy_planckian_locus(cct);
    double2 planckian_isothermal_offset = planckian_isothermal(cct, duv) - white_point_planckian;

    if (cct < 4000)
    {
        return white_point_planckian + planckian_isothermal_offset;
    }
    else
    {
        return white_point_daylight + planckian_isothermal_offset;
    }
}
inline double3x3 Chromaticities::white_balance_matrix(
    double2                    reference_white,
    double2                    target_white_xy,
    bool                       is_white_balance_mode,
    EChromaticAdaptationMethod method
)
{
    if (is_white_balance_mode)
    {
        return chromatic_adaptation_matrix(
            method,
            xyY_to_XYZ({ target_white_xy, 1.0 }),
            xyY_to_XYZ({ reference_white, 1.0 })
        );
    }
    else
    {
        return chromatic_adaptation_matrix(
            method,
            xyY_to_XYZ({ reference_white, 1.0 }),
            xyY_to_XYZ({ target_white_xy, 1.0 })
        );
    }
}

inline void Chromaticities::setup_by_color_space(EColorSpace color_space)
{
    switch (color_space)
    {
    case EColorSpace::CIEXYZ:
        red         = { 1, 0 };
        green       = { 0, 1 };
        blue        = { 0, 0 };
        white_point = get_white_point(EWhitePoint::E);
        break;
    case EColorSpace::AdobeRGB:
        red         = { 0.6400, 0.3300 };
        green       = { 0.2100, 0.7100 };
        blue        = { 0.1500, 0.0600 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::Rec709:
        red         = { 0.64, 0.33 };
        green       = { 0.30, 0.60 };
        blue        = { 0.15, 0.06 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::Rec2020:
        red         = { 0.708, 0.292 };
        green       = { 0.170, 0.797 };
        blue        = { 0.131, 0.046 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::ACES_AP0:
        red         = { 0.7347, 0.2653 };
        green       = { 0.0000, 1.0000 };
        blue        = { 0.0001, -0.0770 };
        white_point = get_white_point(EWhitePoint::ACES_D60);
        break;
    case EColorSpace::ACES_AP1:
        red         = { 0.713, 0.293 };
        green       = { 0.165, 0.830 };
        blue        = { 0.128, 0.044 };
        white_point = get_white_point(EWhitePoint::ACES_D60);
        break;
    case EColorSpace::P3DCI:
        red         = { 0.680, 0.320 };
        green       = { 0.265, 0.690 };
        blue        = { 0.150, 0.060 };
        white_point = get_white_point(EWhitePoint::DCI_CalibrationWhite);
        break;
    case EColorSpace::P3D65:
        red         = { 0.680, 0.320 };
        green       = { 0.265, 0.690 };
        blue        = { 0.150, 0.060 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::REDWideGamut:
        red         = { 0.780308, 0.304253 };
        green       = { 0.121595, 1.493994 };
        blue        = { 0.095612, -0.084589 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::SonySGamut3:
        red         = { 0.730, 0.280 };
        green       = { 0.140, 0.855 };
        blue        = { 0.100, -0.050 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::SonySGamut3Cine:
        red         = { 0.766, 0.275 };
        green       = { 0.225, 0.800 };
        blue        = { 0.089, -0.087 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::AlexaWideGamut:
        red         = { 0.684, 0.313 };
        green       = { 0.221, 0.848 };
        blue        = { 0.0861, -0.1020 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::CanonCinemaGamut:
        red         = { 0.740, 0.270 };
        green       = { 0.170, 1.140 };
        blue        = { 0.080, -0.100 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::GoProProtuneNative:
        red         = { 0.698448, 0.193026 };
        green       = { 0.329555, 1.024597 };
        blue        = { 0.108443, -0.034679 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::PanasonicVGamut:
        red         = { 0.730, 0.280 };
        green       = { 0.165, 0.840 };
        blue        = { 0.100, -0.030 };
        white_point = get_white_point(EWhitePoint::CIE1931_D65);
        break;
    case EColorSpace::PLASA_E1_54:
        red         = { 0.7347, 0.2653 };
        green       = { 0.1596, 0.8404 };
        blue        = { 0.0366, 0.0001 };
        white_point = { 0.4254, 0.4044 };
        break;
    default:
        SKR_UNREACHABLE_CODE();
        break;
    }
}

inline double3x3 Chromaticities::matrix_to_xyz()
{
    // calc unscaled matrix
    double3x3 mat = double3x3{
        { red.x, red.y, 1 - red.x - red.y },
        { green.x, green.y, 1 - green.x - green.y },
        { blue.x, blue.y, 1 - blue.x - blue.y },
    };

    // calc scale
    double3x3 inv_mat   = inverse(mat);
    double3   white_xyz = xyY_to_XYZ({ white_point, 1.0 });
    double3   scale     = mul(white_xyz, inv_mat);

    // apply scale
    mat.axis_x *= scale.x;
    mat.axis_y *= scale.y;
    mat.axis_z *= scale.z;

    return mat;
}

inline ColorSpace::ColorSpace(EColorSpace color_space)
    : chromaticities(color_space)
    , to_xyz(kMathNoInit)
    , from_xyz(kMathNoInit)
{
    to_xyz   = chromaticities.matrix_to_xyz();
    from_xyz = inverse(to_xyz);
}
inline ColorSpace::ColorSpace(
    const double2& red,
    const double2& green,
    const double2& blue,
    const double2& white_point
)
    : chromaticities(red, green, blue, white_point)
    , to_xyz(kMathNoInit)
    , from_xyz(kMathNoInit)
{
    to_xyz   = chromaticities.matrix_to_xyz();
    from_xyz = inverse(to_xyz);
}

// convert matrix
inline double3x3 ColorSpace::convert_matrix(
    const ColorSpace&          from,
    const ColorSpace&          to,
    EChromaticAdaptationMethod method
)
{
    if (method == EChromaticAdaptationMethod::XYZScaling)
    {
        return from.to_xyz * to.from_xyz;
    }
    else
    {
        const auto from_white_xyz = double3(1.0) * from.to_xyz;
        const auto to_white_xyz   = double3(1.0) * to.to_xyz;

        if (all(nearly_equal(from_white_xyz, to_white_xyz, 1.e-7)))
        {
            return from.to_xyz * to.from_xyz;
        }
        else
        {
            auto adaptation_matrix = Chromaticities::chromatic_adaptation_matrix(
                method,
                from_white_xyz,
                to_white_xyz
            );

            return from.to_xyz * adaptation_matrix * to.from_xyz;
        }
    }
}

// convert color
inline double3 ColorSpace::color_to_XYZ(const double3& color) const
{
    return mul(color, to_xyz);
}
inline double3 ColorSpace::color_from_XYZ(const double3& XYZ) const
{
    return mul(XYZ, from_xyz);
}
} // namespace math
} // namespace skr