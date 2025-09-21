#pragma once
#include <SkrBase/config.h>
#include "SkrCore/memory/memory.h"
#include <list>

namespace skr
{

template <typename T, typename Alloc = skr_stl_allocator<T>>
using stl_list [[sfinal_alias]] = std::list<T, Alloc>;

} // namespace skr