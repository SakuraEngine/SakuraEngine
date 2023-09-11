#pragma once
#include "SkrRT/containers_new/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename T>
using USet = container::SparseHashSet<
T,                                                /*element Type*/
size_t,                                           /*BitBlock Type*/
size_t,                                           /*Hash Type*/
Hash<typename container::KeyTraits<T>::KeyType>,  /*Hasher Type*/
Equal<typename container::KeyTraits<T>::KeyType>, /*Comparer Type*/
true,                                             /*Allow MultiKey*/
SkrAllocator>;                                    /*Allocator Type*/
}