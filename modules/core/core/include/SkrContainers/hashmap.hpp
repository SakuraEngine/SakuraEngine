#pragma once
#include "SkrContainersDef/hashmap.hpp"

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <class K, class V, class Hash, class Eq>
struct BinSerde<skr::FlatHashMap<K, V, Hash, Eq>> {
    inline static bool read(SBinaryReader* r, skr::FlatHashMap<K, V, Hash, Eq>& v)
    {
        // read size
        uint32_t size;
        if (!bin_read(r, size)) return false;

        // read content
        skr::FlatHashMap<K, V, Hash, Eq> temp;
        for (uint32_t i = 0; i < size; ++i)
        {
            K key;
            V value;
            if (!bin_read(r, key)) return false;
            if (!bin_read(r, value)) return false;
            temp.insert({ std::move(key), std::move(value) });
        }

        // move to target
        v = std::move(temp);
        return true;
    }
    inline static bool write(SBinaryWriter* w, const skr::FlatHashMap<K, V, Hash, Eq>& v)
    {
        // write size
        if (!bin_write(w, v.size())) return false;

        // write content
        for (auto& pair : v)
        {
            if (!bin_write(w, pair.first)) return false;
            if (!bin_write(w, pair.second)) return false;
        }
        return true;
    }
};
} // namespace skr

// json serde
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