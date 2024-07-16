#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/vector/vector_memory.hpp"
#include "SkrBase/containers/vector/vector.hpp"
#include "SkrContainersDef/skr_allocator.hpp"

namespace skr
{
template <typename T, typename Allocator = SkrAllocator>
using Vector = container::Vector<container::VectorMemory<
T,        /*type*/
uint64_t, /*size type*/
Allocator /*allocator*/
>>;

template <typename T, uint64_t kCount>
using FixedVector = container::Vector<container::FixedVectorMemory<
T,        /*type*/
uint64_t, /*size type*/
kCount    /*allocator*/
>>;

template <typename T, uint64_t kCount, typename Allocator = SkrAllocator>
using InlineVector = container::Vector<container::InlineVectorMemory<
T,        /*type*/
uint64_t, /*size type*/
kCount,   /*allocator*/
Allocator /*allocator*/
>>;

template <typename T>
using SerializeConstVector = Vector<T>;
} // namespace skr
