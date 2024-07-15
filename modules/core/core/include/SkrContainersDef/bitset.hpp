#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/bit_set/bit_set.hpp"

namespace skr
{
template <size_t N, typename TBlock = std::conditional_t<N <= 32, uint32_t, uint64_t>>
using Bitset = container::Bitset<N, TBlock>;
}