#pragma once
#include "SkrBase/types.h"
#include "SkrCore/memory/memory.h"
#include "parallel_hashmap/phmap.h"

namespace skr
{
template <class K, class V,
          class Hash      = phmap::priv::hash_default_hash<K>,
          class Eq        = phmap::priv::hash_default_eq<K>,
          class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using FlatHashMap = phmap::flat_hash_map<K, V, Hash, Eq, Allocator>;

template <class K, class V,
          class Hash      = phmap::priv::hash_default_hash<K>,
          class Eq        = phmap::priv::hash_default_eq<K>,
          class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using ParallelFlatHashMap = phmap::parallel_flat_hash_map<K, V, Hash, Eq, Allocator, 4, std::shared_mutex>;

template <class K,
          class Hash      = phmap::priv::hash_default_hash<K>,
          class Eq        = phmap::priv::hash_default_eq<K>,
          class Allocator = skr_stl_allocator<K>>
using FlatHashSet = phmap::flat_hash_set<K, Hash, Eq, Allocator>;

template <class K,
          class Hash      = phmap::priv::hash_default_hash<K>,
          class Eq        = phmap::priv::hash_default_eq<K>,
          class Allocator = skr_stl_allocator<K>>
using ParallelFlatHashSet = phmap::parallel_flat_hash_set<K, Hash, Eq, Allocator, 4, std::shared_mutex>;
} // namespace skr