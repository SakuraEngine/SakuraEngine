#pragma once
#include "SkrRT/base/config.hpp"
#include "functor.hpp"

namespace skr::algo
{
template <typename T, typename TP = Less<>>
SKR_INLINE bool is_sorted(T begin, T end, TP p = TP())
{
    if (begin < end)
    {
        T next = begin + 1;
        while (next != end)
        {
            if (p(*next, *begin))
            {
                return false;
            }

            ++begin;
            ++next;
        }
    }
    return true;
}
} // namespace skr::algo