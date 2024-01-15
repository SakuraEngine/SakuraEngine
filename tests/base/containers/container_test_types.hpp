#pragma once
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"
#include "SkrBase/containers/vector/vector.hpp"
#include "SkrBase/containers/vector/vector_memory.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_memory.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrBase/containers/bit_vector/bit_vector.hpp"
#include "SkrBase/containers/bit_vector/bit_vector_memory.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_memory.hpp"

#include "SkrBase/misc/hash.hpp"
#include "skr_test_allocator.hpp"

namespace skr
{
using TestSizeType      = uint64_t;
using TestHashType      = uint64_t;
using TestBitBlockType  = uint64_t;
using TestAllocatorType = SkrTestAllocator;

//===========Array===================================================================
template <typename T>
using Array = container::Vector<container::VectorMemory<
T,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedArray = container::Vector<container::FixedVectorMemory<
T,
TestSizeType,
kCount>>;

template <typename T, uint64_t kInlineCount>
using InlineArray = container::Vector<container::InlineVectorMemory<
T,
TestSizeType,
kInlineCount,
TestAllocatorType>>;

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

template <typename T, uint64_t kInlineCount>
using InlineSparseArray = container::SparseArray<container::InlineSparseArrayMemory<
T,
TestBitBlockType,
TestSizeType,
kInlineCount,
TestAllocatorType>>;

//===========Sparse Hash Set===================================================================
template <typename T>
using SparseHashSet = container::SparseHashSet<container::SparseHashSetMemory<
T,
TestBitBlockType,
container::HashTraits<T>,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedSparseHashSet = container::SparseHashSet<container::FixedSparseHashSetMemory<
T,
TestBitBlockType,
container::HashTraits<T>,
TestSizeType,
kCount>>;

template <typename T, uint64_t kInlineCount>
using InlineSparseHashSet = container::SparseHashSet<container::InlineSparseHashSetMemory<
T,
TestBitBlockType,
container::HashTraits<T>,
TestSizeType,
kInlineCount,
TestAllocatorType>>;

//===========Sparse Hash Map===================================================================
template <typename K, typename V>
using SparseHashMap = container::SparseHashMap<container::SparseHashMapMemory<
K,
V,
TestBitBlockType,
container::HashTraits<K>,
TestSizeType,
TestAllocatorType>>;

template <typename K, typename V, uint64_t kCount>
using FixedSparseHashMap = container::SparseHashMap<container::FixedSparseHashMapMemory<
K,
V,
TestBitBlockType,
container::HashTraits<K>,
TestSizeType,
kCount>>;

template <typename K, typename V, uint64_t kInlineCount>
using InlineSparseHashMap = container::SparseHashMap<container::InlineSparseHashMapMemory<
K,
V,
TestBitBlockType,
container::HashTraits<K>,
TestSizeType,
kInlineCount,
TestAllocatorType>>;

//===========Bit Array===================================================================
template <typename TBitBlock>
using BitArray = container::BitArray<container::BitArrayMemory<
TBitBlock,
TestSizeType,
TestAllocatorType>>;

//===========Ring Buffer===================================================================
template <typename T>
using RingBuffer = container::RingBuffer<container::RingBufferMemory<
T,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedRingBuffer = container::RingBuffer<container::FixedRingBufferMemory<
T,
TestSizeType,
kCount>>;

template <typename T, uint64_t kInlineCount>
using InlineRingBuffer = container::RingBuffer<container::InlineRingBufferMemory<
T,
TestSizeType,
kInlineCount,
TestAllocatorType>>;

} // namespace skr