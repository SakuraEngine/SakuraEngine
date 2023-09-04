#pragma once
#include "SkrBase/containers/misc/span.hpp"

namespace skr
{
template <typename T>
using Span = container::Span<T, size_t>;
}