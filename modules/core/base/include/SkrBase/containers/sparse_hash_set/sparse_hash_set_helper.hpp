#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/misc/bit.hpp"
#include "SkrBase/memory/memory_ops.hpp"

// helpers
namespace skr::container
{
template <typename TSize>
inline constexpr TSize sparse_hash_set_calc_bucket_size(TSize capacity) noexcept
{
    constexpr TSize min_size_to_hash    = 4;
    constexpr TSize basic_bucket_size   = 8;
    constexpr TSize avg_bucket_capacity = 2;

    if (capacity >= min_size_to_hash)
    {
        return bit_ceil(TSize(capacity / avg_bucket_capacity) + basic_bucket_size);
    }
    else if (capacity)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
template <typename TSize>
inline void sparse_hash_set_clean_bucket(TSize* bucket, TSize bucket_size) noexcept
{
    std::memset(bucket, 0xFF, sizeof(TSize) * bucket_size);
}
template <typename TStorage, typename TSize, typename BitCursor>
inline void sparse_hash_set_build_bucket(TStorage* data, TSize* bucket, TSize bucket_mask, BitCursor&& cursor) noexcept
{
    while (!cursor.reach_end())
    {
        TStorage& data_ref                                 = data[cursor.index()];
        TSize&    index_ref                                = bucket[data_ref._sparse_vector_data._sparse_hash_set_hash & bucket_mask];
        data_ref._sparse_vector_data._sparse_hash_set_next = index_ref;
        index_ref                                          = cursor.index();

        cursor.move_next();
    }
}
} // namespace skr::container