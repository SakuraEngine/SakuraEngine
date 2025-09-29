// test suite for linear algebra operations
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/math.hpp"
#include <type_traits>

TEST_CASE("ColorSpace")
{
    using namespace skr;

    SUBCASE("Adobe RGB")
    {
        ColorSpace adobe_rgb = ColorSpace(EColorSpace::AdobeRGB);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 adobe_rgb_to_xyz = transpose(double3x3{
            { 0.576669, 0.185558, 0.188229 },
            { 0.297345, 0.627364, 0.075291 },
            { 0.027031, 0.070689, 0.991338 } });
        double3x3 xyz_to_adobe_rgb = transpose(double3x3{
            { 2.041588, -0.565007, -0.344731 },
            { -0.969244, 1.875968, 0.041555 },
            { 0.013444, -0.118362, 1.015175 } });

        REQUIRE(nearly_equal(adobe_rgb.from_xyz, xyz_to_adobe_rgb));
        REQUIRE(nearly_equal(adobe_rgb.to_xyz, adobe_rgb_to_xyz));
    }

    SUBCASE("Rec.709")
    {
        ColorSpace rec709 = ColorSpace(EColorSpace::Rec709);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 rec709_to_xyz = transpose(double3x3{
            { 0.412391, 0.357584, 0.180481 },
            { 0.212639, 0.715169, 0.072192 },
            { 0.019331, 0.119195, 0.950532 } });
        double3x3 xyz_to_rec709 = transpose(double3x3{
            { 3.240970, -1.537383, -0.498611 },
            { -0.969244, 1.875968, 0.041555 },
            { 0.055630, -0.203977, 1.056972 } });

        REQUIRE(nearly_equal(rec709.from_xyz, xyz_to_rec709));
        REQUIRE(nearly_equal(rec709.to_xyz, rec709_to_xyz));
    }

    SUBCASE("Rec.2020")
    {
        ColorSpace rec2020 = ColorSpace(EColorSpace::Rec2020);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 rec2020_to_xyz = transpose(double3x3{
            { 0.636958, 0.144617, 0.168881 },
            { 0.262700, 0.677998, 0.059302 },
            { 0.000000, 0.028073, 1.060985 } });
        double3x3 xyz_to_rec2020 = transpose(double3x3{
            { 1.716651, -0.355671, -0.253366 },
            { -0.666684, 1.616481, 0.015769 },
            { 0.017640, -0.042771, 0.942103 } });

        REQUIRE(nearly_equal(rec2020.from_xyz, xyz_to_rec2020));
        REQUIRE(nearly_equal(rec2020.to_xyz, rec2020_to_xyz));
    }

    SUBCASE("P3-D65")
    {
        ColorSpace p3d65 = ColorSpace(EColorSpace::P3D65);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 p3d65_to_xyz = transpose(double3x3{
            { 0.486571, 0.265668, 0.198217 },
            { 0.228975, 0.691739, 0.079287 },
            { -0.000000, 0.045113, 1.043944 } });
        double3x3 xyz_to_p3d65 = transpose(double3x3{
            { 2.493497, -0.931384, -0.402711 },
            { -0.829489, 1.762664, 0.023625 },
            { 0.035846, -0.076172, 0.956885 } });

        REQUIRE(nearly_equal(p3d65.from_xyz, xyz_to_p3d65));
        REQUIRE(nearly_equal(p3d65.to_xyz, p3d65_to_xyz));
    }

    SUBCASE("P3-DCI")
    {
        ColorSpace p3dci = ColorSpace(EColorSpace::P3DCI);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 p3dci_to_xyz = transpose(double3x3{
            { 0.445170, 0.277134, 0.172283 },
            { 0.209492, 0.721595, 0.068913 },
            { -0.000000, 0.047061, 0.907355 } });
        double3x3 xyz_to_p3dci = transpose(double3x3{
            { 2.725394, -1.018003, -0.440163 },
            { -0.795168, 1.689732, 0.022647 },
            { 0.041242, -0.087639, 1.100929 } });

        REQUIRE(nearly_equal(p3dci.from_xyz, xyz_to_p3dci));
        REQUIRE(nearly_equal(p3dci.to_xyz, p3dci_to_xyz));
    }

    SUBCASE("ACES AP0")
    {
        ColorSpace aces_ap0 = ColorSpace(EColorSpace::ACES_AP0);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 ap0_to_xyz = transpose(double3x3{
            { 0.952552, 0.000000, 0.000094 },
            { 0.343966, 0.728166, -0.072133 },
            { 0.000000, 0.000000, 1.008825 } });
        double3x3 xyz_to_ap0 = transpose(double3x3{
            { 1.049811, 0.000000, -0.000097 },
            { -0.495903, 1.373313, 0.098240 },
            { 0.000000, 0.000000, 0.991252 } });

        REQUIRE(nearly_equal(aces_ap0.from_xyz, xyz_to_ap0));
        REQUIRE(nearly_equal(aces_ap0.to_xyz, ap0_to_xyz));
    }

    SUBCASE("ACES AP1")
    {
        ColorSpace aces_ap1 = ColorSpace(EColorSpace::ACES_AP1);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 ap1_to_xyz = transpose(double3x3{
            { 0.662454, 0.134004, 0.156188 },
            { 0.272229, 0.674082, 0.053690 },
            { -0.005575, 0.004061, 1.010339 } });
        double3x3 xyz_to_ap1 = transpose(double3x3{
            { 1.641023, -0.324803, -0.236425 },
            { -0.663663, 1.615332, 0.016756 },
            { 0.011722, -0.008284, 0.988395 } });

        REQUIRE(nearly_equal(aces_ap1.from_xyz, xyz_to_ap1));
        REQUIRE(nearly_equal(aces_ap1.to_xyz, ap1_to_xyz));
    }

    SUBCASE("CIE to Rec2020 with Bradford")
    {
        ColorSpace cie = ColorSpace(EColorSpace::CIEXYZ);
        ColorSpace rec2020 = ColorSpace(EColorSpace::Rec2020);
        double3x3 convert = ColorSpace::convert_matrix(
            cie,
            rec2020,
            EChromaticAdaptationMethod::Bradford);

        // from https://haraldbrendel.com/colorspacecalculator.html
        double3x3 expected = transpose(double3x3{
            { 1.649171, -0.410842, -0.238330 },
            { -0.697274, 1.680811, 0.016463 },
            { 0.020914, -0.047345, 1.026431 } });

        REQUIRE(nearly_equal(convert, expected));
    }
}