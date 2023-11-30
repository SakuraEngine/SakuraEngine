#pragma once
#include "SkrRT/platform/memory.h"
#include <list>

namespace skr
{

template<typename T, typename Alloc = skr_stl_allocator<T>>
using list = std::list<T, Alloc>;

} // namespace skr