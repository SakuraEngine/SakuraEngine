#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename T, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using Set = container::SparseHashSet<container::SparseHashSetMemory<
T,           /*element Type*/
uint64_t,    /*BitBlock Type*/
HashTraits,  /*Hasher Traits*/
uint64_t,    /*Size Type*/
Allocator>>; /*Allocator Type*/

template <typename T, uint64_t kCount, typename HashTraits = container::HashTraits<T>>
using FixedSet = container::SparseHashSet<container::FixedSparseHashSetMemory<
T,          /*element Type*/
uint64_t,   /*BitBlock Type*/
HashTraits, /*Hasher Traits*/
uint64_t,   /*Size Type*/
kCount>>;   /*Allocator Type*/

template <typename T, uint64_t kInlineCount, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using InlineSet = container::SparseHashSet<container::InlineSparseHashSetMemory<
T,            /*element Type*/
uint64_t,     /*BitBlock Type*/
HashTraits,   /*Hasher Traits*/
uint64_t,     /*Size Type*/
kInlineCount, /*Inline Count*/
Allocator>>;  /*Allocator Type*/
} // namespace skr