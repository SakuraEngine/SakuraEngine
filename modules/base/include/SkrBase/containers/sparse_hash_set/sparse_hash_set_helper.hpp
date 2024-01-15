#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/misc/bit.hpp"
#include "SkrBase/memory/memory_ops.hpp"

// helpers
namespace skr::container
{
template <typename TS>
inline constexpr TS sparse_hash_set_calc_bucket_size(TS capacity) noexcept
{
    constexpr TS min_size_to_hash    = 4;
    constexpr TS basic_bucket_size   = 8;
    constexpr TS avg_bucket_capacity = 2;

    if (capacity >= min_size_to_hash)
    {
        return bit_ceil(TS(capacity / avg_bucket_capacity) + basic_bucket_size);
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
template <typename TS>
inline void sparse_hash_set_clean_bucket(TS* bucket, TS bucket_size) noexcept
{
    std::memset(bucket, 0xFF, sizeof(TS) * bucket_size);
}
template <typename TStorage, typename TS, typename BitCursor>
inline void sparse_hash_set_build_bucket(TStorage* data, TS* bucket, TS bucket_mask, BitCursor&& cursor) noexcept
{
    while (!cursor.reach_end())
    {
        TStorage& data_ref                                 = data[cursor.index()];
        TS&       index_ref                                = bucket[data_ref._sparse_vector_data._sparse_hash_set_hash & bucket_mask];
        data_ref._sparse_vector_data._sparse_hash_set_next = index_ref;
        index_ref                                          = cursor.index();

        cursor.move_next();
    }
}
} // namespace skr::container