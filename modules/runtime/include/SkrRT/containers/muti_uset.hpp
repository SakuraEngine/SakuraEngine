#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename T, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using MultiUSet = container::SparseHashSetOld<container::SparseHashSetMemory<
T,                       /*element Type*/
uint64_t,                /*BitBlock Type*/
container::KeyTraits<T>, /*Key Traits*/
HashTraits,              /*Hasher Traits*/
true,                    /*Allow MultiKey*/
uint64_t,                /*Size Type*/
Allocator>>;             /*Allocator Type*/

template <typename T, uint64_t kCount, typename HashTraits = container::HashTraits<T>>
using FixedMultiUSet = container::SparseHashSetOld<container::FixedSparseHashSetMemory<
T,                       /*element Type*/
uint64_t,                /*BitBlock Type*/
container::KeyTraits<T>, /*Key Traits*/
HashTraits,              /*Hasher Traits*/
true,                    /*Allow MultiKey*/
uint64_t,                /*Size Type*/
kCount>>;                /*Allocator Type*/

template <typename T, uint64_t kInlineCount, typename HashTraits = container::HashTraits<T>, typename Allocator = SkrAllocator>
using InlineMultiUSet = container::SparseHashSetOld<container::InlineSparseHashSetMemory<
T,                       /*element Type*/
uint64_t,                /*BitBlock Type*/
container::KeyTraits<T>, /*Key Traits*/
HashTraits,              /*Hasher Traits*/
true,                    /*Allow MultiKey*/
uint64_t,                /*Size Type*/
kInlineCount,            /*Inline Count*/
Allocator>>;             /*Allocator Type*/
} // namespace skr