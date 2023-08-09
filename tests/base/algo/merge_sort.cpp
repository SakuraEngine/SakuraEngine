#include "SkrTestFramework/framework.hpp"

#include "SkrBase/algo/merge_sort.hpp"

TEST_CASE("test merge sort")
{
    using namespace skr;

    constexpr size_t kArrSize = 99999;

    int32_t* increasing = new int32_t[kArrSize];
    int32_t* decreasing = new int32_t[kArrSize];
    int32_t* random     = new int32_t[kArrSize];

    for (size_t i = 0; i < kArrSize; ++i)
    {
        increasing[i] = static_cast<int32_t>(i);
        decreasing[i] = static_cast<int32_t>(kArrSize - i - 1);
        random[i]     = static_cast<int32_t>(rand());
    }
    int32_t* increasing_begin = increasing;
    int32_t* increasing_end   = increasing + kArrSize;
    int32_t* decreasing_begin = decreasing;
    int32_t* decreasing_end   = decreasing + kArrSize;
    int32_t* random_begin     = random;
    int32_t* random_end       = random + kArrSize;

    algo::merge_sort(increasing_begin, increasing_end);
    algo::merge_sort(decreasing_begin, decreasing_end);
    algo::merge_sort(random_begin, random_end);

    REQUIRE(algo::is_sorted(increasing_begin, increasing_end));
    REQUIRE(algo::is_sorted(decreasing_begin, decreasing_end));
    REQUIRE(algo::is_sorted(random_begin, random_end));

    delete[] increasing;
    delete[] decreasing;
    delete[] random;
}