#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_multi.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename T, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using MultiSet = container::MultiSparseHashSet<container::SparseHashSetMemory<
T,                                            /*element Type*/
uint64_t,                                     /*BitBlock Type*/
HashTraits,                                   /*Hasher Traits*/
container::SparseHashSetMemoryBase<uint64_t>, /*base*/
Allocator>>;                                  /*Allocator Type*/

template <typename T, uint64_t kCount, typename HashTraits = container::HashTraits<T>>
using FixedMultiSet = container::MultiSparseHashSet<container::FixedSparseHashSetMemory<
T,                                            /*element Type*/
uint64_t,                                     /*BitBlock Type*/
HashTraits,                                   /*Hasher Traits*/
container::SparseHashSetMemoryBase<uint64_t>, /*base*/
kCount>>;                                     /*Allocator Type*/

template <typename T, uint64_t kInlineCount, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using InlineMultiSet = container::MultiSparseHashSet<container::InlineSparseHashSetMemory<
T,                                            /*element Type*/
uint64_t,                                     /*BitBlock Type*/
HashTraits,                                   /*Hasher Traits*/
container::SparseHashSetMemoryBase<uint64_t>, /*base*/
kInlineCount,                                 /*Inline Count*/
Allocator>>;                                  /*Allocator Type*/
} // namespace skr