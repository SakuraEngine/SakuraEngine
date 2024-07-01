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

namespace skr
{
namespace binary
{
template <class K, class V, class Hash, class Eq>
struct ReadTrait<skr::FlatHashMap<K, V, Hash, Eq>> {
    static bool Read(SBinaryReader* archive, skr::FlatHashMap<K, V, Hash, Eq>& map)
    {
        skr::FlatHashMap<K, V, Hash, Eq> temp;
        uint32_t                           size;
        SKR_ARCHIVE(size);

        for (uint32_t i = 0; i < size; ++i)
        {
            K key;
            SKR_ARCHIVE(key);
            V value;
            SKR_ARCHIVE(value);
            temp.insert({ std::move(key), std::move(value) });
        }
        map = std::move(temp);
        return true;
    }
};

template <class K, class V, class Hash, class Eq>
struct WriteTrait<skr::FlatHashMap<K, V, Hash, Eq>> {
    static bool Write(SBinaryWriter* archive, const skr::FlatHashMap<K, V, Hash, Eq>& map)
    {
        SKR_ARCHIVE((uint32_t)map.size());
        for (auto& pair : map)
        {
            SKR_ARCHIVE(pair.first);
            SKR_ARCHIVE(pair.second);
        }
        return true;
    }
};
} // namespace binary

template <class K, class V, class Eq>
struct SerdeCompleteChecker<binary::ReadTrait<skr::FlatHashMap<K, V, Eq>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadTrait<K>> && is_complete_serde_v<binary::ReadTrait<V>>> {
};

template <class K, class V, class Eq>
struct SerdeCompleteChecker<binary::WriteTrait<skr::FlatHashMap<K, V, Eq>>>
    : std::bool_constant<is_complete_serde_v<binary::WriteTrait<K>> && is_complete_serde_v<binary::WriteTrait<V>>> {
};

} // namespace skr

#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace skr::json
{
template <class K, class V, class Hash, class Eq>
struct ReadTrait<skr::FlatHashMap<K, V, Hash, Eq>> {
    static bool Read(skr::archive::JsonReader* json, skr::FlatHashMap<K, V, Hash, Eq>& map)
    {
        size_t count = 0;
        json->StartArray(count);
        map.reserve(count);
        for (size_t i = 0; i < count; i += 2)
        {
            K k;
            V v;
            if (!skr::json::Read<K>(json, k))
                return false;
            
            if (!skr::json::Read<V>(json, v))
                return false;

            map.emplace(std::move(k), std::move(v));
        }
        json->EndArray();
        return true;
    }
};

template <class K, class V, class Hash, class Eq>
struct WriteTrait<skr::FlatHashMap<K, V, Hash, Eq>> {
    static bool Write(skr::archive::JsonWriter* json, const skr::FlatHashMap<K, V, Hash, Eq>& map)
    {
        json->StartArray();
        for (auto& pair : map)
        {
            skr::json::Write(json, pair.first);
            skr::json::Write<V>(json, pair.second);
        }
        json->EndArray();
        return true;
    }
};
} // namespace skr::json

namespace skr
{
template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::ReadTrait<skr::FlatHashMap<K, V, Hash, Eq>>>
    : std::bool_constant<is_complete_serde_v<json::ReadTrait<K>> && is_complete_serde_v<json::ReadTrait<V>>> {
};

template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::WriteTrait<skr::FlatHashMap<K, V, Hash, Eq>>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<K>> && is_complete_serde_v<json::WriteTrait<V>>> {
};
} // namespace skr