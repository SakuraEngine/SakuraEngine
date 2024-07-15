#pragma once
#include "SkrContainersDef/btree.hpp"

// bin serde
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
namespace skr::binary
{
template <class K, class V, class Eq>
struct ReadTrait<skr::BTreeMap<K, V, Eq>> {
    static bool Read(SBinaryReader* archive, skr::BTreeMap<K, V, Eq>& map)
    {
        skr::BTreeMap<K, V, Eq> temp;
        uint32_t                size;
        if (!skr::binary::Read(archive, (size))) return false;

        for (int i = 0; i < size; ++i)
        {
            K key;
            if (!skr::binary::Read(archive, (key))) return false;
            V value;
            if (!skr::binary::Read(archive, (value))) return false;
            temp.insert({ std::move(key), std::move(value) });
        }
        map = std::move(temp);
        return true;
    }
};

template <class K, class V, class Eq>
struct WriteTrait<skr::BTreeMap<K, V, Eq>> {
    static bool Write(SBinaryWriter* archive, const skr::BTreeMap<K, V, Eq>& map)
    {
        if (!skr ::binary ::Write(archive, ((uint32_t)map.size()))) return false;
        for (auto& pair : map)
        {
            if (!skr::binary::Write(archive, (pair.first))) return false;
            if (!skr::binary::Write(archive, (pair.second))) return false;
        }
        return true;
    }
};
} // namespace skr::binary