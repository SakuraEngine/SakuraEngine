#pragma once
#include "SkrMemory/memory.h"
#include <list>

namespace skr
{

template<typename T, typename Alloc = skr_stl_allocator<T>>
using stl_list = std::list<T, Alloc>;

} // namespace skr