#pragma once
#include "SkrRT/base/config.hpp"
#include "functor.hpp"

namespace skr::algo
{
template <typename T, typename TP = Equal<>>
SKR_INLINE T find(T begin, T end, TP&& p = TP())
{
    for (; begin != end; ++begin)
    {
        if (p(*begin))
            return begin;
    }
    return nullptr;
}

template <typename T, typename TP = Equal<>>
SKR_INLINE T find_last(T begin, T end, TP&& p = TP())
{
    --end;
    for (; begin != end; --end)
    {
        if (p(*end))
            return end;
    }
    return nullptr;
}
} // namespace skr::algo
