#pragma once
#include "config.h"
#ifndef CGPU_BUILD_STANDALONE
// #include <SkrContainers/hashmap.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrContainers/umap.hpp>
#include <SkrContainers/uset.hpp>

namespace cgpu
{
template <typename T, size_t N>
using FixedUSet = skr::FixedUSet<T, N>;

template <typename K, typename V>
using UMap = skr::UMap<K, V>;

template <typename T>
using Vector = skr::Vector<T>;

} // namespace cgpu

#else
#endif