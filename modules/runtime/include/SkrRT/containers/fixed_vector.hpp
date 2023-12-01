#pragma once
// TODO: REMOVE EASTL
#include "EASTL/fixed_vector.h"

namespace skr
{
template <typename T, size_t N>
using fixed_vector = eastl::fixed_vector<T, N>;
}