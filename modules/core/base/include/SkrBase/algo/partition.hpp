#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/swap.hpp"
#include <type_traits>

namespace skr::algo
{
template <typename T, typename TP>
SKR_INLINE T partition(T begin, T end, TP&& p = TP())
{
    using Swapper = Swap<std::decay_t<decltype(*begin)>>;

    while (begin < end)
    {
        // skip true part in head
        while (p(*begin))
        {
            ++begin;
            if (begin == end)
            {
                break;
            }
        }

        // skip false part in tail
        do
        {
            --end;
            if (begin == end)
            {
                break;
            }
        } while (!p(*end));

        // swap bad point
        Swapper::call(*begin, *end);
        ++begin;
        --end;
    }
    return begin;
}
} // namespace skr::algo