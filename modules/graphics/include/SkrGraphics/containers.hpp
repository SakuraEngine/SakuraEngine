#pragma once
#include "config.h"
#ifndef CGPU_BUILD_STANDALONE
#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/map.hpp>
#include <SkrContainers/set.hpp>
#include <SkrContainers/btree.hpp>
#include <SkrContainers/concurrent_queue.hpp>
#include <SkrContainers/stl_string.hpp>
#include <SkrContainers/stl_vector.hpp>
#include <SkrContainers/string.hpp>
#include <SkrContainers/span.hpp>
#include <SkrContainers/vector.hpp>

namespace cgpu
{

using stl_string = skr::stl_string;
using stl_u8string = skr::stl_u8string;
using stl_wstring = skr::stl_wstring;

template <typename T>
using BTreeSet = skr::BTreeSet<T>;

template <typename K, typename V, typename Hash = phmap::priv::hash_default_hash<K>, typename Eq = phmap::priv::hash_default_eq<K>>
using FlatHashMap = skr::FlatHashMap<K, V, Hash, Eq>;

template <typename K, typename V, typename Hash = phmap::priv::hash_default_hash<K>, typename Eq = phmap::priv::hash_default_eq<K>>
using ParallelFlatHashMap = skr::ParallelFlatHashMap<K, V, Hash, Eq>;

template <typename T>
using stl_vector = skr::stl_vector<T>;

template<typename T>
using ConcurrentQueue = skr::ConcurrentQueue<T>;

//-----------------------------------------------------//

using String = skr::String;

template <typename T>
using Set = skr::Set<T>;

template <typename T, size_t N>
using FixedSet = skr::FixedSet<T, N>;

template <typename K, typename V>
using Map = skr::Map<K, V>;

template <typename T>
using span = skr::span<T>;

template <typename T>
using Vector = skr::Vector<T>;

template <typename T, size_t N>
using InlineVector = skr::Vector<T>;


} // namespace cgpu

#else
#endif