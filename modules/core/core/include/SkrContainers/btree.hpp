#pragma once
#include "SkrContainersDef/btree.hpp"

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <class K, class V, class Eq>
struct BinSerde<skr::BTreeMap<K, V, Eq>> {
    inline static bool read(SBinaryReader* r, skr::BTreeMap<K, V, Eq>& v)
    {
        // read size
        uint32_t size;
        if (!bin_read(r, size)) return false;

        // read content
        skr::BTreeMap<K, V, Eq> temp;
        for (int i = 0; i < size; ++i)
        {
            K key;
            if (!bin_read(r, key)) return false;
            V value;
            if (!bin_read(r, value)) return false;
            temp.insert({ std::move(key), std::move(value) });
        }

        // move to target
        v = std::move(temp);
        return true;
    }
    inline static bool write(SBinaryWriter* w, const skr::BTreeMap<K, V, Eq>& v)
    {
        // write size
        if (!bin_write(w, v.size())) return false;

        // write content
        for (const auto& pair : v)
        {
            if (!bin_write(w, pair.first)) return false;
            if (!bin_write(w, pair.second)) return false;
        }
        return true;
    }
};
} // namespace skr