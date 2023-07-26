#pragma once
#include <cmath>
#include <limits>
#include "SkrRT/containers/string.hpp"  // IWYU pragma: export
#include "doctest_fwd.h" // IWYU pragma: export

#ifndef SkrTestLine
#  define SkrTestLine __LINE__
#endif

#ifndef SkrTestConcat
#  define SkrTestConcat(x,y) SkrTestConcatIndirect(x,y)
#endif

#ifndef SkrTestConcatIndirect
#  define SkrTestConcatIndirect(x,y) x##y
#endif

#define ASSERT_TRUE REQUIRE
#define ASSERT_FALSE(v) REQUIRE(!(v))
#define ASSERT_EQ(a, b) REQUIRE(((a) == (b)))
#define ASSERT_NE(a, b) REQUIRE(((a) != (b)))
#define ASSERT_LE(a, b) REQUIRE(((a) <= (b)))
#define ASSERT_GE(a, b) REQUIRE(((a) >= (b)))

#define EXPECT_TRUE REQUIRE
#define EXPECT_FALSE(v) REQUIRE(!(v))
#define EXPECT_EQ(a, b) REQUIRE(((a) == (b)))
#define EXPECT_NE(a, b) REQUIRE(((a) != (b)))
#define EXPECT_FALSE(v) REQUIRE(!(v))
#define EXPECT_NEAR(a, b, c) REQUIRE(std::abs((a) - (b)) <= (c))

#define SKR_TEST_INFO(...) auto SkrTestConcat(scopedTestMsg, SkrTestLine) = skr::format(__VA_ARGS__); INFO(SkrTestConcat(scopedTestMsg, SkrTestLine).c_str());

#define TEST_CASE_METHOD TEST_CASE_FIXTURE

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