#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/swap.hpp"
#include <type_traits>

namespace skr::algo
{
// rotate by complete system of residues
template <typename T, typename TSize>
SKR_INLINE void rotate(T p, TSize count, TSize amount)
{
    using Swapper = Swap<std::decay_t<decltype(*p)>>;

    if (amount != 0)
    {
        TSize loc_gcd = gcd(count, amount);

        // item count of a complete system of residues
        TSize cycle_size = count / loc_gcd;

        // move per complete system of residues
        for (TSize i = 0; i < loc_gcd; ++i)
        {
            auto  buf_object    = std::move(*(p + i));
            TSize index_to_fill = i;
            // move per element of complete system of residues by amount
            for (TSize j = 0; j < cycle_size; ++j)
            {
                index_to_fill = (index_to_fill + amount) % count;
                Swapper::call(*(p + index_to_fill), buf_object);
            }
        }
    }
}
} // namespace skr::algo