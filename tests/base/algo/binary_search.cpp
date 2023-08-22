#include "SkrTestFramework/framework.hpp"

#include "SkrBase/algo/binary_search.hpp"

TEST_CASE("test binary search")
{
    using namespace skr;
    constexpr int32_t kArrSize = 1000;

    int32_t full_one[kArrSize], full_zero[kArrSize], increasing[kArrSize];
    for (int32_t i = 0; i < kArrSize; ++i)
    {
        full_one[i]   = 1;
        full_zero[i]  = 0;
        increasing[i] = i;
    }
    int32_t* full_one_begin   = full_one;
    int32_t* full_one_end     = full_one + kArrSize;
    int32_t* full_zero_begin  = full_zero;
    int32_t* full_zero_end    = full_zero + kArrSize;
    int32_t* increasing_begin = increasing;
    int32_t* increasing_end   = increasing + kArrSize;

    SUBCASE("lower bound")
    {
        REQUIRE_EQ(algo::lower_bound(full_one_begin, full_one_end, 1), full_one_begin);
        REQUIRE_EQ(algo::lower_bound(full_one_begin, full_one_end, 0), full_one_begin);
        REQUIRE_EQ(algo::lower_bound(full_zero_begin, full_zero_end, 0), full_zero_begin);
        REQUIRE_EQ(algo::lower_bound(full_zero_begin, full_zero_end, 1), full_zero_end);
        REQUIRE_EQ(algo::lower_bound(increasing_begin, increasing_end, -1), increasing_begin);
        REQUIRE_EQ(algo::lower_bound(increasing_begin, increasing_end, 0), increasing_begin);
        REQUIRE_EQ(algo::lower_bound(increasing_begin, increasing_end, kArrSize - 1), increasing_end - 1);
        REQUIRE_EQ(algo::lower_bound(increasing_begin, increasing_end, kArrSize), increasing_end);
        REQUIRE_EQ(algo::lower_bound(increasing_begin, increasing_end, kArrSize / 2), increasing_begin + kArrSize / 2);
    }

    SUBCASE("upper bound")
    {
        REQUIRE_EQ(algo::upper_bound(full_one_begin, full_one_end, 1), full_one_end);
        REQUIRE_EQ(algo::upper_bound(full_one_begin, full_one_end, 0), full_one_begin);
        REQUIRE_EQ(algo::upper_bound(full_zero_begin, full_zero_end, 0), full_zero_end);
        REQUIRE_EQ(algo::upper_bound(full_zero_begin, full_zero_end, 1), full_zero_end);
        REQUIRE_EQ(algo::upper_bound(increasing_begin, increasing_end, -1), increasing_begin);
        REQUIRE_EQ(algo::upper_bound(increasing_begin, increasing_end, 0), increasing_begin + 1);
        REQUIRE_EQ(algo::upper_bound(increasing_begin, increasing_end, kArrSize - 1), increasing_end);
        REQUIRE_EQ(algo::upper_bound(increasing_begin, increasing_end, kArrSize), increasing_end);
        REQUIRE_EQ(algo::upper_bound(increasing_begin, increasing_end, kArrSize / 2), increasing_begin + kArrSize / 2 + 1);
    }

    SUBCASE("binary search")
    {
        REQUIRE_EQ(algo::binary_search(full_one_begin, full_one_end, 1), full_one_begin);
        REQUIRE_EQ(algo::binary_search(full_one_begin, full_one_end, 0), full_one_end);
        REQUIRE_EQ(algo::binary_search(full_zero_begin, full_zero_end, 1), full_zero_end);
        REQUIRE_EQ(algo::binary_search(full_zero_begin, full_zero_end, 0), full_zero_begin);
        REQUIRE_EQ(algo::binary_search(increasing_begin, increasing_end, -1), increasing_end);
        REQUIRE_EQ(algo::binary_search(increasing_begin, increasing_end, 0), increasing_begin);
        REQUIRE_EQ(algo::binary_search(increasing_begin, increasing_end, kArrSize - 1), increasing_end - 1);
        REQUIRE_EQ(algo::binary_search(increasing_begin, increasing_end, kArrSize), increasing_end);
        REQUIRE_EQ(algo::binary_search(increasing_begin, increasing_end, kArrSize / 2), increasing_begin + kArrSize / 2);
    }
}