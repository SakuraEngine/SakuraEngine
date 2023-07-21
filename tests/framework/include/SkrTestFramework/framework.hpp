#pragma once
#include <cmath>
#include <limits>
#include <catch2/catch_test_macros.hpp> // IWYU pragma: export

#define ASSERT_TRUE REQUIRE
#define ASSERT_FALSE(v) REQUIRE(!(v))
#define ASSERT_EQ(a, b) REQUIRE((a) == (b))
#define ASSERT_NE(a, b) REQUIRE((a) != (b))
#define ASSERT_LE(a, b) REQUIRE((a) <= (b))
#define ASSERT_GE(a, b) REQUIRE((a) >= (b))

#define EXPECT_TRUE REQUIRE
#define EXPECT_FALSE(v) REQUIRE(!(v))
#define EXPECT_EQ(a, b) REQUIRE((a) == (b))
#define EXPECT_NE(a, b) REQUIRE((a) != (b))
#define EXPECT_FALSE(v) REQUIRE(!(v))
#define EXPECT_NEAR(a, b, c) REQUIRE(std::abs((a) - (b)) <= (c))

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    test_almost_equal(T x, T y, int ulp = 4)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
        // unless the result is subnormal
        || std::fabs(x - y) < std::numeric_limits<T>::min();
}


int test_framework_dummy();