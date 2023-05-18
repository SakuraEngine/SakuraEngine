#include "resource/resource_handle.h"
#include "serde/json/writer.h"
#include "platform/debug.h"
#include "containers/string.hpp"

skr_json_writer_t::skr_json_writer_t(size_t levelDepth, skr_json_format_t format)
    : _format(format)
{
    _levelStack.reserve(levelDepth);
}

skr::string skr_json_writer_t::Str() const
{
    SKR_ASSERT(_levelStack.size() == 0);
    return skr::string(skr::string_view(buffer.u8_str(), buffer.size()));
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

bool skr_json_writer_t::RawNumber(skr::string_view view) { return RawNumber(view.u8_str(), view.size()); }

bool skr_json_writer_t::String(const TChar* str, TSize length)
{
    _Prefix(SKR_JSONTYPE_STRING);
    return _WriteString(str, length);
}

bool skr_json_writer_t::String(skr::string_view view) { return String(view.u8_str(), view.size()); }

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

bool skr_json_writer_t::RawValue(skr::string_view view, ESkrJsonType type)
{
    return RawValue(view.u8_str(), view.size(), type);
}

bool skr_json_writer_t::_WriteBool(bool b)
{
    if (b)
        buffer.append(u8"true");
    else
        buffer.append(u8"false");
    return true;
}

bool skr_json_writer_t::_WriteInt(int32_t i)
{
    buffer += skr::format(u8"{}", i);
    return true;
}

bool skr_json_writer_t::_WriteUInt(uint32_t i)
{
    buffer += skr::format(u8"{}", i);
    return true;
}

bool skr_json_writer_t::_WriteInt64(int64_t i)
{
    buffer += skr::format(u8"{}", i);
    return true;
}

bool skr_json_writer_t::_WriteUInt64(uint64_t i)
{
    buffer += skr::format(u8"{}", i);
    return true;
}

bool skr_json_writer_t::_WriteFloat(float f)
{
    buffer += skr::format(u8"{}", static_cast<double>(f));
    return true;
}

bool skr_json_writer_t::_WriteDouble(double d)
{
    buffer += skr::format(u8"{}", d);
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
    const auto n = buffer.size() + 2 + length * 6;
    buffer.raw().reserve((int32_t)n);
    buffer += u8'\"';
    for (TSize i = 0; i < length; ++i)
    {
        const char c = str[i];
        if (escape[static_cast<unsigned char>(c)])
        {
            buffer += u8'\\';
            buffer += escape[static_cast<unsigned char>(c)];
            if (escape[static_cast<unsigned char>(c)] == 'u')
            {
                buffer += u8'0';
                buffer += u8'0';
                buffer += hexDigits[static_cast<unsigned char>(c) >> 4];
                buffer += hexDigits[static_cast<unsigned char>(c) & 0xF];
            }
        }
        else
            buffer += c;
    }
    buffer += u8'\"';
    return true;
}

bool skr_json_writer_t::_WriteStartObject()
{
    buffer += u8'{';
    return true;
}

bool skr_json_writer_t::_WriteEndObject()
{
    if(_format.enable)
        _NewLine();
    buffer += u8'}';
    return true;
}

bool skr_json_writer_t::_WriteStartArray()
{
    buffer += u8'[';
    return true;
}

bool skr_json_writer_t::_WriteEndArray()
{
    if(_format.enable)
        _NewLine();
    buffer += u8']';
    return true;
}

bool skr_json_writer_t::_WriteRawValue(const TChar* str, TSize length)
{
    buffer.append(skr::string_view(str, (int32_t)length));
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
                buffer += u8','; // add comma if it is not the first element in array
            else                       // in object
                buffer += (level.valueCount % 2 == 0) ? ',' : ':';
        }
        if (!level.isArray && level.valueCount % 2 == 0)
            SKR_ASSERT(type == SKR_JSONTYPE_STRING); // if it's in object, then even number should be a name
            
        if(_format.enable)
        {
            bool newLineAndIndent = level.valueCount == 0 || level.isArray || level.valueCount % 2 == 0;
            if (newLineAndIndent)
                _NewLine();
        }
        level.valueCount++;
    }
    else
    {
        SKR_ASSERT(!_hasRoot); // Should only has one and only one root.
        _hasRoot = true;
    }
    return true;
}


bool skr_json_writer_t::_NewLine()
{
    static const char8_t indentLiteral[] = u8"                                                                      ";
    buffer += u8'\n';
    auto ident = _levelStack.size() * _format.indentSize;
    while (ident > 0)
    {
        auto n = std::min(ident, sizeof(indentLiteral) - 1);
        buffer.append(skr::string_view(indentLiteral, (int32_t)n));
        ident -= n;
    }
    return true;
}

namespace skr::json
{
void WriteTrait<const skr_guid_t&>::Write(skr_json_writer_t* writer, const skr_guid_t& guid)
{
    auto str = skr::format(u8"{}", guid);
    writer->String(str.u8_str(), (skr_json_writer_size_t)str.size());
}

void WriteTrait<const skr_md5_t&>::Write(skr_json_writer_t* writer, const skr_md5_t& md5)
{
    auto str = skr::format(u8"{}", md5);
    writer->String(str.u8_str(), (skr_json_writer_size_t)str.size());
}

void WriteTrait<const skr_resource_handle_t&>::Write(skr_json_writer_t* writer, const skr_resource_handle_t& handle)
{
    WriteTrait<const skr_guid_t&>::Write(writer, handle.get_serialized());
}

void WriteTrait<const skr_float2_t&>::Write(skr_json_writer_t* writer, const skr_float2_t& v)
{
    writer->StartArray();
    writer->Float(v.x);
    writer->Float(v.y);
    writer->EndArray();
};

void WriteTrait<const skr_float3_t&>::Write(skr_json_writer_t* writer, const skr_float3_t& v)
{
    writer->StartArray();
    writer->Float(v.x);
    writer->Float(v.y);
    writer->Float(v.z);
    writer->EndArray();
};

void WriteTrait<const skr_float4_t&>::Write(skr_json_writer_t* writer, const skr_float4_t& v)
{
    writer->StartArray();
    writer->Float(v.x);
    writer->Float(v.y);
    writer->Float(v.z);
    writer->Float(v.w);
    writer->EndArray();
};

void WriteTrait<const skr_rotator_t&>::Write(skr_json_writer_t *writer, const skr_rotator_t &v)
{
    writer->StartArray();
    writer->Float(v.pitch);
    writer->Float(v.yaw);
    writer->Float(v.roll);
    writer->EndArray();
};

void WriteTrait<const skr_quaternion_t&>::Write(skr_json_writer_t* writer, const skr_quaternion_t& v)
{
    writer->StartArray();
    writer->Float(v.x);
    writer->Float(v.y);
    writer->Float(v.z);
    writer->Float(v.w);
    writer->EndArray();
};

void WriteTrait<const skr_float4x4_t&>::Write(skr_json_writer_t* writer, const skr_float4x4_t& v)
{
    writer->StartArray();
    writer->Float(v.M[0][0]);
    writer->Float(v.M[0][1]);
    writer->Float(v.M[0][2]);
    writer->Float(v.M[0][3]);
    writer->Float(v.M[1][0]);
    writer->Float(v.M[1][1]);
    writer->Float(v.M[1][2]);
    writer->Float(v.M[1][3]);
    writer->Float(v.M[2][0]);
    writer->Float(v.M[2][1]);
    writer->Float(v.M[2][2]);
    writer->Float(v.M[2][3]);
    writer->Float(v.M[3][0]);
    writer->Float(v.M[3][1]);
    writer->Float(v.M[3][2]);
    writer->Float(v.M[3][3]);
    writer->EndArray();
};
} // namespace skr::json