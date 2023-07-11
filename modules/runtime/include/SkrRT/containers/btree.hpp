#pragma once
#include "platform/memory.h"
#include "parallel_hashmap/btree.h"

namespace skr
{
template <class K, class V, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using btree_map = phmap::btree_map<K, V, Eq, Allocator>;

template <class K, class V, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using btree_multimap = phmap::btree_multimap<K, V, Eq, Allocator>;

template <class K, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<K>>
using btree_set = phmap::btree_set<K, Eq, Allocator>;

template <class K, class Eq = phmap::Less<K>, class Allocator = skr_stl_allocator<K>>
using btree_multiset = phmap::btree_multiset<K, Eq, Allocator>;
} // namespace skr

#include "serde/binary/reader.h"
#include "serde/binary/writer.h"

namespace skr
{
namespace binary
{
template <class K, class V, class Eq>
struct ReadTrait<skr::btree_map<K, V, Eq>> {
    static int Read(skr_binary_reader_t* archive, skr::btree_map<K, V, Eq>& map)
    {
        skr::btree_map<K, V, Eq> temp;
        uint32_t size;
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
struct WriteTrait<const skr::btree_map<K, V, Eq>&> {
    static int Write(skr_binary_writer_t* archive, const skr::btree_map<K, V, Eq>& map)
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

template<class K, class V, class Eq>
struct SerdeCompleteChecker<binary::ReadTrait<skr::btree_map<K, V, Eq>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadTrait<K>> && is_complete_serde_v<binary::ReadTrait<V>>> {
};

template<class K, class V, class Eq>
struct SerdeCompleteChecker<binary::WriteTrait<const skr::btree_map<K, V, Eq>&>>
    : std::bool_constant<is_complete_serde_v<binary::WriteTrait<const K&>> && is_complete_serde_v<binary::WriteTrait<const V&>>> {
};

} // namespace skr