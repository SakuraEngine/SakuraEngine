#pragma once
#include <stdint.h>

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
    #include <sstream>
    #include "utils/allocator.hpp"

struct RUNTIME_API skr_json_writer_t {
public:
    using TChar = char;
    using TSize = size_t;
    // TODO: stringstream is slow
    using TStream = std::basic_stringstream<TChar, std::char_traits<TChar>, skr::mi_allocator<TChar>>;
    TStream& _os;

    struct Level {
        bool isArray = false;
        uint32_t valueCount = 0;
    };

    eastl::vector<Level> _levelStack;

    skr_json_writer_t(TStream& os, size_t levelDepth)
        : _os(os)
    {
        _levelStack.reserve(levelDepth);
    }

    bool IsComplete();

    bool Bool(bool b) { return false; }
    bool Int(int32_t i) { return false; }
    bool UInt(uint32_t i) { return false; }
    bool Int64(int64_t i) { return false; }
    bool UInt64(uint64_t i) { return false; }
    bool Double(double d) { return false; }
    bool RawNumber(const TChar* str, TSize length) { return false; }
    bool String(const TChar* str, TSize length) { return false; }
    bool StartObject() { return false; }
    bool Key(const TChar* str, TSize length) { return false; }
    bool EndObject() { return false; }
    bool StartArray() { return false; }
    bool EndArray() { return false; }
    bool RawValue(const TChar* str, TSize length, ESkrJsonType type) { return false; }

    bool _WriteBool(bool b) { return false; }
    bool _WriteInt(int32_t i) { return false; }
    bool _WriteUInt(uint32_t i) { return false; }
    bool _WriteInt64(int64_t i) { return false; }
    bool _WriteUInt64(uint64_t i) { return false; }
    bool _WriteDouble(double d) { return false; }
    bool _WriteString(const TChar* str, TSize length) { return false; }
    bool _WriteStartObject() { return false; }
    bool _WriteEndObject() { return false; }
    bool _WriteStartArray() { return false; }
    bool _WriteEndArray() { return false; }
    bool _WriteRawValue(const TChar* str, TSize length) { return false; }
    bool _Prefix(ESkrJsonType type) { return false; }
    bool _EndValue(bool ret) { return false; }
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
void Write(skr_json_writer_t* writer, T value);

template <>
RUNTIME_API void Write(skr_json_writer_t* writer, bool b);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, int32_t b);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, uint32_t b);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, int64_t b);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, uint64_t b);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, double b);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, const eastl::string& str);
template <>
RUNTIME_API void Write(skr_json_writer_t* writer, const skr_guid_t& guid);
} // namespace json
} // namespace skr
#endif