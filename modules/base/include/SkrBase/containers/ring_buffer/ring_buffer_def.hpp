#pragma once
#include "SkrBase/containers/vector/vector_def.hpp"

namespace skr::container
{
template <typename T, typename TS, bool kConst>
using RingBufferDataRef = VectorDataRef<T, TS, kConst>;
}