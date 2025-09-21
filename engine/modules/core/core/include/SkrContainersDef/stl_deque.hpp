#pragma once
#include <SkrBase/config.h>
#include "SkrCore/memory/memory.h"
#include <deque>

namespace skr
{

template <typename T, typename Alloc = skr_stl_allocator<T>>
using stl_deque [[sfinal_alias]] = std::deque<T, Alloc>;

} // namespace skr