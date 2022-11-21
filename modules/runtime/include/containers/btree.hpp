#pragma once
#include "platform/memory.h"
#include "btree.h"

namespace skr
{
template <class K, class V, class Eq = phmap::Less<K>>
using btree_map = phmap::btree_map<K, V, Eq, skr_stl_allocator<phmap::priv::Pair<const K, V>>>;

template <class K, class Eq = phmap::Less<K>>
using btree_set = phmap::btree_set<K, Eq, skr_stl_allocator<K>>;
} // namespace skr

#include "binary/reader.h"
#include "binary/writer.h"

namespace skr
{
namespace binary
{
template <class K, class V, class Eq>
struct ReadHelper<skr::btree_map<K, V, Eq>> {
    static int Read(skr_binary_reader_t* reader, skr::btree_map<K, V, Eq>& map)
    {
        skr::btree_map<K, V, Eq> temp;
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

template <class K, class V, class Eq>
struct WriteHelper<const skr::btree_map<K, V, Eq>&> {
    static int Write(skr_binary_writer_t* json, const skr::btree_map<K, V, Eq>& map)
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