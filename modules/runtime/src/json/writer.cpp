#include "resource/resource_handle.h"
#include "json/writer.h"
#include "utils/format.hpp"
#include "platform/debug.h"

skr_json_writer_t::skr_json_writer_t(size_t levelDepth)
{
    _levelStack.reserve(levelDepth);
}

eastl::string skr_json_writer_t::Str() const
{
    SKR_ASSERT(_levelStack.size() == 0);
    return { buffer.data(), buffer.size() };
}

bool skr_json_writer_t::Bool(bool b)
{
    _Prefix(SKR_JSONTYPE_BOOL);
    return _WriteBool(b);
}

bool skr_json_writer_t::Int(int32_t i)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteInt(i);
}

bool skr_json_writer_t::UInt(uint32_t i)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteUInt(i);
}

bool skr_json_writer_t::Int64(int64_t i)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteInt64(i);
}

bool skr_json_writer_t::UInt64(uint64_t i)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteUInt64(i);
}

bool skr_json_writer_t::Double(double d)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteDouble(d);
}

bool skr_json_writer_t::Float(float f)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteFloat(f);
}

bool skr_json_writer_t::RawNumber(const TChar* str, TSize length)
{
    _Prefix(SKR_JSONTYPE_NUMBER);
    return _WriteRawValue(str, length);
}

bool skr_json_writer_t::RawNumber(std::basic_string_view<TChar> view) { return RawNumber(view.data(), view.size()); }

bool skr_json_writer_t::String(const TChar* str, TSize length)
{
    _Prefix(SKR_JSONTYPE_STRING);
    return _WriteString(str, length);
}

bool skr_json_writer_t::String(std::basic_string_view<TChar> view) { return String(view.data(), view.size()); }

bool skr_json_writer_t::StartObject()
{
    _Prefix(SKR_JSONTYPE_OBJECT);
    _levelStack.emplace_back();
    return _WriteStartObject();
}

bool skr_json_writer_t::EndObject()
{
    SKR_ASSERT(_levelStack.size() > 0);                 // not inside an Object
    SKR_ASSERT(!_levelStack.back().isArray);            // currently inside an Array, not Object
    SKR_ASSERT(0 == _levelStack.back().valueCount % 2); // Object has a Key without a Value
    _levelStack.pop_back();
    return _WriteEndObject();
}

bool skr_json_writer_t::StartArray()
{
    _Prefix(SKR_JSONTYPE_ARRAY);
    _levelStack.push_back({ true, 0 });
    return _WriteStartArray();
}

bool skr_json_writer_t::EndArray()
{
    SKR_ASSERT(_levelStack.size() > 0);     // not inside an Object
    SKR_ASSERT(_levelStack.back().isArray); // currently inside an Array, not Object
    _levelStack.pop_back();
    return _WriteEndArray();
}

bool skr_json_writer_t::RawValue(const TChar* str, TSize length, ESkrJsonType type)
{
    _Prefix(type);
    return _WriteRawValue(str, length);
}

bool skr_json_writer_t::RawValue(std::basic_string_view<TChar> view, ESkrJsonType type) 
{
    return RawValue(view.data(), view.size(), type); 
}

bool skr_json_writer_t::_WriteBool(bool b)
{
    if (b)
        buffer.append(std::string_view("true"));
    else
        buffer.append(std::string_view("false"));
    return true;
}

bool skr_json_writer_t::_WriteInt(int32_t i)
{
    fmt::format_to(std::back_inserter(buffer), "{}", i);
    return true;
}

bool skr_json_writer_t::_WriteUInt(uint32_t i)
{
    fmt::format_to(std::back_inserter(buffer), "{}", i);
    return true;
}

bool skr_json_writer_t::_WriteInt64(int64_t i)
{
    fmt::format_to(std::back_inserter(buffer), "{}", i);
    return true;
}

bool skr_json_writer_t::_WriteUInt64(uint64_t i)
{
    fmt::format_to(std::back_inserter(buffer), "{}", i);
    return true;
}

bool skr_json_writer_t::_WriteFloat(float f)
{
    fmt::format_to(std::back_inserter(buffer), "{}", static_cast<double>(f));
    return true;
}

bool skr_json_writer_t::_WriteDouble(double d)
{
    fmt::format_to(std::back_inserter(buffer), "{}", d);
    return true;
}

bool skr_json_writer_t::_WriteString(const TChar* str, TSize length)
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

bool skr_json_writer_t::_WriteStartObject()
{
    buffer.push_back('{');
    return true;
}

bool skr_json_writer_t::_WriteEndObject()
{
    buffer.push_back('}');
    return true;
}

bool skr_json_writer_t::_WriteStartArray()
{
    buffer.push_back('[');
    return true;
}

bool skr_json_writer_t::_WriteEndArray()
{
    buffer.push_back(']');
    return true;
}

bool skr_json_writer_t::_WriteRawValue(const TChar* str, TSize length)
{
    buffer.append(str, str + length);
    return true;
}

bool skr_json_writer_t::_Prefix(ESkrJsonType type)
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

namespace skr::json
{
template <>
void WriteValue(skr_json_writer_t* writer, bool b) { writer->Bool(b); }
template <>
void WriteValue(skr_json_writer_t* writer, int32_t b) { writer->Int(b); }
template <>
void WriteValue(skr_json_writer_t* writer, uint32_t b) { writer->UInt(b); }
template <>
void WriteValue(skr_json_writer_t* writer, int64_t b) { writer->Int64(b); }
template <>
void WriteValue(skr_json_writer_t* writer, uint64_t b) { writer->UInt64(b); }
template <>
void WriteValue(skr_json_writer_t* writer, float b) { writer->Float(b); }
template <>
void WriteValue(skr_json_writer_t* writer, double b) { writer->Double(b); }
template <>
void WriteValue(skr_json_writer_t* writer, const eastl::string_view& str) { writer->String(str.data(), (TSize)str.size()); }
template <>
void WriteValue(skr_json_writer_t* writer, const eastl::string& str) { writer->String(str.data(), (TSize)str.size()); }
template <>
void WriteValue(skr_json_writer_t* writer, const skr_guid_t& guid)
{
    auto str = skr::format("{}", guid);
    writer->String(str.data(), (TSize)str.size());
}
template <>
void WriteValue(skr_json_writer_t* writer, const skr_resource_handle_t& handle)
{
    WriteValue<const skr_guid_t&>(writer, handle.get_serialized());
}
} // namespace skr::json