#include "SkrSerde/json/reader.h"
#include "SkrProfile/profile.h"

namespace skr::json
{
// primitive types
bool ReadTrait<bool>::Read(skr::archive::JsonReader* _json, bool& value)
{
    if (_json->Bool(value).has_value())
        return true;
    return false;
}

bool ReadTrait<int8_t>::Read(skr::archive::JsonReader* _json, int8_t& value)
{
    int32_t _value;
    if (_json->Int32(_value).has_value())
    {
        value = _value;
        return true;
    }
    return false;
}

bool ReadTrait<int16_t>::Read(skr::archive::JsonReader* _json, int16_t& value)
{
    int32_t _value;
    if (_json->Int32(_value).has_value())
    {
        value = _value;
        return true;
    }
    return false;
}

bool ReadTrait<int32_t>::Read(skr::archive::JsonReader* _json, int32_t& value)
{
    if (_json->Int32(value).has_value())
        return true;
    return false;
}

bool ReadTrait<int64_t>::Read(skr::archive::JsonReader* _json, int64_t& value)
{
    if (_json->Int64(value).has_value())
        return true;
    return false;
}

bool ReadTrait<uint8_t>::Read(skr::archive::JsonReader* _json, uint8_t& value)
{
    uint32_t _value;
    if (_json->UInt32(_value).has_value())
    {
        value = _value;
        return true;
    }
    return false;
}

bool ReadTrait<uint16_t>::Read(skr::archive::JsonReader* _json, uint16_t& value)
{
    uint32_t _value;
    if (_json->UInt32(_value).has_value())
    {
        value = _value;
        return true;
    }
    return false;
}

bool ReadTrait<uint32_t>::Read(skr::archive::JsonReader* _json, uint32_t& value)
{
    if (_json->UInt32(value).has_value())
        return true;
    return false;
}

bool ReadTrait<uint64_t>::Read(skr::archive::JsonReader* _json, uint64_t& value)
{
    if (_json->UInt64(value).has_value())
        return true;
    return false;
}

bool ReadTrait<float>::Read(skr::archive::JsonReader* _json, float& value)
{
    if (_json->Float(value).has_value())
        return true;
    return false;
}

bool ReadTrait<double>::Read(skr::archive::JsonReader* _json, double& value)
{
    if (_json->Double(value).has_value())
        return true;
    return false;
}

// skr types
bool ReadTrait<skr_float2_t>::Read(skr::archive::JsonReader* _json, skr_float2_t& value)
{
    size_t count;
    _json->StartArray(count);
    if (count != 2)
        return false;
    _json->Float(value.x);
    _json->Float(value.y);
    _json->EndArray();
    return true;
}

bool ReadTrait<skr_float3_t>::Read(skr::archive::JsonReader* _json, skr_float3_t& value)
{
    size_t count;
    _json->StartArray(count);
    if (count != 3)
        return false;
    _json->Float(value.x);
    _json->Float(value.y);
    _json->Float(value.z);
    _json->EndArray();
    return true;
}

bool ReadTrait<skr_float4_t>::Read(skr::archive::JsonReader* _json, skr_float4_t& value)
{
    size_t count;
    _json->StartArray(count);
    if (count != 4)
        return false;
    _json->Float(value.x);
    _json->Float(value.y);
    _json->Float(value.z);
    _json->Float(value.w);
    _json->EndArray();
    return true;
}

bool ReadTrait<skr_float4x4_t>::Read(skr::archive::JsonReader* _json, skr_float4x4_t& v)
{
    size_t count;
    _json->StartArray(count);
    if (count != 16)
        return false;
    _json->Float(v.M[0][0]);
    _json->Float(v.M[0][1]);
    _json->Float(v.M[0][2]);
    _json->Float(v.M[0][3]);
    _json->Float(v.M[1][0]);
    _json->Float(v.M[1][1]);
    _json->Float(v.M[1][2]);
    _json->Float(v.M[1][3]);
    _json->Float(v.M[2][0]);
    _json->Float(v.M[2][1]);
    _json->Float(v.M[2][2]);
    _json->Float(v.M[2][3]);
    _json->Float(v.M[3][0]);
    _json->Float(v.M[3][1]);
    _json->Float(v.M[3][2]);
    _json->Float(v.M[3][3]);
    _json->EndArray();
    return true;
}

bool ReadTrait<skr_rotator_t>::Read(skr::archive::JsonReader* _json, skr_rotator_t& value)
{
    size_t count;
    _json->StartArray(count);
    if (count != 3)
        return false;
    _json->Float(value.pitch);
    _json->Float(value.yaw);
    _json->Float(value.roll);
    _json->EndArray();
    return true;
}

bool ReadTrait<skr_quaternion_t>::Read(skr::archive::JsonReader* _json, skr_quaternion_t& value)
{
    size_t count;
    _json->StartArray(count);
    if (count != 4)
        return false;
    _json->Float(value.x);
    _json->Float(value.y);
    _json->Float(value.z);
    _json->Float(value.w);
    _json->EndArray();
    return true;
}

bool ReadTrait<skr_md5_t>::Read(skr::archive::JsonReader* _json, skr_md5_t& value)
{
    skr::String str;
    if (_json->String(str).has_value())
    {
        if (!skr_parse_md5(str.u8_str(), &value))
            return false;
        return true;
    }
    return false;
}

bool ReadTrait<skr_guid_t>::Read(skr::archive::JsonReader* _json, skr_guid_t& value)
{
    skr::String str;
    if (_json->String(str).has_value())
    {
        if (!skr::guid::make_guid(str.u8_str(), value))
            return false;
        return true;
    }
    return false;
}

bool ReadTrait<skr::String>::Read(skr::archive::JsonReader* _json, skr::String& value)
{
    SkrZoneScopedN("json::ReadTrait<skr::String>::Read");

    if (_json->String(value).has_value())
        return true;
    return false;
}
} // namespace skr::json