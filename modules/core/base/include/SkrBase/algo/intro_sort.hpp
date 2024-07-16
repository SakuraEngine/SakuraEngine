#pragma once
#include "SkrBase/config.h"
#include "utils.hpp"
#include "heap.hpp"
#include <cmath>

// introspective sort
namespace skr::algo
{
template <typename T, typename TP = Less<>>
SKR_INLINE void intro_sort(T begin, T end, TP&& p = {})
{
    using Swapper = Swap<std::decay_t<decltype(*begin)>>;

    // simulate stack
    struct Stack {
        T      min;
        T      max;
        size_t max_depth;
    };

    // too few items
    if (end - begin < 2)
    {
        return;
    }

    // setup stack
    Stack stack[32] = { { begin, end - 1, (size_t)(std::log((float)(end - begin)) * 2.f) } };
    Stack current, inner;

    // start introspective sort
    for (Stack* stack_top = stack; stack_top >= stack; --stack_top)
    {
        // save top
        current = *stack_top;

    LOOP:
        // current element of stack
        size_t count = current.max - current.min + 1;

        // out of depth, use heap sort
        if (current.max_depth == 0)
        {
            heap_sort(current.min, count, std::forward<TP>(p));
            continue;
        }

        // select sort or continue recurve
        if (count <= 8)
        {
            // use select sort
            while (current.max > current.min)
            {
                T max, item;
                // select max item
                for (max = current.min, item = current.min + 1; item <= current.max; ++item)
                {
                    if (p(*max, *item))
                    {
                        max = item;
                    }
                }

                // swap max item
                Swapper::call(*max, *current.max);
                --current.max;
            }
        }
        else
        {
            // swap mid to head
            Swapper::call(*current.min, *(current.min + count / 2));

            // see partition, split by middle value
            inner.min = current.min;     // skip head
            inner.max = current.max + 1; // for skip tail too
            while (true)
            {
                // find item witch bigger than head, from min to max
                while (++inner.min <= current.max && !p(*current.min, *inner.min)) {}
                // find item witch less than head, from max to min
                while (--inner.max > current.min && !p(*inner.max, *current.min)) {}
                // end split items
                if (inner.min > inner.max)
                {
                    break;
                }
                // swap bad point
                Swapper::call(*inner.min, *inner.max);
            }
            // resume mid value
            Swapper::call(*current.min, *inner.max);

            // prepare for recurve
            --current.max_depth;

            // do recurve
            if ((inner.max - current.min - 1) >= (current.max - inner.min))
            {
                // cache left first
                if (current.min + 1 < inner.max)
                {
                    stack_top->min       = current.min;
                    stack_top->max       = inner.max - 1;
                    stack_top->max_depth = current.max_depth;
                    stack_top++;
                }
                // cache right, and reuse current state for loop
                if (current.max > inner.min)
                {
                    current.min = inner.min;
                    goto LOOP;
                }
            }
            else
            {
                // cache right first
                if (current.max > inner.min)
                {
                    stack_top->min       = inner.min;
                    stack_top->max       = current.max;
                    stack_top->max_depth = current.max_depth;
                    stack_top++;
                }
                // cache left, and reuse current state for loop
                if (current.min + 1 < inner.max)
                {
                    current.max = inner.max - 1;
                    goto LOOP;
                }
            }
        }
    }
}
} // namespace skr::algo