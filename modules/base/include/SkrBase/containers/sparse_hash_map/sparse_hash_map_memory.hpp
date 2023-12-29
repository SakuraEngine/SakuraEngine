#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"

// sparse hash map memory
namespace skr::container
{
template <typename K, typename V, typename TBitBlock, typename KeyTraits, typename HashTraits, bool AllowMultiKey, typename TS, typename Allocator>
struct SparseHashMapMemory : public SparseHashSetMemory<KVPair<K, V>, TBitBlock, KeyTraits, HashTraits, AllowMultiKey, TS, Allocator> {
    using Super = SparseHashSetMemory<KVPair<K, V>, TBitBlock, KeyTraits, HashTraits, AllowMultiKey, TS, Allocator>;

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
    inline SparseHashMapMemory(const SparseHashMapMemory& other) noexcept
        : Super(other)
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

// fixed sparse hash map memory
namespace skr::container
{
template <typename K, typename V, typename TBitBlock, typename KeyTraits, typename HashTraits, bool AllowMultiKey, typename TS, uint64_t kCount>
struct FixedSparseHashMapMemory : public FixedSparseHashSetMemory<KVPair<K, V>, TBitBlock, KeyTraits, HashTraits, AllowMultiKey, TS, kCount> {
    using Super = FixedSparseHashSetMemory<KVPair<K, V>, TBitBlock, KeyTraits, HashTraits, AllowMultiKey, TS, kCount>;

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
    using typename Super::SetDataType;
    using typename Super::SetStorageType;
    using Super::allow_multi_key;

    // sparse hash map configure
    using MapKeyType   = K;
    using MapValueType = V;
    using MapDataType  = KVPair<K, V>;

    // ctor & dtor
    inline FixedSparseHashMapMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
    }
    inline ~FixedSparseHashMapMemory() noexcept = default;

    // copy & move
    inline FixedSparseHashMapMemory(const FixedSparseHashMapMemory& other) noexcept
        : Super(other)
    {
    }
    inline FixedSparseHashMapMemory(FixedSparseHashMapMemory&& other) noexcept
        : Super(std::move(other))
    {
    }

    // assign & move assign
    inline FixedSparseHashMapMemory& operator=(const FixedSparseHashMapMemory& other) noexcept
    {
        Super::operator=(other);
        return *this;
    }
    inline FixedSparseHashMapMemory& operator=(FixedSparseHashMapMemory&& other) noexcept
    {
        Super::operator=(std::move(other));
        return *this;
    }
};
} // namespace skr::container

// inline hash map memory
namespace skr::container
{
template <typename K, typename V, typename TBitBlock, typename KeyTraits, typename HashTraits, bool AllowMultiKey, typename TS, uint64_t kCount, typename Allocator>
struct InlineSparseHashMapMemory : public InlineSparseHashSetMemory<KVPair<K, V>, TBitBlock, KeyTraits, HashTraits, AllowMultiKey, TS, kCount, Allocator> {
    using Super = InlineSparseHashSetMemory<KVPair<K, V>, TBitBlock, KeyTraits, HashTraits, AllowMultiKey, TS, kCount, Allocator>;

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
    using typename Super::SetDataType;
    using typename Super::SetStorageType;
    using Super::allow_multi_key;

    // sparse hash map configure
    using MapKeyType   = K;
    using MapValueType = V;
    using MapDataType  = KVPair<K, V>;

    // ctor & dtor
    inline InlineSparseHashMapMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
    }
    inline ~InlineSparseHashMapMemory() noexcept = default;

    // copy & move
    inline InlineSparseHashMapMemory(const InlineSparseHashMapMemory& other) noexcept
        : Super(other)
    {
    }
    inline InlineSparseHashMapMemory(InlineSparseHashMapMemory&& other) noexcept
        : Super(std::move(other))
    {
    }

    // assign & move assign
    inline InlineSparseHashMapMemory& operator=(const InlineSparseHashMapMemory& other) noexcept
    {
        Super::operator=(other);
        return *this;
    }
    inline InlineSparseHashMapMemory& operator=(InlineSparseHashMapMemory&& other) noexcept
    {
        Super::operator=(std::move(other));
        return *this;
    }
};
} // namespace skr::container
