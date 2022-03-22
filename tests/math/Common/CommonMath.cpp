#include "gtest/gtest.h"
#include "math/scalarmath.h"

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