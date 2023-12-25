#pragma once
#include "SkrBase/containers/sparse_array/sparse_array_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"

namespace skr
{
template <typename T, typename Allocator = SkrAllocator_New>
using SparseVector = container::SparseArray<container::SparseArrayMemory<
T,        /*type*/
uint64_t, /*bit block type*/
uint64_t, /*size type*/
Allocator /*allocator type*/
>>;

template <typename T, uint64_t kCount>
using FixedSparseVector = container::SparseArray<container::FixedSparseArrayMemory<
T,        /*type*/
uint64_t, /*bit block type*/
uint64_t, /*size type*/
kCount    /*count*/
>>;

template <typename T, uint64_t kInlineCount, typename Allocator = SkrAllocator_New>
using InlineSparseVector = container::SparseArray<container::InlineSparseArrayMemory<
T,            /*type*/
uint64_t,     /*bit block type*/
uint64_t,     /*size type*/
kInlineCount, /*count*/
Allocator     /*allocator type*/
>>;

} // namespace skr