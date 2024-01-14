#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrContainers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_multi.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename K, typename V, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using MultiUMap = container::MultiSparseHashMap<container::SparseHashMapMemory<
K,           /*Key Type*/
V,           /*Value Type*/
uint64_t,    /*BitBlock Type*/
HashTraits,  /*Hash Traits*/
uint64_t,    /*Size Type*/
Allocator>>; /*Allocator Type*/

template <typename K, typename V, uint64_t kCount, typename HashTraits = container::HashTraits<K>>
using FixedMultiUMap = container::MultiSparseHashMap<container::FixedSparseHashMapMemory<
K,          /*Key Type*/
V,          /*Value Type*/
uint64_t,   /*BitBlock Type*/
HashTraits, /*Hasher Traits*/
uint64_t,   /*Size Type*/
kCount      /*Count*/
>>;

template <typename K, typename V, uint64_t kInlineCount, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using InlineMultiUMap = container::MultiSparseHashMap<container::InlineSparseHashMapMemory<
K,            /*Key Type*/
V,            /*Value Type*/
uint64_t,     /*BitBlock Type*/
HashTraits,   /*Hasher Traits*/
uint64_t,     /*Size Type*/
kInlineCount, /*Inline Count*/
Allocator>>;  /*Allocator Type*/
} // namespace skr