#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_multi.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
using MultiSetMemoryBase = container::SparseHashSetMemoryBase<uint64_t>;

template <typename T, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using MultiSet = container::MultiSparseHashSet<container::SparseHashSetMemory<
T,                  /*element Type*/
uint64_t,           /*BitBlock Type*/
HashTraits,         /*Hasher Traits*/
MultiSetMemoryBase, /*base*/
Allocator>>;        /*Allocator Type*/

template <typename T, uint64_t kCount, typename HashTraits = container::HashTraits<T>>
using FixedMultiSet = container::MultiSparseHashSet<container::FixedSparseHashSetMemory<
T,                 /*element Type*/
uint64_t,          /*BitBlock Type*/
HashTraits,        /*Hasher Traits*/
kCount,            /*Fixed Count*/
MultiSetMemoryBase /*base*/
>>;

template <typename T, uint64_t kInlineCount, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using InlineMultiSet = container::MultiSparseHashSet<container::InlineSparseHashSetMemory<
T,                  /*element Type*/
uint64_t,           /*BitBlock Type*/
HashTraits,         /*Hasher Traits*/
kInlineCount,       /*Inline Count*/
MultiSetMemoryBase, /*base*/
Allocator>>;        /*Allocator Type*/
} // namespace skr