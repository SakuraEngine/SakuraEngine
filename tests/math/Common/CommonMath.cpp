#include "gtest/gtest.h"
#include "math/scalarmath.h"
#include "math/vectormath.hpp"
#include "utils/types.h"

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
    auto uvalue = smath_round_up(value, alignment);
    auto dvalue = smath_round_down(value, alignment);
    EXPECT_TRUE(uvalue == 16);
    EXPECT_TRUE(dvalue == 12);
}

TEST(CommonMath, Cast)
{
    skr_float3_t f3 = { 1, 2, 3 };
    skr::math::Vector3f v3 = f3;
    EXPECT_TRUE(v3 == f3);
    EXPECT_FALSE(v3 != f3);
    
    skr_float4_t f4 = { 1, 2, 3, 4 };
    skr::math::Vector4f v4 = f4;
    EXPECT_TRUE(v4 == f4);
    EXPECT_FALSE(v4 != f4);
    skr_float4_t f4_1 = skr::math::Vector4f::vector_one();
    EXPECT_TRUE(f4_1 == skr::math::Vector4f::vector_one());
    EXPECT_FALSE(f4_1 != skr::math::Vector4f::vector_one());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}