#pragma once
#include "SkrMemory/memory.h"
#include <vector>

namespace skr
{

template<typename T, typename Alloc = skr_stl_allocator<T>>
using stl_vector = std::vector<T, Alloc>;

} // namespace skr