#pragma once
// now we use skr::Swap, yeah!
// #include "SkrBase/containers/sparse_vector/sparse_vector_def.hpp"
// #include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
// #include "SkrBase/containers/sparse_hash_map/kvpair.hpp"

// allocator
#include "skr_test_allocator.hpp"

// vector
#include "SkrBase/containers/vector/vector.hpp"
#include "SkrBase/containers/vector/vector_memory.hpp"

// sparse vector
#include "SkrBase/containers/sparse_vector/sparse_vector.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_memory.hpp"

// sparse hash set
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"

// multi sparse hash set
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_multi.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"

// sparse hash map
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"

// multi sparse hash map
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_multi.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_memory.hpp"

// bit vector
#include "SkrBase/containers/bit_vector/bit_vector.hpp"
#include "SkrBase/containers/bit_vector/bit_vector_memory.hpp"

// ring buffer
#include "SkrBase/containers/ring_buffer/ring_buffer.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_memory.hpp"

namespace skr::test_container
{
using TestSizeType      = uint64_t;
using TestHashType      = uint64_t;
using TestBitBlockType  = uint64_t;
using TestAllocatorType = SkrTestAllocator;

//===========Vector===================================================================
template <typename T>
using Vector = container::Vector<container::VectorMemory<
container::VectorMemoryBase<
T,
TestSizeType>,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedVector = container::Vector<container::FixedVectorMemory<
container::VectorMemoryBase<
T,
TestSizeType>,
kCount>>;

template <typename T, uint64_t kInlineCount>
using InlineVector = container::Vector<container::InlineVectorMemory<
container::VectorMemoryBase<
T,
TestSizeType>,
kInlineCount,
TestAllocatorType>>;

//===========Sparse Vector===================================================================
template <typename T>
using SparseVector = container::SparseVector<container::SparseVectorMemory<
T,
TestBitBlockType,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedSparseVector = container::SparseVector<container::FixedSparseVectorMemory<
T,
TestBitBlockType,
TestSizeType,
kCount>>;

template <typename T, uint64_t kInlineCount>
using InlineSparseVector = container::SparseVector<container::InlineSparseVectorMemory<
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

//===========Multi Sparse Hash Set===================================================================
template <typename T>
using MultiSparseHashSet = container::MultiSparseHashSet<container::SparseHashSetMemory<
T,
TestBitBlockType,
container::HashTraits<T>,
TestSizeType,
TestAllocatorType>>;

template <typename T, uint64_t kCount>
using FixedMultiSparseHashSet = container::MultiSparseHashSet<container::FixedSparseHashSetMemory<
T,
TestBitBlockType,
container::HashTraits<T>,
TestSizeType,
kCount>>;

template <typename T, uint64_t kInlineCount>
using InlineMultiSparseHashSet = container::MultiSparseHashSet<container::InlineSparseHashSetMemory<
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

//===========Multi Sparse Hash Map===================================================================
template <typename K, typename V>
using MultiSparseHashMap = container::MultiSparseHashMap<container::SparseHashMapMemory<
K,
V,
TestBitBlockType,
container::HashTraits<K>,
TestSizeType,
TestAllocatorType>>;

template <typename K, typename V, uint64_t kCount>
using FixedMultiSparseHashMap = container::MultiSparseHashMap<container::FixedSparseHashMapMemory<
K,
V,
TestBitBlockType,
container::HashTraits<K>,
TestSizeType,
kCount>>;

template <typename K, typename V, uint64_t kInlineCount>
using InlineMultiSparseHashMap = container::MultiSparseHashMap<container::InlineSparseHashMapMemory<
K,
V,
TestBitBlockType,
container::HashTraits<K>,
TestSizeType,
kInlineCount,
TestAllocatorType>>;

//===========Bit Vector===================================================================
template <typename TBitBlock>
using BitVector = container::BitVector<container::BitVectorMemory<
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

} // namespace skr::test_container