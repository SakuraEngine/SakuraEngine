#pragma once
#include "SkrBase/containers/sparse_vector/sparse_vector_memory.hpp"
#include "SkrContainers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector.hpp"

namespace skr
{
template <typename T, typename Allocator = SkrAllocator>
using SparseVector = container::SparseVector<container::SparseVectorMemory<
T,        /*type*/
uint64_t, /*bit block type*/
uint64_t, /*size type*/
Allocator /*allocator type*/
>>;

template <typename T, uint64_t kCount>
using FixedSparseVector = container::SparseVector<container::FixedSparseVectorMemory<
T,        /*type*/
uint64_t, /*bit block type*/
uint64_t, /*size type*/
kCount    /*count*/
>>;

template <typename T, uint64_t kInlineCount, typename Allocator = SkrAllocator>
using InlineSparseVector = container::SparseVector<container::InlineSparseVectorMemory<
T,            /*type*/
uint64_t,     /*bit block type*/
uint64_t,     /*size type*/
kInlineCount, /*count*/
Allocator     /*allocator type*/
>>;

} // namespace skr