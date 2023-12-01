#pragma once
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"

namespace skr
{
template <typename T>
using SparseArray = container::SparseArray<T, uint64_t, SkrAllocator>;
}