#pragma once
#include "SkrContainersDef/map.hpp" // IWYU pragma: export

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
static constexpr GUID kMapGenericId = u8"9eae06c4-d7ea-4246-af0e-c95d401a7a71"_guid;
template <typename K, typename V>
struct TypeSignatureTraits<::skr::Map<K, V>>
{
    inline static constexpr bool is_supported = concepts::WithRTTRTraits<K> && concepts::WithRTTRTraits<V>;
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<K>::buffer_size + TypeSignatureTraits<V>::buffer_size;
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kMapGenericId, 2);
        pos = TypeSignatureTraits<K>::write(pos, end);
        return TypeSignatureTraits<V>::write(pos, end);
    }
};
} // namespace skr

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename Map>
struct SerializeSkrMapImpl
{
    using KeyType = typename Map::MapKeyType;
    using ValueType = typename Map::MapValueType;

    inline static void read(ArchiveRead& r, Map& v)
    {
        SkrZoneScopedN("Serialize<Map>::read");

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
            KeyType key;
            ValueType value;

            SKR_FAST_CHECK(r.value<KeyType>(key), );
            SKR_FAST_CHECK(r.value<ValueType>(value), );

            v.add(std::move(key), std::move(value));
        }
    }
    inline static void write(ArchiveWrite& w, const Map& v)
    {
        SkrZoneScopedN("Serialize<Map>::write");

        Archive::ArrayScope arr_scope{ w };
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write count
        SKR_FAST_CHECK(w.array_size<uint64_t>(uint64_t(v.size() * 2)), );

        // write content
        for (const auto& pair : v)
        {
            SKR_FAST_CHECK(w.value<KeyType>(pair.key), );
            SKR_FAST_CHECK(w.value<ValueType>(pair.value), );
        }
    }
};

template <typename K, typename V, typename HashTraits, typename Allocator>
struct Serialize<skr::Map<K, V, HashTraits, Allocator>>
    : SerializeSkrMapImpl<skr::Map<K, V, HashTraits, Allocator>>
{
};
template <typename K, typename V, uint64_t kCount, typename HashTraits>
struct Serialize<skr::FixedMap<K, V, kCount, HashTraits>>
    : SerializeSkrMapImpl<skr::FixedMap<K, V, kCount, HashTraits>>
{
};
template <typename K, typename V, uint64_t kInlineCount, typename HashTraits, typename Allocator>
struct Serialize<skr::InlineMap<K, V, kInlineCount, HashTraits, Allocator>>
    : SerializeSkrMapImpl<skr::InlineMap<K, V, kInlineCount, HashTraits, Allocator>>
{
};
} // namespace skr