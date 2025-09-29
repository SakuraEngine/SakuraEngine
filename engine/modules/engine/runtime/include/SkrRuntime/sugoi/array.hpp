#pragma once
#include "SkrContainers/vector.hpp" // IWYU pragma: export

namespace sugoi
{
using ArrayComponentBase = ::skr::VectorMemoryBase;

template <class T, size_t N>
using ArrayComponent = ::skr::InlineVector<T, N>;
} // namespace sugoi