#pragma once
#include "phmap.h"
#include "mimalloc.h"

namespace skr
{
template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using flat_hash_map = phmap::flat_hash_map<K, V, Hash, Eq, mi_stl_allocator<phmap::priv::Pair<const K, V>>>;

template <class K,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using flat_hash_set = phmap::flat_hash_set<K, Hash, Eq, mi_stl_allocator<K>>;
} // namespace skr