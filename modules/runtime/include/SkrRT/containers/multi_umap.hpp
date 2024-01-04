#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename K, typename V, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using MultiUMap = container::SparseHashMapOld<container::SparseHashMapMemory<
K,                                             /*Key Type*/
V,                                             /*Value Type*/
uint64_t,                                      /*BitBlock Type*/
container::KeyTraits<container::KVPair<K, V>>, /*Key Traits*/
HashTraits,                                    /*Hash Traits*/
true,                                          /*Allow MultiKey*/
uint64_t,                                      /*Size Type*/
Allocator>>;                                   /*Allocator Type*/

template <typename K, typename V, uint64_t kCount, typename HashTraits = container::HashTraits<K>>
using FixedMultiUMap = container::SparseHashMapOld<container::FixedSparseHashMapMemory<
K,                                             /*Key Type*/
V,                                             /*Value Type*/
uint64_t,                                      /*BitBlock Type*/
container::KeyTraits<container::KVPair<K, V>>, /*Key Traits*/
HashTraits,                                    /*Hasher Traits*/
true,                                          /*Allow MultiKey*/
uint64_t,                                      /*Size Type*/
kCount                                         /*Count*/
>>;

template <typename K, typename V, uint64_t kInlineCount, typename HashTraits = container::HashTraits<K>, typename Allocator = SkrAllocator>
using InlineMultiUMap = container::SparseHashMapOld<container::InlineSparseHashMapMemory<
K,                                             /*Key Type*/
V,                                             /*Value Type*/
uint64_t,                                      /*BitBlock Type*/
container::KeyTraits<container::KVPair<K, V>>, /*Key Traits*/
HashTraits,                                    /*Hasher Traits*/
true,                                          /*Allow MultiKey*/
uint64_t,                                      /*Size Type*/
kInlineCount,                                  /*Inline Count*/
Allocator>>;                                   /*Allocator Type*/
} // namespace skr