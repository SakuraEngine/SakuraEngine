#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_multi.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
using MultiMapMemoryBase = container::SparseHashMapMemoryBase<uint64_t>;

template <typename K, typename V, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using MultiMap = container::MultiSparseHashMap<container::SparseHashMapMemory<
K,                  /*Key Type*/
V,                  /*Value Type*/
uint64_t,           /*BitBlock Type*/
HashTraits,         /*Hash Traits*/
MultiMapMemoryBase, /*Size Type*/
Allocator>>;        /*Allocator Type*/

template <typename K, typename V, uint64_t kCount, typename HashTraits = container::HashTraits<K>>
using FixedMultiMap = container::MultiSparseHashMap<container::FixedSparseHashMapMemory<
K,                 /*Key Type*/
V,                 /*Value Type*/
uint64_t,          /*BitBlock Type*/
HashTraits,        /*Hasher Traits*/
kCount,            /*Count*/
MultiMapMemoryBase /*Size Type*/
>>;

template <typename K, typename V, uint64_t kInlineCount, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using InlineMultiMap = container::MultiSparseHashMap<container::InlineSparseHashMapMemory<
K,                  /*Key Type*/
V,                  /*Value Type*/
uint64_t,           /*BitBlock Type*/
HashTraits,         /*Hasher Traits*/
kInlineCount,       /*Inline Count*/
MultiMapMemoryBase, /*Size Type*/
Allocator>>;        /*Allocator Type*/
} // namespace skr