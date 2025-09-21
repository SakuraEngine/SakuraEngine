#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"

namespace skr
{
using MapMemoryBase = container::SparseHashMapMemoryBase<uint64_t>;

template <typename K, typename V, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using Map [[sfinal_alias]] = container::SparseHashMap<container::SparseHashMapMemory<
    K,             /*Key Type*/
    V,             /*Value Type*/
    uint64_t,      /*BitBlock Type*/
    HashTraits,    /*Hash Traits*/
    MapMemoryBase, /*Size Type*/
    Allocator>>;   /*Allocator Type*/

template <typename K, typename V, uint64_t kCount, typename HashTraits = container::HashTraits<K>>
using FixedMap [[sfinal_alias]] = container::SparseHashMap<container::FixedSparseHashMapMemory<
    K,            /*Key Type*/
    V,            /*Value Type*/
    uint64_t,     /*BitBlock Type*/
    HashTraits,   /*Hasher Traits*/
    kCount,       /*Count*/
    MapMemoryBase /*Size Type*/
    >>;

template <typename K, typename V, uint64_t kInlineCount, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using InlineMap [[sfinal_alias]] = container::SparseHashMap<container::InlineSparseHashMapMemory<
    K,             /*Key Type*/
    V,             /*Value Type*/
    uint64_t,      /*BitBlock Type*/
    HashTraits,    /*Hasher Traits*/
    kInlineCount,  /*Inline Count*/
    MapMemoryBase, /*Size Type*/
    Allocator>>;   /*Allocator Type*/
} // namespace skr