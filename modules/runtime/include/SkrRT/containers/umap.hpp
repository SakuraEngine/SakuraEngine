#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename K, typename V, typename Hasher = Hash<K>, typename Allocator = SkrAllocator>
using UMap = container::SparseHashMap<container::SparseHashMapMemory<
K,           /*Key Type*/
V,           /*Value Type*/
uint64_t,    /*BitBlock Type*/
uint64_t,    /*Hash Type*/
Hasher,      /*Hasher Type*/
Equal<K>,    /*Comparer Type*/
false,       /*Allow MultiKey*/
uint64_t,    /*Size Type*/
Allocator>>; /*Allocator Type*/

template <typename K, typename V, uint64_t kCount, typename Hasher = Hash<K>>
using FixedUMap = container::SparseHashMap<container::FixedSparseHashMapMemory<
K,        /*Key Type*/
V,        /*Value Type*/
uint64_t, /*BitBlock Type*/
uint64_t, /*Hash Type*/
Hasher,   /*Hasher Type*/
Equal<K>, /*Comparer Type*/
false,    /*Allow MultiKey*/
uint64_t, /*Size Type*/
kCount    /*Count*/
>>;

template <typename K, typename V, uint64_t kInlineCount, typename Hasher = Hash<K>, typename Allocator = SkrAllocator>
using InlineUMap = container::SparseHashMap<container::InlineSparseHashMapMemory<
K,            /*Key Type*/
V,            /*Value Type*/
uint64_t,     /*BitBlock Type*/
uint64_t,     /*Hash Type*/
Hasher,       /*Hasher Type*/
Equal<K>,     /*Comparer Type*/
false,        /*Allow MultiKey*/
uint64_t,     /*Size Type*/
kInlineCount, /*Inline Count*/
Allocator>>;  /*Allocator Type*/
} // namespace skr