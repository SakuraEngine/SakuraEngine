#pragma once
#include "SkrRT/containers_new/skr_allocator.hpp"
#include "SkrBase/containers/array/array.hpp"

namespace skr
{
template <typename T>
using Array = container::Array<T, SkrAllocator>;
}