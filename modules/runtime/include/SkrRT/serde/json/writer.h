#pragma once
#include "writer_fwd.h"

typedef enum ESkrJsonType
{
    SKR_JSONTYPE_BOOL,
    SKR_JSONTYPE_NUMBER,
    SKR_JSONTYPE_STRING,
    SKR_JSONTYPE_OBJECT,
    SKR_JSONTYPE_ARRAY,
} ESkrJsonType;

#if defined(__cplusplus)
    #include "SkrRT/containers_new/variant.hpp"
    #include "SkrRT/containers_new/string.hpp"
    #include "SkrRT/containers_new/hashmap.hpp"
    #include "SkrRT/containers_new/span.hpp"
    #include "SkrRT/containers_new/stl_vector.hpp"
    #include "SkrRT/rttr/rttr_traits.hpp"

// forward declaration for resources
struct skr_resource_handle_t;
namespace skr::resource
{
template <class T>
struct TResourceHandle;
}
// end forward declaration for resources

// json writer
struct skr_json_format_t {
    bool     enable     = true;
    uint32_t indentSize = 4;
};
struct SKR_STATIC_API skr_json_writer_t {
public:
    using TChar = skr_json_writer_char_t;
    using TSize = skr_json_writer_size_t;

    skr_json_writer_t(size_t levelDepth, skr_json_format_t format = skr_json_format_t());
    inline bool IsComplete() { return _hasRoot && _levelStack.empty(); }
    skr::string Str() const;
    bool        Bool(bool b);
    bool        Int(int32_t i);
    bool        UInt(uint32_t i);
    bool        Int64(int64_t i);
    bool        UInt64(uint64_t i);
    bool        Float(float f);
    bool        Double(double d);
    bool        RawNumber(const TChar* str, TSize length);
    bool        RawNumber(skr::string_view view);
    bool        String(const TChar* str, TSize length);
    bool        String(skr::string_view view);
    bool        StartObject();
    bool        Key(const TChar* str, TSize length);
    bool        Key(skr::string_view view);
    bool        EndObject();
    bool        StartArray();
    bool        EndArray();
    bool        RawValue(const TChar* str, TSize length, ESkrJsonType type);
    bool        RawValue(skr::string_view view, ESkrJsonType type);

    skr::string buffer;

protected:
    struct Level {
        bool     isArray    = false;
        uint32_t valueCount = 0;
    };
    bool _WriteBool(bool b);
    bool _WriteInt(int32_t i);
    bool _WriteUInt(uint32_t i);
    bool _WriteInt64(int64_t i);
    bool _WriteUInt64(uint64_t i);
    bool _WriteFloat(float f);
    bool _WriteDouble(double d);
    bool _WriteString(const TChar* str, TSize length);
    bool _WriteStartObject();
    bool _WriteEndObject();
    bool _WriteStartArray();
    bool _WriteEndArray();
    bool _WriteRawValue(const TChar* str, TSize length);
    bool _Prefix(ESkrJsonType type);
    bool _NewLine();

    bool               _hasRoot = false;
    skr::stl_vector<Level> _levelStack;
    skr_json_format_t  _format;
};
#else
typedef struct skr_json_writer_t skr_json_writer_t;
#endif

#if defined(__cplusplus)
    #include "SkrRT/platform/guid.hpp"

// helper functions
namespace skr::json
{
template <class T>
void Write(skr_json_writer_t* writer, const T& value);
}

// primitive types
// bool
// int/uint 8/16/32/64
// float/double
namespace skr::json
{
template <>
struct SKR_STATIC_API WriteTrait<bool> {
    static void Write(skr_json_writer_t* writer, bool b);
};

// int
template <>
struct SKR_STATIC_API WriteTrait<int8_t> {
    static void Write(skr_json_writer_t* writer, int8_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int16_t> {
    static void Write(skr_json_writer_t* writer, int16_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int32_t> {
    static void Write(skr_json_writer_t* writer, int32_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int64_t> {
    static void Write(skr_json_writer_t* writer, int64_t i);
};

// uint
template <>
struct SKR_STATIC_API WriteTrait<uint8_t> {
    static void Write(skr_json_writer_t* writer, uint8_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint16_t> {
    static void Write(skr_json_writer_t* writer, uint16_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint32_t> {
    static void Write(skr_json_writer_t* writer, uint32_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint64_t> {
    static void Write(skr_json_writer_t* writer, uint64_t i);
};

// float
template <>
struct SKR_STATIC_API WriteTrait<float> {
    static void Write(skr_json_writer_t* writer, float f);
};
template <>
struct SKR_STATIC_API WriteTrait<double> {
    static void Write(skr_json_writer_t* writer, double d);
};

} // namespace skr::json

// skr types
namespace skr::json
{
template <>
struct SKR_STATIC_API WriteTrait<skr_float2_t> {
    static void Write(skr_json_writer_t* writer, const skr_float2_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float3_t> {
    static void Write(skr_json_writer_t* writer, const skr_float3_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4_t> {
    static void Write(skr_json_writer_t* writer, const skr_float4_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4x4_t> {
    static void Write(skr_json_writer_t* writer, const skr_float4x4_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_rotator_t> {
    static void Write(skr_json_writer_t* writer, const skr_rotator_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_quaternion_t> {
    static void Write(skr_json_writer_t* writer, const skr_quaternion_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_guid_t> {
    static void Write(skr_json_writer_t* writer, const skr_guid_t& guid);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_md5_t> {
    static void Write(skr_json_writer_t* writer, const skr_md5_t& md5);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_resource_handle_t> {
    static void Write(skr_json_writer_t* writer, const skr_resource_handle_t& handle);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::string_view> {
    static void Write(skr_json_writer_t* writer, const skr::string_view& str);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::string> {
    static void Write(skr_json_writer_t* writer, const skr::string& str);
};
} // namespace skr::json

// container & template
namespace skr::json
{
template <class K, class V, class Hash, class Eq>
struct WriteTrait<skr::flat_hash_map<K, V, Hash, Eq>> {
    static void Write(skr_json_writer_t* json, const skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        json->StartObject();
        for (auto& pair : map)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(pair.first)>, skr::string>)
            {
                skr::json::Write<skr::string>(json, pair.first);
            }
            else
            {
                skr::json::Write<skr::string>(json, skr::string(pair.first));
            }
            skr::json::Write<V>(json, pair.second);
        }
        json->EndObject();
    }
};
template <class T>
struct WriteTrait<skr::resource::TResourceHandle<T>> {
    static void Write(skr_json_writer_t* json, const skr::resource::TResourceHandle<T>& handle)
    {
        skr::json::Write<skr_resource_handle_t>(json, (const skr_resource_handle_t&)handle);
    }
};
template <class V>
struct WriteTrait<skr::vector<V>> {
    static void Write(skr_json_writer_t* json, const skr::vector<V>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<V>(json, v);
        }
        json->EndArray();
    }
};
template <class V>
struct WriteTrait<skr::span<V>> {
    static void Write(skr_json_writer_t* json, const skr::span<V>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<V>(json, v);
        }
        json->EndArray();
    }
};
template <class... Ts>
struct WriteTrait<skr::variant<Ts...>> {
    static void Write(skr_json_writer_t* json, const skr::variant<Ts...>& v)
    {
        std::visit([&](auto&& value) {
            using raw = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            json->StartObject();
            json->Key(u8"type");
            skr::json::Write<skr_guid_t>(json, ::skr::rttr::type_id<raw>());
            json->Key(u8"value");
            skr::json::Write<decltype(value)>(json, value);
            json->EndObject();
        },
                   v);
    }
};
} // namespace skr::json

// help function impl
namespace skr::json
{
template <class T>
void Write(skr_json_writer_t* writer, const T& value)
{
    WriteTrait<T>::Write(writer, value);
}
} // namespace skr::json

// serde complete check
namespace skr
{
template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::WriteTrait<skr::flat_hash_map<K, V, Hash, Eq>>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<K>> && is_complete_serde_v<json::WriteTrait<V>>> {
};

template <class V>
struct SerdeCompleteChecker<json::WriteTrait<skr::vector<V>>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<V>>> {
};

template <class... Ts>
struct SerdeCompleteChecker<json::WriteTrait<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<json::WriteTrait<Ts>> && ...)> {
};

} // namespace skr
#endif