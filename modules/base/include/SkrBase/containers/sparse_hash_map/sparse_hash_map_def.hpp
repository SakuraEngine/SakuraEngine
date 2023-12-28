#pragma once
#include "SkrBase/algo/utils.hpp"
#include "SkrBase/config.h"
#include "SkrBase/misc/hash.hpp"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"

namespace skr::container
{
template <typename K, typename V, typename TS>
using SparseHashMapDataRef = SparseHashSetDataRef<KVPair<K, V>, TS>;
} // namespace skr::container
