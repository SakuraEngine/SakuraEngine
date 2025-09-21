#pragma once
#include "SkrContainersDef/hashmap.hpp" // IWYU pragma: export

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <template <class...> class Map, class K, class V, class Hash, class Eq>
struct SerializePhMapImpl
{
    inline static void read(ArchiveRead& r, Map<K, V, Hash, Eq>& v)
    {
        Archive::ArrayScope arr_scope{ r };
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // reserve
        v.clear();
        uint64_t arr_size;
        SKR_FAST_CHECK(r.array_size<uint64_t>(arr_size), );
        v.reserve(arr_size / 2);

        // read content
        for (uint64_t i = 0; i < arr_size; i += 2)
        {
            K key;
            V value;

            SKR_FAST_CHECK(r.value<K>(key), );
            SKR_FAST_CHECK(r.value<V>(value), );

            v.emplace(std::move(key), std::move(value));
        }
    }
    inline static void write(ArchiveWrite& w, const Map<K, V, Hash, Eq>& v)
    {
        Archive::ArrayScope arr_scope{ w };
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write size
        SKR_FAST_CHECK(w.array_size<uint64_t>(static_cast<uint64_t>(v.size() * 2)), );

        // write content
        for (const auto& pair : v)
        {
            SKR_FAST_CHECK(w.value<K>(pair.first), );
            SKR_FAST_CHECK(w.value<V>(pair.second), );
        }
    }
};

// FlatHashMap specialization
template <class K, class V, class Hash, class Eq>
struct Serialize<skr::FlatHashMap<K, V, Hash, Eq>>
    : SerializePhMapImpl<skr::FlatHashMap, K, V, Hash, Eq>
{
};

// ParallelFlatHashMap specialization
template <class K, class V, class Hash, class Eq>
struct Serialize<skr::ParallelFlatHashMap<K, V, Hash, Eq>>
    : SerializePhMapImpl<skr::ParallelFlatHashMap, K, V, Hash, Eq>
{
};
} // namespace skr
