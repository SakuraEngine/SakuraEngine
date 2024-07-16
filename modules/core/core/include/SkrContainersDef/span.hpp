#pragma once
#include "SkrBase/types.h"
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/span.hpp"
#include "SkrContainersDef/vector.hpp"

namespace skr
{
template <typename T, size_t Extent = skr::container::kDynamicExtent>
using span = container::Span<T, size_t, Extent>;
}