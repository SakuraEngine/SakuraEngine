#pragma once
#include <SkrBase/config.h>
#include "SkrCore/memory/memory.h"
#include <vector> // IWYU pragma: export

namespace skr
{

template <typename T, typename Alloc = skr_stl_allocator<T>>
using stl_vector [[sfinal_alias]] = std::vector<T, Alloc>;

template <typename T, typename Alloc = skr_stl_allocator<T>>
using STLVector [[sfinal_alias]] = std::vector<T, Alloc>;

} // namespace skr