#pragma once
#include "SkrBase/containers/sparse_vector/sparse_vector_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector.hpp"

namespace skr
{
template <typename T, typename Allocator = SkrAllocator>
using SparseVector = container::SparseVector<container::SparseVectorMemory<
T,                                           /*type*/
uint64_t,                                    /*bit block type*/
container::SparseVectorMemoryBase<uint64_t>, /*base*/
Allocator                                    /*allocator type*/
>>;

template <typename T, uint64_t kCount>
using FixedSparseVector = container::SparseVector<container::FixedSparseVectorMemory<
T,                                           /*type*/
uint64_t,                                    /*bit block type*/
container::SparseVectorMemoryBase<uint64_t>, /*base*/
kCount                                       /*count*/
>>;

template <typename T, uint64_t kInlineCount, typename Allocator = SkrAllocator>
using InlineSparseVector = container::SparseVector<container::InlineSparseVectorMemory<
T,                                           /*type*/
uint64_t,                                    /*bit block type*/
container::SparseVectorMemoryBase<uint64_t>, /*base*/
kInlineCount,                                /*count*/
Allocator                                    /*allocator type*/
>>;

} // namespace skr