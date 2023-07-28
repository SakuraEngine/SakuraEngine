#pragma once
#include <cstdint>

namespace skr
{
using DefaultBitBlock = uint64_t;

// allocator
class PmrAllocator;
using DefaultAllocator = PmrAllocator;

// array & bit array & sparse array
template <typename T, typename Alloc = DefaultAllocator>
class Array;
template <typename TBlock = DefaultBitBlock, typename Alloc = DefaultAllocator>
class BitArray;
template <typename T, typename TBitBlock = DefaultBitBlock, typename Alloc = DefaultAllocator>
class SparseArray;

// unordered set
template <typename T, bool MultiKey = false>
struct USetConfigDefault;
template <typename T, typename TBitBlock = DefaultBitBlock, typename Config = USetConfigDefault<T>, typename Alloc = DefaultAllocator>
class USet;
} // namespace skr