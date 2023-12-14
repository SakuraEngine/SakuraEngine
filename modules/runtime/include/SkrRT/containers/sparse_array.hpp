#pragma once
#include "SkrBase/containers/sparse_array/sparse_array_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"

namespace skr
{
template <typename T>
using SparseArray = container::SparseArray<container::SparseArrayMemory<T, uint64_t, size_t, SkrAllocator_New>>;
}