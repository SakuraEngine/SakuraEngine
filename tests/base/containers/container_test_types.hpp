#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrBase/containers/array/array.hpp"
#include "SkrBase/containers/array/array_memory.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_memory.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrBase/containers/bit_array/bit_array.hpp"
#include "SkrBase/containers/bit_array/bit_array_memory.hpp"

#include "SkrBase/misc/hash.hpp"
#include "skr_test_allocator.hpp"

namespace skr
{
using TestSizeType      = uint64_t;
using TestHashType      = uint64_t;
using TestBitBlockType  = uint64_t;
using TestAllocatorType = SkrTestAllocator_New;

//===========Array===================================================================
template <typename T>
using Array = container::Array<container::ArrayMemory<
T,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedArray = container::Array<container::FixedArrayMemory<
T,
TestSizeType,
kCount>>;

//===========Sparse Array===================================================================
template <typename T>
using SparseArray = container::SparseArray<container::SparseArrayMemory<
T,
TestBitBlockType,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedSparseArray = container::SparseArray<container::FixedSparseArrayMemory<
T,
TestBitBlockType,
TestSizeType,
kCount>>;

//===========Sparse Hash Set===================================================================
template <typename T>
using SparseHashSet = container::SparseHashSet<container::SparseHashSetMemory<
T,
TestBitBlockType,
TestHashType,
Hash<T>,
Equal<T>,
false,
TestSizeType,
TestAllocatorType>>;

//===========Sparse Hash Map===================================================================
template <typename K, typename V>
using SparseHashMap = container::SparseHashMap<container::SparseHashMapMemory<
K,
V,
TestBitBlockType,
TestHashType,
Hash<K>,
Equal<K>,
false,
TestSizeType,
TestAllocatorType>>;

//===========Bit Array===================================================================
template <typename TBitBlock>
using BitArray = container::BitArray<container::BitArrayMemory<
TBitBlock,
TestSizeType,
TestAllocatorType>>;

} // namespace skr