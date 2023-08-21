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

// hash set & hash map
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
struct SparseHashSet;
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
struct SparseHashMap;
} // namespace skr