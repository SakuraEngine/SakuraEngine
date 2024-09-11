#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/vector/vector_def.hpp"
#include "SkrBase/misc/integer_tools.hpp"
#include <type_traits>

namespace skr::container
{
template <typename T, typename TS, bool kConst>
using StringDataRef = VectorDataRef<T, TS, kConst>;
} // namespace skr::container