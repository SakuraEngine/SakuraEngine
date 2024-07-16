#pragma once
#include "SkrCore/memory/memory.h"
#include <deque>

namespace skr
{

template<typename T, typename Alloc = skr_stl_allocator<T>>
using stl_deque = std::deque<T, Alloc>;

} // namespace skr