#pragma once
#include "SkrContainersDef/hashmap.hpp" // IWYU pragma: export

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
#include "SkrSerde/json_serde.hpp"
namespace skr
{
template <class K, class V, class Hash, class Eq>
struct JsonSerde<skr::FlatHashMap<K, V, Hash, Eq>> {
    inline static bool read(skr::archive::JsonReader* r, skr::FlatHashMap<K, V, Hash, Eq>& v)
    {
        size_t count = 0;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        v.reserve(count / 2);
        for (size_t i = 0; i < count; i += 2)
        {
            K key;
            V value;

            if (!json_read<K>(r, key))
                return false;
            if (!json_read<V>(r, value))
                return false;
            v.emplace(std::move(key), std::move(value));
        }
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::FlatHashMap<K, V, Hash, Eq>& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        for (const auto& pair : v)
        {
            if (!json_write<K>(w, pair.first)) return false;
            if (!json_write<V>(w, pair.second)) return false;
        }
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
} // namespace skr