#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
using SetMemoryBase = container::SparseHashSetMemoryBase<uint64_t>;

template <typename T, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using Set = container::SparseHashSet<container::SparseHashSetMemory<
T,             /*element Type*/
uint64_t,      /*BitBlock Type*/
HashTraits,    /*Hasher Traits*/
SetMemoryBase, /*base*/
Allocator>>;   /*Allocator Type*/

template <typename T, uint64_t kCount, typename HashTraits = container::HashTraits<T>>
using FixedSet = container::SparseHashSet<container::FixedSparseHashSetMemory<
T,            /*element Type*/
uint64_t,     /*BitBlock Type*/
HashTraits,   /*Hasher Traits*/
kCount,       /*Fixed count*/
SetMemoryBase /*base*/
>>;

template <typename T, uint64_t kInlineCount, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using InlineSet = container::SparseHashSet<container::InlineSparseHashSetMemory<
T,             /*element Type*/
uint64_t,      /*BitBlock Type*/
HashTraits,    /*Hasher Traits*/
kInlineCount,  /*Inline Count*/
SetMemoryBase, /*base*/
Allocator>>;   /*Allocator Type*/
} // namespace skr