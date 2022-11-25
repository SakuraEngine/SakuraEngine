#pragma once
#include "platform/memory.h"
#include "phmap.h"

namespace skr
{
template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>,
class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using flat_hash_map = phmap::flat_hash_map<K, V, Hash, Eq, Allocator>;

template <class K, class V,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>,
class Allocator = skr_stl_allocator<phmap::priv::Pair<const K, V>>>
using parallel_flat_hash_map = phmap::parallel_flat_hash_map<K, V, Hash, Eq, Allocator, 4, std::shared_mutex>;

template <class K,
class Hash = phmap::priv::hash_default_hash<K>,
class Eq = phmap::priv::hash_default_eq<K>,
class Allocator = skr_stl_allocator<K>>
using flat_hash_set = phmap::flat_hash_set<K, Hash, Eq, Allocator>;
} // namespace skr

#include "binary/reader.h"
#include "binary/writer.h"

namespace skr
{
namespace binary
{
template <class K, class V, class Hash, class Eq>
struct ReadHelper<skr::flat_hash_map<K, V, Hash, Eq>> {
    static int Read(skr_binary_reader_t* archive, skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        skr::flat_hash_map<K, V, Hash, Eq> temp;
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

template <class K, class V, class Hash, class Eq>
struct WriteHelper<const skr::flat_hash_map<K, V, Hash, Eq>&> {
    static int Write(skr_binary_writer_t* archive, const skr::flat_hash_map<K, V, Hash, Eq>& map)
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
struct SerdeCompleteChecker<binary::ReadHelper<skr::flat_hash_map<K, V, Eq>>>
    : std::bool_constant<is_complete_serde_v<binary::ReadHelper<K>> && is_complete_serde_v<binary::ReadHelper<V>>> {
};

template<class K, class V, class Eq>
struct SerdeCompleteChecker<binary::WriteHelper<const skr::flat_hash_map<K, V, Eq>&>>
    : std::bool_constant<is_complete_serde_v<binary::WriteHelper<const K&>> && is_complete_serde_v<binary::WriteHelper<const V&>>> {
};

} // namespace skr