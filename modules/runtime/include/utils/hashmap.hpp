#pragma once
#include "platform/configure.h"
#include "phmap.h"
#ifdef SKR_RUNTIME_USE_MIMALLOC
#include "mimalloc.h"
#endif

namespace skr
{
template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using flat_hash_map = phmap::flat_hash_map<K, V, Hash, Eq
#ifdef SKR_RUNTIME_USE_MIMALLOC
, mi_stl_allocator<phmap::priv::Pair<const K, V>>
#endif
>;

template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using parallel_flat_hash_map = phmap::parallel_flat_hash_map<K, V, Hash, Eq
#ifdef SKR_RUNTIME_USE_MIMALLOC
, mi_stl_allocator<phmap::priv::Pair<const K, V>>, 4, std::shared_mutex
#endif
>;

template <class K,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>>
using flat_hash_set = phmap::flat_hash_set<K, Hash, Eq
#ifdef SKR_RUNTIME_USE_MIMALLOC
, mi_stl_allocator<K>
#endif
>;
} // namespace skr