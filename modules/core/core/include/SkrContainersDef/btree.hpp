#pragma once
#include "SkrBase/types.h"
#include "SkrCore/memory/memory.h"
#include "parallel_hashmap/btree.h"

namespace skr
{
template <class K, class V, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using BTreeMap = phmap::btree_map<K, V, Eq, Allocator>;

template <class K, class V, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using BTreeMultiMap = phmap::btree_multimap<K, V, Eq, Allocator>;

template <class K, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<K>>
using BTreeSet = phmap::btree_set<K, Eq, Allocator>;

template <class K, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<K>>
using BTreeMultiSet = phmap::btree_multiset<K, Eq, Allocator>;
} // namespace skr