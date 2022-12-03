#include "gtest/gtest.h"
#include "utils/types.h"
#include "utils/format.hpp"

class CommonMath : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST(CommonMath, CommonMath0)
{
    uint32_t value = 15;
    uint32_t alignment = 4;
    //auto uvalue = smath_round_up(value, alignment);
    ///auto dvalue = smath_round_down(value, alignment);
    //EXPECT_TRUE(uvalue == 16);
    //EXPECT_TRUE(dvalue == 12);
}

TEST(CommonMath, Cast)
{
    skr_float3_t f3 = { 1, 2, 3 };

}

TEST(CommonMath, MD5)
{
    // "MD5 Hash Generator" ->
    // 992927e5b1c8a237875c70a302a34e22
    skr_md5_t MD5 = {};
    const char* string = "MD5 Hash Generator";
    skr_make_md5(string, (uint32_t)strlen(string), &MD5);
    skr_md5_t MD5_2 = {};
    auto formatted = skr::format("{}", MD5);
    skr_parse_md5(formatted.c_str(), &MD5_2);
    EXPECT_EQ(MD5, MD5_2);
    EXPECT_EQ(formatted, "992927e5b1c8a237875c70a302a34e22");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}