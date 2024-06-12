#pragma once
#include "SkrJson/writer.h"

#if defined(__cplusplus)
    #include "SkrGuid/guid.hpp"
    #include "SkrContainers/span.hpp"
    #include "SkrContainers/variant.hpp"
    #include "SkrContainers/hashmap.hpp"
    #include "SkrRTTR/rttr_traits.hpp"

// helper functions
namespace skr::json
{
template <class T>
bool Write(skr::json::Writer* writer, const T& value);
}

// primitive types
// bool
// int/uint 8/16/32/64
// float/double
namespace skr::json
{
template <>
struct SKR_STATIC_API WriteTrait<bool> {
    static bool Write(skr::json::Writer* writer, bool b);
};

// int
template <>
struct SKR_STATIC_API WriteTrait<int8_t> {
    static bool Write(skr::json::Writer* writer, int8_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int16_t> {
    static bool Write(skr::json::Writer* writer, int16_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int32_t> {
    static bool Write(skr::json::Writer* writer, int32_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int64_t> {
    static bool Write(skr::json::Writer* writer, int64_t i);
};

// uint
template <>
struct SKR_STATIC_API WriteTrait<uint8_t> {
    static bool Write(skr::json::Writer* writer, uint8_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint16_t> {
    static bool Write(skr::json::Writer* writer, uint16_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint32_t> {
    static bool Write(skr::json::Writer* writer, uint32_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint64_t> {
    static bool Write(skr::json::Writer* writer, uint64_t i);
};

// float
template <>
struct SKR_STATIC_API WriteTrait<float> {
    static bool Write(skr::json::Writer* writer, float f);
};
template <>
struct SKR_STATIC_API WriteTrait<double> {
    static bool Write(skr::json::Writer* writer, double d);
};

} // namespace skr::json

// skr types
namespace skr::json
{
template <>
struct SKR_STATIC_API WriteTrait<skr_float2_t> {
    static bool Write(skr::json::Writer* writer, const skr_float2_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float3_t> {
    static bool Write(skr::json::Writer* writer, const skr_float3_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4_t> {
    static bool Write(skr::json::Writer* writer, const skr_float4_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4x4_t> {
    static bool Write(skr::json::Writer* writer, const skr_float4x4_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_rotator_t> {
    static bool Write(skr::json::Writer* writer, const skr_rotator_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_quaternion_t> {
    static bool Write(skr::json::Writer* writer, const skr_quaternion_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_guid_t> {
    static bool Write(skr::json::Writer* writer, const skr_guid_t& guid);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_md5_t> {
    static bool Write(skr::json::Writer* writer, const skr_md5_t& md5);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    static bool Write(skr::json::Writer* writer, const skr::StringView& str);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::String> {
    static bool Write(skr::json::Writer* writer, const skr::String& str);
};
} // namespace skr::json

// container & template
namespace skr::json
{
template <class K, class V, class Hash, class Eq>
struct WriteTrait<skr::FlatHashMap<K, V, Hash, Eq>> {
    static bool Write(skr::json::Writer* json, const skr::FlatHashMap<K, V, Hash, Eq>& map)
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
template <class V>
struct WriteTrait<skr::Vector<V>> {
    static bool Write(skr::json::Writer* json, const skr::Vector<V>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<V>(json, v);
        }
        json->EndArray();
        return true;
    }
};
template <class V>
struct WriteTrait<skr::span<V>> {
    static bool Write(skr::json::Writer* json, const skr::span<V>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<V>(json, v);
        }
        json->EndArray();
        return true;
    }
};
template <class... Ts>
struct WriteTrait<skr::variant<Ts...>> {
    static bool Write(skr::json::Writer* json, const skr::variant<Ts...>& v)
    {
        skr::visit([&](auto&& value) {
            using raw = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            json->StartObject();
            json->Key(u8"type");
            skr::json::Write<skr_guid_t>(json, ::skr::rttr::type_id_of<raw>());
            json->Key(u8"value");
            skr::json::Write<decltype(value)>(json, value);
            json->EndObject();
        }, v);
        return true;
    }
};
} // namespace skr::json

// help function impl
namespace skr::json
{
template <class T>
bool Write(skr::json::Writer* writer, const T& value)
{
    return WriteTrait<T>::Write(writer, value);
}
} // namespace skr::json

// serde complete check
namespace skr
{
template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::WriteTrait<skr::FlatHashMap<K, V, Hash, Eq>>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<K>> && is_complete_serde_v<json::WriteTrait<V>>> {
};

template <class V>
struct SerdeCompleteChecker<json::WriteTrait<skr::Vector<V>>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<V>>> {
};

template <class... Ts>
struct SerdeCompleteChecker<json::WriteTrait<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<json::WriteTrait<Ts>> && ...)> {
};

} // namespace skr
#endif