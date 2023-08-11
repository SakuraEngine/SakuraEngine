#pragma once
#include <cstdint>

namespace skr
{
using DefaultBitBlock = uint64_t;

// allocator
struct PmrAllocator;

// array & bit array & sparse array
template <typename T, typename Alloc>
struct Array;
template <typename TBlock, typename Alloc>
struct BitArray;
template <typename T, typename TBitBlock, typename Alloc>
struct SparseArray;

// unordered set
template <typename T, bool MultiKey = false>
struct SparseHashSetConfigDefault;
template <typename T, typename TBitBlock, typename Config, typename Alloc>
struct SparseHashSet;
} // namespace skr