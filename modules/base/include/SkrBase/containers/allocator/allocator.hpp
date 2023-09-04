#pragma once
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/config.h"
#include "SkrBase/tools/assert.h"
#include "SkrBase/memory.hpp"
#include "SkrBase/tools/assert.h"

namespace skr::container
{
// TODO. container specific allocator
// [Array]
//  1. grow/shrink policy
//  2. alloc/free/realloc
// [SparseArray]
//  1. [Array] grow/shrink policy
//  2. [Array] alloc/free/realloc
//  3. resize_bit_array
// [SparseHashSet]
//  1. [Array] grow/shrink policy
//  2. [Array] alloc/free/realloc
//  3. [SparseArray] resize_bit_bucket
//  4. resize_hash_bucket
// [SparseHashMap]
//  same as [SparseHashSet]

template <typename TDerived, typename TS>
struct AllocTemplate {
    using SizeType = TS;

    // impl it
    // SKR_INLINE void  free_raw(void* p, SizeType align) const;
    // SKR_INLINE void* alloc_raw(SizeType size, SizeType align) const;
    // SKR_INLINE void* realloc_raw(void* p, SizeType size, SizeType align) const;

    // helper
    template <typename T>
    SKR_INLINE void free(T* p) const
    {
        static_cast<const TDerived*>(this)->free_raw(p, alignof(T));
    }
    template <typename T>
    SKR_INLINE T* alloc(SizeType size) const
    {
        return (T*)static_cast<const TDerived*>(this)->alloc_raw(size * sizeof(T), alignof(T));
    }
    template <typename T>
    SKR_INLINE T* realloc(T* p, SizeType size) const
    {
        return (T*)static_cast<const TDerived*>(this)->realloc_raw(p, size * sizeof(T), alignof(T));
    }

    // size > capacity, calc grow
    SKR_INLINE SizeType get_grow(SizeType size, SizeType capacity) const
    {
        constexpr SizeType first_grow    = 4;
        constexpr SizeType constant_grow = 16;

        SKR_ASSERT(size > capacity && size > 0);

        // init data
        SizeType result = first_grow;

        // calc grow
        if (capacity || size > first_grow)
        {
            result = size + 3 * size / 8 + constant_grow;
        }

        // handle num over flow
        if (size > result)
            result = std::numeric_limits<SizeType>::max();

        return result;
    }
    // size < capacity, calc shrink
    SKR_INLINE SizeType get_shrink(SizeType size, SizeType capacity) const
    {
        SKR_ASSERT(size < capacity);

        SizeType result;
        if ((3 * size < 2 * capacity) && (capacity - size > 64 || !size))
        {
            result = size;
        }
        else
        {
            result = capacity;
        }

        return result;
    }
};
} // namespace skr::container