#include "SkrTestFramework/framework.hpp"

#include "SkrBase/algo/heap.hpp"

TEST_CASE("test heap")
{
    using namespace skr;

    int32_t heap[10]     = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int32_t not_heap[10] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    SUBCASE("is heap")
    {
        REQUIRE(algo::is_heap(heap, 10));
        REQUIRE_FALSE(algo::is_heap(not_heap, 10));
    }

    SUBCASE("heapify")
    {
        algo::heapify(heap, 10);
        REQUIRE(algo::is_heap(heap, 10));
        algo::heapify(not_heap, 10);
        REQUIRE(algo::is_heap(not_heap, 10));
    }

    SUBCASE("heap sort")
    {
        constexpr size_t kArrSize = 999999;

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

        algo::heap_sort(increasing, kArrSize);
        algo::heap_sort(decreasing, kArrSize);
        algo::heap_sort(random, kArrSize);

        REQUIRE(algo::is_sorted(increasing_begin, increasing_end));
        REQUIRE(algo::is_sorted(decreasing_begin, decreasing_end));
        REQUIRE(algo::is_sorted(random_begin, random_end));

        delete[] increasing;
        delete[] decreasing;
        delete[] random;
    }
}