#pragma once
#include "SkrBase/config.h"
#include "binary_search.hpp"
#include "rotate.hpp"
#include <algorithm>
#include "SkrBase/misc/swap.hpp"

// merge sort help
namespace skr::algo::__help
{
template <class T, class TP>
SKR_INLINE void merge(T begin, T mid, T end, TP&& p)
{
    T l_start = begin;
    T r_start = mid;

    while (l_start < r_start && r_start < end)
    {
        // find insert location for first element of right part head in left part
        T new_l_start = upper_bound(l_start, r_start, *r_start, std::forward<TP>(p));
        l_start       = new_l_start;

        // check done
        if (l_start >= r_start)
            break;

        // find insert location for first element of updated left part head in right part
        // then r_start <---> new_r_start can be rotated to l_start, and l_start <---> r_start can be rotated to r_start
        // and then l_start <---> moved new_r_start will be sorted
        // here is example
        // init state, left part is '.', right part is ','                                       |..........|,,,,,,,,,|
        // step.1 update left part, all '.' is less than any  ','                                |....LLLLLL|,,,,,,,,,|
        // step.2 update right part, all 'R' is less than any 'L'                                |....LLLLLL|RRRRR,,,,|
        // step.3 rotate, new bound is after 'R', because any 'R' is less than 'L' and ','       |....RRRRR|LLLLLL,,,,|
        T      new_r_start = lower_bound(r_start, end, *l_start, std::forward<TP>(p));
        size_t r_offset    = new_r_start - r_start;

        // rotate mid part
        rotate(l_start, (size_t)(new_r_start - l_start), r_offset);

        // now left bound is sorted, we can skip it
        l_start += r_offset + 1;
        r_start = new_r_start;
    }
}
} // namespace skr::algo::__help

// merge sort impl
namespace skr::algo
{
template <typename T, typename TP = Less<>, int MinMergeSubgroupSize = 2>
SKR_INLINE void merge_sort(T begin, T end, TP&& p = TP())
{
    using Swapper = Swap<std::decay_t<decltype(*begin)>>;

    size_t subgroup_start = 0;
    size_t count          = end - begin;

    // step1. sort per group
    if constexpr (MinMergeSubgroupSize > 1)
    {
        if constexpr (MinMergeSubgroupSize > 2)
        {
            // if group size > 3, we use simple bubble-sort.
            do
            {
                size_t group_end = std::min(subgroup_start + MinMergeSubgroupSize, count);
                do
                {
                    for (size_t it = subgroup_start; it < group_end - 1; ++it)
                    {
                        if (p(*(begin + (it + 1)), *(begin + it)))
                        {
                            Swapper::call(*(begin + it), *(begin + (it + 1)));
                        }
                    }
                    --group_end;
                } while (group_end - subgroup_start > 1);

                subgroup_start += MinMergeSubgroupSize;
            } while (subgroup_start < count);
        }
        else
        {
            for (size_t subgroup = 0; subgroup < count; subgroup += 2)
            {
                if ((subgroup + 1) < count && p(*(begin + (subgroup + 1)), *(begin + subgroup)))
                {
                    Swapper::call(*(begin + subgroup), *(begin + (subgroup + 1)));
                }
            }
        }
    }

    // step2. merge groups
    size_t subgroup_size = MinMergeSubgroupSize;
    while (subgroup_size < count)
    {
        auto next_group_size = subgroup_size << 1;
        subgroup_start       = 0;
        do
        {
            auto cur_start = begin + subgroup_start;
            __help::merge(cur_start, cur_start + subgroup_size, cur_start + std::min(next_group_size, count - subgroup_start), std::forward<TP>(p));
            subgroup_start += next_group_size;
        } while (subgroup_start < count);

        subgroup_size = next_group_size;
    }
}
} // namespace skr::algo