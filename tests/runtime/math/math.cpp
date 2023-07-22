#include "SkrRT/misc/types.h"
#include "SkrRT/misc/traits.hpp"
#include "SkrRT/math/quat.h"
#include "SkrRT/containers/string.hpp"

#include "SkrTestFramework/framework.hpp"

class CommonMathTests
{

};

TEST_CASE_METHOD(CommonMathTests, "AssumeAligned")
{
    alignas(16) float a[4] = {1, 2, 3, 4};
    auto ptr = skr::assume_aligned<16>(a);
    EXPECT_EQ(ptr, a);
}

TEST_CASE_METHOD(CommonMathTests, "QuatEuler")
{
    skr_rotator_t euler{0, 80.f, 15.f};
    auto quat = skr::math::load(euler);
    skr_rotator_t loaded;
    skr::math::store(quat, loaded);
    EXPECT_NEAR(euler.pitch, loaded.pitch, 0.001);
    EXPECT_NEAR(euler.yaw, loaded.yaw, 0.001);
    EXPECT_NEAR(euler.roll, loaded.roll, 0.001);
}

TEST_CASE_METHOD(CommonMathTests, "MD5")
{
    // "MD5 Hash Generator" ->
    // 992927e5b1c8a237875c70a302a34e22
    skr_md5_t MD5 = {};
    auto string = u8"MD5 Hash Generator";
    skr_make_md5(string, (uint32_t)strlen((const char*)string), &MD5);
    skr_md5_t MD5_2 = {};
    auto formatted = skr::format(u8"{}", MD5);
    skr_parse_md5(formatted.u8_str(), &MD5_2);
    EXPECT_EQ(MD5, MD5_2);
    EXPECT_EQ(formatted, u8"992927e5b1c8a237875c70a302a34e22");
}