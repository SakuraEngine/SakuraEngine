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
template <typename TS, typename Iter>
inline void sparse_hash_set_build_bucket(TS* bucket, TS bucket_mask, Iter&& it) noexcept
{
    for (; it; ++it)
    {
        TS& index_ref             = bucket[it->_sparse_hash_set_hash & bucket_mask];
        it->_sparse_hash_set_next = index_ref;
        index_ref                 = it.index();
    }
}
} // namespace skr::container