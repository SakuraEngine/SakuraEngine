#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_memory.hpp"
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename T>
using USet = container::SparseHashSet<container::SparseHashSetMemory<
T,                                                /*element Type*/
uint64_t,                                         /*BitBlock Type*/
uint64_t,                                         /*Hash Type*/
Hash<typename container::KeyTraits<T>::KeyType>,  /*Hasher Type*/
Equal<typename container::KeyTraits<T>::KeyType>, /*Comparer Type*/
false,                                            /*Allow MultiKey*/
uint64_t,                                         /*Size Type*/
SkrAllocator_New>>;                               /*Allocator Type*/
}