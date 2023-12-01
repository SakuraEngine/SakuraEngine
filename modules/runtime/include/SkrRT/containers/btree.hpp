#pragma once
#include "SkrMemory/memory.h"
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

#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/serde/binary/writer.h"

namespace skr
{
namespace binary
{
template <class K, class V, class Eq>
struct ReadTrait<skr::BTreeMap<K, V, Eq>> {
    static int Read(skr_binary_reader_t* archive, skr::BTreeMap<K, V, Eq>& map)
    {
        skr::BTreeMap<K, V, Eq> temp;
        uint32_t                 size;
        SKR_ARCHIVE(size);

        for (int i = 0; i < size; ++i)
        {
            K key;
            SKR_ARCHIVE(key);
            V value;
            SKR_ARCHIVE(value);
            temp.insert({ std::move(key), std::move(value) });
        }
        map = std::move(temp);
        return 0;
    }
};

template <class K, class V, class Eq>
struct WriteTrait<skr::BTreeMap<K, V, Eq>> {
    static int Write(skr_binary_writer_t* archive, const skr::BTreeMap<K, V, Eq>& map)
    {
        SKR_ARCHIVE((uint32_t)map.size());
        for (auto& pair : map)
        {
            SKR_ARCHIVE(pair.first);
            SKR_ARCHIVE(pair.second);
        }
        return 0;
    }
};
} // namespace binary

template <class K, class V, class Eq>
struct SerdeCompleteChecker<binary::ReadTrait<skr::BTreeMap<K, V, Eq>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadTrait<K>> && is_complete_serde_v<binary::ReadTrait<V>>> {
};

template <class K, class V, class Eq>
struct SerdeCompleteChecker<binary::WriteTrait<skr::BTreeMap<K, V, Eq>>>
    : std::bool_constant<is_complete_serde_v<binary::WriteTrait<K>> && is_complete_serde_v<binary::WriteTrait<V>>> {
};

} // namespace skr