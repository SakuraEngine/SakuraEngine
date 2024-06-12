#pragma once
#include "SkrBase/types.h"
#if defined(__cplusplus)
    #include "SkrArchive/json/reader.h"
    #include "SkrContainers/hashmap.hpp"
    #include "SkrContainers/string.hpp"
    #include "SkrContainers/vector.hpp"
    #include "SkrContainers/variant.hpp"
    #include "SkrRTTR/rttr_traits.hpp"

// helper functions
namespace skr::json
{
template <class T>
bool Read(skr::json::Reader* json, T& value);

// primitive types
// bool
// int/uint 8/16/32/64
// float/double
template <>
struct SKR_STATIC_API ReadTrait<bool> {
    static bool Read(skr::json::Reader* json, bool& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int8_t> {
    static bool Read(skr::json::Reader* json, int8_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int16_t> {
    static bool Read(skr::json::Reader* json, int16_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int32_t> {
    static bool Read(skr::json::Reader* json, int32_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int64_t> {
    static bool Read(skr::json::Reader* json, int64_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint8_t> {
    static bool Read(skr::json::Reader* json, uint8_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint16_t> {
    static bool Read(skr::json::Reader* json, uint16_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint32_t> {
    static bool Read(skr::json::Reader* json, uint32_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint64_t> {
    static bool Read(skr::json::Reader* json, uint64_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<float> {
    static bool Read(skr::json::Reader* json, float& value);
};
template <>
struct SKR_STATIC_API ReadTrait<double> {
    static bool Read(skr::json::Reader* json, double& value);
};
} // namespace skr::json

// skr type
namespace skr::json
{
template <>
struct SKR_STATIC_API ReadTrait<skr_float2_t> {
    static bool Read(skr::json::Reader* json, skr_float2_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float3_t> {
    static bool Read(skr::json::Reader* json, skr_float3_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float4_t> {
    static bool Read(skr::json::Reader* json, skr_float4_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float4x4_t> {
    static bool Read(skr::json::Reader* json, skr_float4x4_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_rotator_t> {
    static bool Read(skr::json::Reader* json, skr_rotator_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_quaternion_t> {
    static bool Read(skr::json::Reader* json, skr_quaternion_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_guid_t> {
    static bool Read(skr::json::Reader* json, skr_guid_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_md5_t> {
    static bool Read(skr::json::Reader* json, skr_md5_t& md5);
};
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    static bool Read(skr::json::Reader* json, skr::String& value);
};
} // namespace skr::json

// container & template
namespace skr::json
{
template <class K, class V, class Hash, class Eq>
struct ReadTrait<skr::FlatHashMap<K, V, Hash, Eq>> {
    static bool Read(skr::json::Reader* json, skr::FlatHashMap<K, V, Hash, Eq>& map)
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

template <class V>
struct ReadTrait<skr::Vector<V>> {
    static bool Read(skr::json::Reader* json, skr::Vector<V>& vec)
    {
        size_t count;
        json->StartArray(count);
        vec.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            V          v;
            if (!skr::json::Read<V>(json, v))
                return false;
            vec.emplace(std::move(v));
        }
        json->EndArray();
        return true;
    }
};

template <class... Ts>
struct ReadTrait<skr::variant<Ts...>> {
    template <class T>
    static bool ReadByIndex(skr::json::Reader* json, skr::variant<Ts...>& value, skr_guid_t index)
    {
        if (index == ::skr::rttr::type_id_of<T>())
        {
            T          t;
            if (!skr::json::Read(json, t))
                return false;
            value = std::move(t);
            return true;
        }
        return false;
    }

    static bool Read(skr::json::Reader* json, skr::variant<Ts...>& value)
    {
        json->StartObject();
        
        json->Key(u8"type");
        skr_guid_t index;
        if (!skr::json::Read<skr_guid_t>(json, index))
            return false;

        json->Key(u8"value");
        (void)(((ReadByIndex<Ts>(json, value, index)) != true) && ...);
        
        json->EndObject();

        return true;
    }
};

} // namespace skr::json

// helper functions impl
namespace skr::json
{
template <class T>
bool Read(skr::json::Reader* json, T& value)
{
    return ReadTrait<T>::Read(json, value);
}
} // namespace skr::json

// serde complete check
namespace skr
{
template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::ReadTrait<skr::FlatHashMap<K, V, Hash, Eq>>>
    : std::bool_constant<is_complete_serde_v<json::ReadTrait<K>> && is_complete_serde_v<json::ReadTrait<V>>> {
};

template <class V>
struct SerdeCompleteChecker<json::ReadTrait<skr::Vector<V>>>
    : std::bool_constant<is_complete_serde_v<json::ReadTrait<V>>> {
};

template <class... Ts>
struct SerdeCompleteChecker<json::ReadTrait<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<json::ReadTrait<Ts>> && ...)> {
};
} // namespace skr

#endif