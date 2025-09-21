#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/bit_vector/bit_vector.hpp"
#include "SkrBase/containers/bit_vector/bit_vector_memory.hpp"
#include "SkrContainersDef/skr_allocator.hpp"

namespace skr
{
template <typename Allocator = SkrAllocator>
using BitVector = container::BitVector<container::BitVectorMemory<
    uint64_t,
    uint64_t,
    Allocator 
>>;
} // namespace skr