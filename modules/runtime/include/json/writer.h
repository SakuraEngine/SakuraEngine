#pragma once
#include "platform/configure.h"

typedef enum ESkrJsonType
{
    SKR_JSONTYPE_BOOL,
    SKR_JSONTYPE_NUMBER,
    SKR_JSONTYPE_STRING,
    SKR_JSONTYPE_OBJECT,
    SKR_JSONTYPE_ARRAY,
} ESkrJsonType;

#if defined(__cplusplus)
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include "fmt/format.h"
#include "containers/hashmap.hpp"

// forward declaration for resources
struct skr_resource_handle_t;
namespace skr::resource { template <class T> struct TResourceHandle; }
// end forward declaration for resources

struct RUNTIME_API skr_json_writer_t {
public:
    using TChar = char;
    using TSize = size_t;

    skr_json_writer_t(size_t levelDepth);
    inline bool IsComplete() { return _hasRoot && _levelStack.empty(); }
    eastl::string Str() const;
    bool Bool(bool b);
    bool Int(int32_t i);
    bool UInt(uint32_t i);
    bool Int64(int64_t i);
    bool UInt64(uint64_t i);
    bool Float(float f);
    bool Double(double d);
    bool RawNumber(const TChar* str, TSize length);
    bool RawNumber(std::basic_string_view<TChar> view);
    bool String(const TChar* str, TSize length);
    bool String(std::basic_string_view<TChar> view);
    bool StartObject();
    bool Key(const TChar* str, TSize length) { return String(str, length); }
    bool Key(std::basic_string_view<TChar> view) { return String(view.data(), view.size()); }
    bool EndObject();
    bool StartArray();
    bool EndArray();
    bool RawValue(const TChar* str, TSize length, ESkrJsonType type);
    bool RawValue(std::basic_string_view<TChar> view, ESkrJsonType type);

    fmt::memory_buffer buffer;
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

    bool _hasRoot = false;
    eastl::vector<Level> _levelStack;
};
#else
typedef struct skr_json_writer_t skr_json_writer_t;
#endif

#if defined(__cplusplus)
    #include "EASTL/string.h"
    #include "type/type_id.hpp"
    #include "platform/guid.hpp"
// utils for codegen
namespace skr
{
namespace json
{
using TChar = skr_json_writer_t::TChar;
using TSize = skr_json_writer_t::TSize;
template <class T>
using TParamType = std::conditional_t<std::is_fundamental_v<T> || std::is_enum_v<T>, T, const T&>;
template <class T>
std::enable_if_t<!std::is_enum_v<T>, void> WriteValue(skr_json_writer_t* writer, T value)
{
    static_assert(!sizeof(T), "WriteValue not implemented for this type");
}
template <class T>
void WriteFields(skr_json_writer_t* writer, T value)
{
    static_assert(!sizeof(T), "WriteFields not implemented for this type");
}

template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, bool b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, int32_t b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, uint32_t b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, int64_t b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, uint64_t b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, float b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, double b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const eastl::string_view& str);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const eastl::string& str);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const skr_guid_t& guid);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const skr_resource_handle_t& handle);

template <class T>
std::enable_if_t<std::is_enum_v<T>, void> WriteValue(skr_json_writer_t* writer, T value)
{
    WriteValue(writer, static_cast<std::underlying_type_t<T>>(value));
}
template <class T>
void Write(skr_json_writer_t* writer, const T& value);

template <class T>
struct WriteHelper {
    static void Write(skr_json_writer_t* json, T map)
    {
        using TType = std::remove_const_t<std::remove_reference_t<T>>;
        WriteValue<TParamType<TType>>(json, map);
    }
};

template <class K, class V, class Hash, class Eq>
struct WriteHelper<const skr::flat_hash_map<K, V, Hash, Eq>&> {
    static void Write(skr_json_writer_t* json, const skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        json->StartObject();
        for (auto& pair : map)
        {
            skr::json::Write<const eastl::string&>(json, pair.first);
            skr::json::Write<TParamType<V>>(json, pair.second);
        }
        json->EndObject();
    }
};

template <class T>
struct WriteHelper<const skr::resource::TResourceHandle<T>&> {
    static void Write(skr_json_writer_t* json, const skr::resource::TResourceHandle<T>& handle)
    {
        WriteValue<const skr_resource_handle_t&>(json, (const skr_resource_handle_t&)handle);
    }
};

template <class V, class Allocator>
struct WriteHelper<const eastl::vector<V, Allocator>&> {
    static void Write(skr_json_writer_t* json, const eastl::vector<V, Allocator>& vec)
    {
        json->StartArray();
        for (auto& v : vec)
        {
            skr::json::Write<TParamType<V>>(json, v);
        }
        json->EndArray();
    }
};

template<class ...Ts>
struct WriteHelper<const skr::variant<Ts...>&>
{
    static void Write(skr_json_writer_t* json, const skr::variant<Ts...>& v)
    {
        std::visit([&](auto&& value) {
            using raw = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
            skr::json::WriteValue<const skr_guid_t&>(json, skr::type::type_id<raw>::get());
            skr::json::Write<decltype(value)>(json, value);
        }, v);
    }
};

template <class T>
void Write(skr_json_writer_t* writer, const T& value)
{
    WriteHelper<const T&>::Write(writer, value);
}
} // namespace json
} // namespace skr
#endif