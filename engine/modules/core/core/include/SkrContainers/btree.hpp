#pragma once
#include "SkrContainersDef/btree.hpp"

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <class K, class V, class Eq>
struct Serialize<skr::BTreeMap<K, V, Eq>>
{
    inline static void read(ArchiveRead& r, skr::BTreeMap<K, V, Eq>& v)
    {
        Archive::ObjectScope arr_scope(r);
        SKR_ASSERT(arr_scope.is_success());

        // read size
        v.clear();
        uint64_t arr_size = 0;
        SKR_ASSERT(r.array_size<uint64_t>(arr_size));
        v.reserve(arr_size);

        // read content
        for (uint64_t i = 0; i < arr_size; i += 2)
        {
            K key;
            SKR_ASSERT(r.value<K>(key));
            V value;
            SKR_ASSERT(r.value<V>(value));
            v.insert({ std::move(key), std::move(value) });
        }
    }
    inline static void write(ArchiveWrite& w, const skr::BTreeMap<K, V, Eq>& v)
    {
        Archive::ObjectScope arr_scope(w);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write size
        SKR_FAST_CHECK(w.array_size<uint64_t>(uint64_t(v.size() * 2)), );

        // write content
        for (const auto& pair : v)
        {
            SKR_FAST_CHECK(w.value<K>(pair.first), );
            SKR_FAST_CHECK(w.value<V>(pair.second), );
        }
    }
};
} // namespace skr