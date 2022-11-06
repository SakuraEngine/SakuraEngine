#pragma once
#include "platform/memory.h"
#include "phmap.h"

namespace skr
{
template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using flat_hash_map = phmap::flat_hash_map<K, V, Hash, Eq, skr_stl_allocator<phmap::priv::Pair<const K, V>>>;

template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using parallel_flat_hash_map = phmap::parallel_flat_hash_map<K, V, Hash, Eq, skr_stl_allocator<phmap::priv::Pair<const K, V>>, 4, std::shared_mutex>;

template <class K,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using flat_hash_set = phmap::flat_hash_set<K, Hash, Eq, skr_stl_allocator<K>>;
} // namespace skr