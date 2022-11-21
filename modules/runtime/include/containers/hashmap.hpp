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
    static int Read(skr_binary_reader_t* reader, skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        skr::flat_hash_map<K, V, Hash, Eq> temp;
        uint32_t size;
        int ret = ReadValue(reader, size);
        if (ret != 0)
            return ret;

        temp.reserve(size);
        for (int i = 0; i < size; ++i)
        {
            K key;
            ret = skr::binary::Read(reader, key);
            if (ret != 0)
                return ret;
            V value;
            ret = skr::binary::Read(reader, value);
            if (ret != 0)
                return ret;
            temp.insert({ std::move(key), std::move(value) });
        }
        map = std::move(temp);
        return ret;
    }
};

template <class K, class V, class Hash, class Eq>
struct WriteHelper<const skr::flat_hash_map<K, V, Hash, Eq>&> {
    static int Write(skr_binary_writer_t* json, const skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        int ret = WriteValue(json, (uint32_t)map.size());
        if (ret != 0)
            return ret;
        for (auto& pair : map)
        {
            ret = skr::binary::Write<TParamType<K>>(json, pair.first);
            if (ret != 0)
                return ret;
            ret = skr::binary::Write<TParamType<V>>(json, pair.second);
            if (ret != 0)
                return ret;
        }
        return ret;
    }
};
}
}