#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"

namespace skr::container
{
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename TS, typename Allocator>
struct SparseHashMapMemory : public SparseHashSetMemory<KVPair<K, V>, TBitBlock, THash, THasher, TComparer, AllowMultiKey, TS, Allocator> {
    using Super = SparseHashSetMemory<KVPair<K, V>, TBitBlock, THash, THasher, TComparer, AllowMultiKey, TS, Allocator>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using typename Super::HashType;
    using typename Super::HasherType;
    using typename Super::KeyType;
    using typename Super::KeyMapperType;
    using typename Super::ComparerType;
    using typename Super::SetDataType;
    using typename Super::SetStorageType;
    using Super::allow_multi_key;

    // sparse hash map configure
    using MapKeyType   = K;
    using MapValueType = V;
    using MapDataType  = KVPair<K, V>;

    // ctor & dtor
    inline SparseHashMapMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
    }
    inline ~SparseHashMapMemory() noexcept = default;

    // copy & move
    inline SparseHashMapMemory(const SparseHashMapMemory& other, AllocatorCtorParam param) noexcept
        : Super(other, std::move(param))
    {
    }
    inline SparseHashMapMemory(SparseHashMapMemory&& other) noexcept
        : Super(std::move(other))
    {
    }

    // assign & move assign
    inline SparseHashMapMemory& operator=(const SparseHashMapMemory& other) noexcept
    {
        Super::operator=(other);
        return *this;
    }
    inline SparseHashMapMemory& operator=(SparseHashMapMemory&& other) noexcept
    {
        Super::operator=(std::move(other));
        return *this;
    }
};
} // namespace skr::container