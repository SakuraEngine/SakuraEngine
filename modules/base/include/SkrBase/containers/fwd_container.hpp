#pragma once
#include <cstdint>

namespace skr
{
using DefaultBitBlock = uint64_t;

// allocator
class PmrAllocator;

// array & bit array & sparse array
template <typename T, typename Alloc>
class Array;
template <typename TBlock, typename Alloc>
class BitArray;
template <typename T, typename TBitBlock, typename Alloc>
class SparseArray;

// unordered set
template <typename T, bool MultiKey = false>
struct USetConfigDefault;
template <typename T, typename TBitBlock, typename Config, typename Alloc>
class USet;
} // namespace skr