#pragma once
#include "SkrBase/config.h"
#include "utils.hpp"

namespace skr::algo
{
// find first value that >= target
template <typename T, typename TF, typename TP = Less<>>
SKR_INLINE T lower_bound(T begin, T end, const TF& v, TP p = TP())
{
    while (end > begin)
    {
        const size_t size           = (end - begin);
        const size_t left_over_size = size % 2;
        auto         middle         = begin + (size / 2);

        bool pass_check = p(*middle, v);

        begin = pass_check ? middle + left_over_size : begin;
        end   = pass_check ? end : middle;
    }
    return begin;
}

// find first value that > target
template <typename T, typename TF, typename TP = Less<>>
SKR_INLINE T upper_bound(T begin, T end, const TF& v, TP p = TP())
{
    while (end > begin)
    {
        const size_t size           = (end - begin);
        const size_t left_over_size = size % 2;
        auto         middle         = begin + (size / 2);

        bool pass_check = !p(v, *middle);

        begin = pass_check ? middle + left_over_size : begin;
        end   = pass_check ? end : middle;
    }
    return begin;
}
template <typename T, typename TF, typename TP = Less<>>
SKR_INLINE T binary_search(T begin, T end, const TF& v, TP p = TP())
{
    auto check_item = lower_bound(begin, end, v, p);
    if (check_item < end)
    {
        if (!p(v, *check_item))
        {
            return check_item;
        }
    }
    return end;
}
} // namespace skr::algo