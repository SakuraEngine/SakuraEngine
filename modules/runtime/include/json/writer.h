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
    #include <EASTL/vector.h>
    #include "containers/variant.hpp"
    #include "containers/string.hpp"
    #include "containers/hashmap.hpp"
    #include "type/type_helper.hpp"

// forward declaration for resources
struct skr_resource_handle_t;
namespace skr::resource
{
template <class T>
struct TResourceHandle;
}
// end forward declaration for resources
struct skr_json_format_t
{
    bool enable = true;
    uint32_t indentSize = 4;
};

struct RUNTIME_STATIC_API skr_json_writer_t {
public:
    using TChar = skr_json_writer_char_t;
    using TSize = skr_json_writer_size_t;

    skr_json_writer_t(size_t levelDepth, skr_json_format_t format = skr_json_format_t());
    inline bool IsComplete() { return _hasRoot && _levelStack.empty(); }
    skr::string Str() const;
    bool Bool(bool b);
    bool Int(int32_t i);
    bool UInt(uint32_t i);
    bool Int64(int64_t i);
    bool UInt64(uint64_t i);
    bool Float(float f);
    bool Double(double d);
    bool RawNumber(const TChar* str, TSize length);
    bool RawNumber(skr::string_view view);
    bool String(const TChar* str, TSize length);
    bool String(skr::string_view view);
    bool StartObject();
    bool Key(const TChar* str, TSize length) { return String(str, length); }
    bool Key(skr::string_view view) { return String(view.u8_str(), view.size()); }
    bool EndObject();
    bool StartArray();
    bool EndArray();
    bool RawValue(const TChar* str, TSize length, ESkrJsonType type);
    bool RawValue(skr::string_view view, ESkrJsonType type);

    skr::text::text buffer;

protected:
    struct Level {
        bool isArray = false;
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

    bool _hasRoot = false;
    eastl::vector<Level> _levelStack;
    skr_json_format_t _format;
};
#else
typedef struct skr_json_writer_t skr_json_writer_t;
#endif

#if defined(__cplusplus)
    #include "type/type_id.hpp"
    #include "platform/guid.hpp"
// utils for codegen
namespace skr
{
namespace json
{
template <class T>
void Write(skr_json_writer_t* writer, const T& value);

template <>
struct WriteTrait<const bool&> {
    static void Write(skr_json_writer_t* writer, bool b)
    {
        writer->Bool(b);
    }
};

template <>
struct WriteTrait<const int32_t&> {
    static void Write(skr_json_writer_t* writer, int32_t i)
    {
        writer->Int(i);
    }
};

template <>
struct WriteTrait<const int64_t&> {
    static void Write(skr_json_writer_t* writer, int64_t i)
    {
        writer->Int64(i);
    }
};

template <>
struct WriteTrait<const uint8_t&> {
    static void Write(skr_json_writer_t* writer, uint8_t i)
    {
        writer->UInt(i);
    }
};

template <>
struct WriteTrait<const uint16_t&> {
    static void Write(skr_json_writer_t* writer, uint16_t i)
    {
        writer->UInt(i);
    }
};

template <>
struct WriteTrait<const uint32_t&> {
    static void Write(skr_json_writer_t* writer, uint32_t i)
    {
        writer->UInt(i);
    }
};

template <>
struct WriteTrait<const uint64_t&> {
    static void Write(skr_json_writer_t* writer, uint64_t i)
    {
        writer->UInt64(i);
    }
};

template <>
struct WriteTrait<const float&> {
    static void Write(skr_json_writer_t* writer, float f)
    {
        writer->Float(f);
    }
};

template <>
struct WriteTrait<const double&> {
    static void Write(skr_json_writer_t* writer, double d)
    {
        writer->Double(d);
    }
};

template <>
struct WriteTrait<const skr::string_view&> {
    static void Write(skr_json_writer_t* writer, const skr::string_view& str)
    {
        writer->String(str.u8_str(), str.size());
    }
};

template <>
struct WriteTrait<const skr::string&> {
    static void Write(skr_json_writer_t* writer, const skr::string& str)
    {
        writer->String(str.u8_str(), str.size());
    }
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_guid_t&> {
    static void Write(skr_json_writer_t* writer, const skr_guid_t& guid);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_md5_t&> {
    static void Write(skr_json_writer_t* writer, const skr_md5_t& md5);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float2_t&> {
    static void Write(skr_json_writer_t* writer, const skr_float2_t& v);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float3_t&> {
    static void Write(skr_json_writer_t* writer, const skr_float3_t& v);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float4_t&> {
    static void Write(skr_json_writer_t* writer, const skr_float4_t& v);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_rotator_t&> {
    static void Write(skr_json_writer_t* writer, const skr_rotator_t& v);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float4x4_t&> {
    static void Write(skr_json_writer_t* writer, const skr_float4x4_t& v);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_quaternion_t&> {
    static void Write(skr_json_writer_t* writer, const skr_quaternion_t& v);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_resource_handle_t&> {
    static void Write(skr_json_writer_t* writer, const skr_resource_handle_t& handle);
};

template <class K, class V, class Hash, class Eq>
struct WriteTrait<const skr::flat_hash_map<K, V, Hash, Eq>&> {
    static void Write(skr_json_writer_t* json, const skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        json->StartObject();
        for (auto& pair : map)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(pair.first)>, skr::string>)
            {
                skr::json::Write<const skr::string&>(json, pair.first);
            }
            else
            {
                skr::json::Write<const skr::string&>(json, skr::string(pair.first));
            }
            skr::json::Write<const V&>(json, pair.second);
        }
        json->EndObject();
    }
};

template <class T>
struct WriteTrait<const TEnumAsByte<T>&> {
    static void Write(skr_json_writer_t* writer, const TEnumAsByte<T>& value)
    {
        skr::json::Write(writer, value.as_byte());
    }
};

template <class T>
struct WriteTrait<const skr::resource::TResourceHandle<T>&> {
    static void Write(skr_json_writer_t* json, const skr::resource::TResourceHandle<T>& handle)
    {
        skr::json::Write<const skr_resource_handle_t&>(json, (const skr_resource_handle_t&)handle);
    }
};

template <class V, class Allocator>
struct WriteTrait<const eastl::vector<V, Allocator>&> {
    static void Write(skr_json_writer_t* json, const eastl::vector<V, Allocator>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<const V&>(json, v);
        }
        json->EndArray();
    }
};

template <class V>
struct WriteTrait<const eastl::span<V>&> {
    static void Write(skr_json_writer_t* json, const eastl::span<V>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<const V&>(json, v);
        }
        json->EndArray();
    }
};

template <class... Ts>
struct WriteTrait<const skr::variant<Ts...>&> {
    static void Write(skr_json_writer_t* json, const skr::variant<Ts...>& v)
    {
        std::visit([&](auto&& value) {
            using raw = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            json->StartObject();
            json->Key(u8"type");
            skr::json::Write<const skr_guid_t&>(json, skr::type::type_id<raw>::get());
            json->Key(u8"value");
            skr::json::Write<decltype(value)>(json, value);
            json->EndObject();
        },
        v);
    }
};

template <class T>
void Write(skr_json_writer_t* writer, const T& value)
{
    WriteTrait<const T&>::Write(writer, value);
}
} // namespace json

template <class K, class V, class Hash, class Eq>
struct SerdeCompleteChecker<json::WriteTrait<const skr::flat_hash_map<K, V, Hash, Eq>&>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<K>> && is_complete_serde_v<json::WriteTrait<V>>> {
};

template <class V, class Allocator>
struct SerdeCompleteChecker<json::WriteTrait<const eastl::vector<V, Allocator>&>>
    : std::bool_constant<is_complete_serde_v<json::WriteTrait<V>>> {
};

template <class... Ts>
struct SerdeCompleteChecker<json::WriteTrait<const skr::variant<Ts...>&>>
    : std::bool_constant<(is_complete_serde_v<json::WriteTrait<Ts>> && ...)> {
};

} // namespace skr
#endif