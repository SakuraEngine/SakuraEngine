#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include <limits>

namespace skr::container
{
template <typename T, typename TS>
inline TS default_get_grow(TS expect_size, TS current_capacity)
{
    constexpr TS first_grow    = 4;
    constexpr TS constant_grow = 16;

    SKR_ASSERT(expect_size > current_capacity && expect_size > 0);

    // init data
    TS result = first_grow;

    // calc grow
    if (current_capacity || expect_size > first_grow)
    {
        result = expect_size + 3 * expect_size / 8 + constant_grow;
    }

    // handle num over flow
    if (expect_size > result)
        result = std::numeric_limits<TS>::max();

    return result;
}
template <typename T, typename TS>
inline TS default_get_shrink(TS expect_size, TS current_capacity)
{
    SKR_ASSERT(expect_size <= current_capacity);

    TS result;
    if (((3 * expect_size) < (2 * current_capacity)) &&
        ((current_capacity - expect_size) > 64 || !expect_size))
    {
        result = expect_size;
    }
    else
    {
        result = current_capacity;
    }

    return result;
}
}; // namespace skr::container