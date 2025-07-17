#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/memory.hpp"
#include "utils.hpp"
#include <type_traits>

namespace skr::algo
{
template <typename T, typename TP>
SKR_INLINE T remove_all(T begin, T end, TP&& p = TP())
{
    if (begin < end)
    {
        auto write     = begin;
        auto read      = begin;
        bool do_remove = p(*read);

        do
        {
            auto run_start = read;
            ++read;

            // collect run scope
            while (read < end && do_remove == p(*read))
            {
                ++read;
            }
            size_t run_len = read - run_start;
            SKR_ASSERT(run_len > 0);

            // do scope op
            if (do_remove)
            {
                // destruct items
                ::skr::memory::destruct(run_start, run_len);
            }
            else
            {
                // move item
                if (write != run_start)
                {
                    ::skr::memory::move(write, run_start, run_len);
                }
                write += run_len;
            }

            // update flag
            do_remove = !do_remove;
        } while (read < end);
        return write;
    }
    return end;
}
template <typename T, typename TP>
SKR_INLINE T remove_all_swap(T begin, T end, TP&& p = TP())
{
    --end;

    while (true)
    {
        // skip items that needn't remove on header
        while (true)
        {
            if (begin > end)
            {
                return begin;
            }
            if (p(*begin))
            {
                break;
            }
            ++begin;
        }

        // skip items that need remove on tail
        while (true)
        {
            if (begin > end)
            {
                return begin;
            }
            if (!p(*end))
            {
                break;
            }
            ::skr::memory::destruct(end);
            --end;
        }

        // swap items at bad pos
        ::skr::memory::destruct(begin);
        ::skr::memory::move(begin, end);

        // update iterator
        ++begin;
        --end;
    }
}
} // namespace skr::algo