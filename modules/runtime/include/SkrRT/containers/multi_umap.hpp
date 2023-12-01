#pragma once
#include "SkrRT/containers/skr_allocator.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr
{
template <typename K, typename V>
using MultiUMap = container::SparseHashMap<
K,             /*Key Type*/
V,             /*Value Type*/
size_t,        /*BitBlock Type*/
size_t,        /*Hash Type*/
Hash<K>,       /*Hasher Type*/
Equal<K>,      /*Comparer Type*/
true,          /*Allow MultiKey*/
SkrAllocator>; /*Allocator Type*/
}