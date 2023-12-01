#pragma once
#include "SkrMemory/memory.h"
#include <deque>

namespace skr
{

template<typename T, typename Alloc = skr_stl_allocator<T>>
using deque = std::deque<T, Alloc>;

} // namespace skr