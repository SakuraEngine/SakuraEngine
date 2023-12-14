#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"

namespace skr::container
{
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename TS, typename Allocator>
struct SparseHashMapMemory : public SparseHashSetMemory<KVPair<K, V>, TBitBlock, THash, THasher, TComparer, AllowMultiKey, TS, Allocator> {
    using KeyType   = K;
    using ValueType = V;

    using Super = SparseHashSetMemory<KVPair<K, V>, TBitBlock, THash, THasher, TComparer, AllowMultiKey, TS, Allocator>;

    using Super::Super;
};
} // namespace skr::container