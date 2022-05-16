#pragma once

typedef enum ESkrJsonType
{
    SKR_JSONTYPE_BOOL,
    SKR_JSONTYPE_NUMBER,
    SKR_JSONTYPE_STRING,
    SKR_JSONTYPE_OBJECT,
    SKR_JSONTYPE_ARRAY,
} ESkrJsonType;

#if defined(__cplusplus)
    #include <stdint.h>
    #include <iterator>
    #include <EASTL/vector.h>
    #include "utils/allocator.hpp"
    #include "resource/resource_handle.h"
    #include "fmt/format.h"
    #include <EASTL/string.h>
    #include "utils/hashmap.hpp"

struct RUNTIME_API skr_json_writer_t {
public:
    using TChar = char;
    using TSize = size_t;
    fmt::memory_buffer buffer;
    bool _hasRoot = false;

    struct Level {
        bool isArray = false;
        uint32_t valueCount = 0;
    };

    eastl::vector<Level> _levelStack;

    skr_json_writer_t(size_t levelDepth)
    {
        _levelStack.reserve(levelDepth);
    }

    eastl::string Str() const
    {
        return { buffer.data(), buffer.size() };
    }

    bool IsComplete() { return _hasRoot && _levelStack.empty(); }

    bool Bool(bool b)
    {
        _Prefix(SKR_JSONTYPE_BOOL);
        return _WriteBool(b);
    }
    bool Int(int32_t i)
    {
        _Prefix(SKR_JSONTYPE_NUMBER);
        return _WriteInt(i);
    }
    bool UInt(uint32_t i)
    {
        _Prefix(SKR_JSONTYPE_NUMBER);
        return _WriteUInt(i);
    }
    bool Int64(int64_t i)
    {
        _Prefix(SKR_JSONTYPE_NUMBER);
        return _WriteInt64(i);
    }
    bool UInt64(uint64_t i)
    {
        _Prefix(SKR_JSONTYPE_NUMBER);
        return _WriteUInt64(i);
    }
    bool Double(double d)
    {
        _Prefix(SKR_JSONTYPE_NUMBER);
        return _WriteDouble(d);
    }
    bool RawNumber(const TChar* str, TSize length)
    {
        _Prefix(SKR_JSONTYPE_NUMBER);
        return _WriteRawValue(str, length);
    }
    bool RawNumber(std::basic_string_view<TChar> view) { return RawNumber(view.data(), view.size()); }
    bool String(const TChar* str, TSize length)
    {
        _Prefix(SKR_JSONTYPE_STRING);
        return _WriteString(str, length);
    }
    bool String(std::basic_string_view<TChar> view) { return String(view.data(), view.size()); }
    bool StartObject()
    {
        _Prefix(SKR_JSONTYPE_OBJECT);
        _levelStack.emplace_back();
        return _WriteStartObject();
    }
    bool Key(const TChar* str, TSize length) { return String(str, length); }
    bool Key(std::basic_string_view<TChar> view) { return String(view.data(), view.size()); }
    bool EndObject()
    {
        SKR_ASSERT(_levelStack.size() > 0);                 // not inside an Object
        SKR_ASSERT(!_levelStack.back().isArray);            // currently inside an Array, not Object
        SKR_ASSERT(0 == _levelStack.back().valueCount % 2); // Object has a Key without a Value
        _levelStack.pop_back();
        return _WriteEndObject();
    }
    bool StartArray()
    {
        _Prefix(SKR_JSONTYPE_ARRAY);
        _levelStack.push_back({ true, 0 });
        return _WriteStartArray();
    }
    bool EndArray()
    {
        SKR_ASSERT(_levelStack.size() > 0);     // not inside an Object
        SKR_ASSERT(_levelStack.back().isArray); // currently inside an Array, not Object
        _levelStack.pop_back();
        return _WriteEndArray();
    }
    bool RawValue(const TChar* str, TSize length, ESkrJsonType type)
    {
        _Prefix(type);
        return _WriteRawValue(str, length);
    }
    bool RawValue(std::basic_string_view<TChar> view, ESkrJsonType type) { return RawValue(view.data(), view.size(), type); }

    bool _WriteBool(bool b)
    {
        if (b)
            buffer.append(std::string_view("true"));
        else
            buffer.append(std::string_view("false"));
        return true;
    }
    bool _WriteInt(int32_t i)
    {
        fmt::format_to(std::back_inserter(buffer), "{}", i);
        return true;
    }
    bool _WriteUInt(uint32_t i)
    {
        fmt::format_to(std::back_inserter(buffer), "{}", i);
        return true;
    }
    bool _WriteInt64(int64_t i)
    {
        fmt::format_to(std::back_inserter(buffer), "{}", i);
        return true;
    }
    bool _WriteUInt64(uint64_t i)
    {
        fmt::format_to(std::back_inserter(buffer), "{}", i);
        return true;
    }
    bool _WriteDouble(double d)
    {
        fmt::format_to(std::back_inserter(buffer), "{}", d);
        return true;
    }
    bool _WriteString(const TChar* str, TSize length)
    {
        static const char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        static const char escape[256] = {
    #define Z16 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            // 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
            0, 0, '"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                               // 20
            Z16, Z16,                                                                       // 30~4F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0,                              // 50
            Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16                                // 60~FF
    #undef Z16
        };
        buffer.reserve(buffer.size() + 2 + length * 6);
        buffer.push_back('\"');
        for (TSize i = 0; i < length; ++i)
        {
            const char c = str[i];
            if (escape[static_cast<unsigned char>(c)])
            {
                buffer.push_back('\\');
                buffer.push_back(escape[static_cast<unsigned char>(c)]);
                if (escape[static_cast<unsigned char>(c)] == 'u')
                {
                    buffer.push_back('0');
                    buffer.push_back('0');
                    buffer.push_back(hexDigits[static_cast<unsigned char>(c) >> 4]);
                    buffer.push_back(hexDigits[static_cast<unsigned char>(c) & 0xF]);
                }
            }
            else
                buffer.push_back(c);
        }
        buffer.push_back('\"');
        return true;
    }
    bool _WriteStartObject()
    {
        buffer.push_back('{');
        return true;
    }
    bool _WriteEndObject()
    {
        buffer.push_back('}');
        return true;
    }
    bool _WriteStartArray()
    {
        buffer.push_back('[');
        return true;
    }
    bool _WriteEndArray()
    {
        buffer.push_back(']');
        return true;
    }
    bool _WriteRawValue(const TChar* str, TSize length)
    {
        buffer.append(str, str + length);
        return true;
    }
    bool _Prefix(ESkrJsonType type)
    {
        if (_levelStack.size() != 0)
        { // this value is not at root
            Level& level = _levelStack.back();
            if (level.valueCount > 0)
            {
                if (level.isArray)
                    buffer.push_back(','); // add comma if it is not the first element in array
                else                       // in object
                    buffer.push_back((level.valueCount % 2 == 0) ? ',' : ':');
            }
            if (!level.isArray && level.valueCount % 2 == 0)
                SKR_ASSERT(type == SKR_JSONTYPE_STRING); // if it's in object, then even number should be a name
            level.valueCount++;
        }
        else
        {
            SKR_ASSERT(!_hasRoot); // Should only has one and only one root.
            _hasRoot = true;
        }
        return true;
    }
};
#else
typedef struct skr_json_writer_t skr_json_writer_t;
#endif

#if defined(__cplusplus)
    #include "EASTL/string.h"
// utils for codegen
typedef struct skr_guid_t skr_guid_t;
namespace skr
{
namespace json
{
using TChar = skr_json_writer_t::TChar;
using TSize = skr_json_writer_t::TSize;
template <class T>
using TParamType = std::conditional_t<std::is_fundamental_v<T> || std::is_enum_v<T>, T, const T&>;
template <class T>
void WriteValue(skr_json_writer_t* writer, T value);

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
RUNTIME_API void WriteValue(skr_json_writer_t* writer, double b);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const eastl::string& str);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const skr_guid_t& guid);
template <>
RUNTIME_API void WriteValue(skr_json_writer_t* writer, const skr_resource_handle_t& handle);

template <class T>
void Write(skr_json_writer_t* writer, T value);

template <class T>
struct WriteHelper {
    static void Write(skr_json_writer_t* json, T map)
    {
        WriteValue<T>(json, map);
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
        skr::json::Write<const skr_resource_handle_t&>(json, (const skr_resource_handle_t&)handle);
    }
};

template <class T>
void Write(skr_json_writer_t* writer, T value)
{
    WriteHelper<T>::Write(writer, value);
}
} // namespace json
} // namespace skr
#endif