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

    #include "EASTL/vector.h"
    #include <sstream>
    #include "utils/allocator.hpp"

extern "C" class skr_json_writer_t
{
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

    bool Bool(bool b);
    bool Int(int32_t i);
    bool UInt(uint32_t i);
    bool Int64(int64_t i);
    bool UInt64(uint64_t i);
    bool Double(double d);
    bool RawNumber(const TChar* str, TSize length);
    bool String(const TChar* str, TSize length);
    bool StartObject();
    bool Key(const TChar* str, TSize length);
    bool EndObject();
    bool StartArray();
    bool EndArray();
    bool RawValue(const TChar* str, TSize length, ESkrJsonType type);

    bool _WriteBool(bool b);
    bool _WriteInt(int32_t i);
    bool _WriteUInt(uint32_t i);
    bool _WriteInt64(int64_t i);
    bool _WriteUInt64(uint64_t i);
    bool _WriteDouble(double d);
    bool _WriteString(const TChar* str, TSize length);
    bool _WriteStartObject();
    bool _WriteEndObject();
    bool _WriteStartArray();
    bool _WriteEndArray();
    bool _WriteRawValue(const TChar* str, TSize length);
    bool _Prefix(ESkrJsonType type);
    bool _EndValue(bool ret);
};
#else
typedef struct skr_json_writer_t skr_json_writer_t;
#endif

#if defined(__cplusplus)
    #include "EASTL/string.h"
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
void Write(skr_json_writer_t* writer, T value);
} // namespace json
} // namespace skr
#endif