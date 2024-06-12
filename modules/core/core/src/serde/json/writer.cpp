#include "SkrBase/misc/debug.h"
#include "SkrProfile/profile.h"
#include "SkrContainers/string.hpp"
#include "SkrSerde/json/writer.h"

namespace skr::json
{
// primitive types
bool WriteTrait<bool>::Write(skr::json::Writer* writer, bool b)
{
    return writer->Bool(b).has_value();
}

bool WriteTrait<int8_t>::Write(skr::json::Writer* writer, int8_t i)
{
    return writer->Int(i).has_value();
}

bool WriteTrait<int16_t>::Write(skr::json::Writer* writer, int16_t i)
{
    return writer->Int(i).has_value();
}

bool WriteTrait<int32_t>::Write(skr::json::Writer* writer, int32_t i)
{
    return writer->Int(i).has_value();
}

bool WriteTrait<int64_t>::Write(skr::json::Writer* writer, int64_t i)
{
    return writer->Int64(i).has_value();
}

bool WriteTrait<uint8_t>::Write(skr::json::Writer* writer, uint8_t i)
{
    return writer->UInt(i).has_value();
}

bool WriteTrait<uint16_t>::Write(skr::json::Writer* writer, uint16_t i)
{
    return writer->UInt(i).has_value();
}

bool WriteTrait<uint32_t>::Write(skr::json::Writer* writer, uint32_t i)
{
    return writer->UInt(i).has_value();
}

bool WriteTrait<uint64_t>::Write(skr::json::Writer* writer, uint64_t i)
{
    return writer->UInt64(i).has_value();
}

bool WriteTrait<float>::Write(skr::json::Writer* writer, float f)
{
    return writer->Float(f).has_value();
}

bool WriteTrait<double>::Write(skr::json::Writer* writer, double d)
{
    return writer->Double(d).has_value();
}

#define TRUE_OR_RETURN_FALSE(x) if (!(x).has_value()) return false;

// skr types
bool WriteTrait<skr_float2_t>::Write(skr::json::Writer* writer, const skr_float2_t& v)
{
    TRUE_OR_RETURN_FALSE(writer->StartArray());
    TRUE_OR_RETURN_FALSE(writer->Float(v.x));
    TRUE_OR_RETURN_FALSE(writer->Float(v.y));
    TRUE_OR_RETURN_FALSE(writer->EndArray());
    return true;
};

bool WriteTrait<skr_float3_t>::Write(skr::json::Writer* writer, const skr_float3_t& v)
{
    TRUE_OR_RETURN_FALSE(writer->StartArray());
    TRUE_OR_RETURN_FALSE(writer->Float(v.x));
    TRUE_OR_RETURN_FALSE(writer->Float(v.y));
    TRUE_OR_RETURN_FALSE(writer->Float(v.z));
    TRUE_OR_RETURN_FALSE(writer->EndArray());
    return true;
};

bool WriteTrait<skr_float4_t>::Write(skr::json::Writer* writer, const skr_float4_t& v)
{
    TRUE_OR_RETURN_FALSE(writer->StartArray());
    TRUE_OR_RETURN_FALSE(writer->Float(v.x));
    TRUE_OR_RETURN_FALSE(writer->Float(v.y));
    TRUE_OR_RETURN_FALSE(writer->Float(v.z));
    TRUE_OR_RETURN_FALSE(writer->Float(v.w));
    TRUE_OR_RETURN_FALSE(writer->EndArray());
    return true;
};

bool WriteTrait<skr_float4x4_t>::Write(skr::json::Writer* writer, const skr_float4x4_t& v)
{
    TRUE_OR_RETURN_FALSE(writer->StartArray());
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[0][0]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[0][1]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[0][2]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[0][3]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[1][0]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[1][1]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[1][2]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[1][3]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[2][0]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[2][1]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[2][2]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[2][3]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[3][0]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[3][1]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[3][2]));
    TRUE_OR_RETURN_FALSE(writer->Float(v.M[3][3]));
    TRUE_OR_RETURN_FALSE(writer->EndArray());
    return true;
};

bool WriteTrait<skr_rotator_t>::Write(skr::json::Writer* writer, const skr_rotator_t& v)
{
    TRUE_OR_RETURN_FALSE(writer->StartArray());
    TRUE_OR_RETURN_FALSE(writer->Float(v.pitch));
    TRUE_OR_RETURN_FALSE(writer->Float(v.yaw));
    TRUE_OR_RETURN_FALSE(writer->Float(v.roll));
    TRUE_OR_RETURN_FALSE(writer->EndArray());
    return true;
};

bool WriteTrait<skr_quaternion_t>::Write(skr::json::Writer* writer, const skr_quaternion_t& v)
{
    TRUE_OR_RETURN_FALSE(writer->StartArray());
    TRUE_OR_RETURN_FALSE(writer->Float(v.x));
    TRUE_OR_RETURN_FALSE(writer->Float(v.y));
    TRUE_OR_RETURN_FALSE(writer->Float(v.z));
    TRUE_OR_RETURN_FALSE(writer->Float(v.w));
    TRUE_OR_RETURN_FALSE(writer->EndArray());
    return true;
};

bool WriteTrait<skr_guid_t>::Write(skr::json::Writer* writer, const skr_guid_t& guid)
{
    auto str = skr::format(u8"{}", guid);
    TRUE_OR_RETURN_FALSE(writer->String(str));
    return true;
}

bool WriteTrait<skr_md5_t>::Write(skr::json::Writer* writer, const skr_md5_t& md5)
{
    auto str = skr::format(u8"{}", md5);
    TRUE_OR_RETURN_FALSE(writer->String(str));
    return true;
}

// string types
bool WriteTrait<skr::StringView>::Write(skr::json::Writer* writer, const skr::StringView& str)
{
    TRUE_OR_RETURN_FALSE(writer->String(str));
    return true;
}

bool WriteTrait<skr::String>::Write(skr::json::Writer* writer, const skr::String& str)
{
    TRUE_OR_RETURN_FALSE(writer->String(str.view()));
    return true;
}

} // namespace skr::json

#undef TRUE_OR_RETURN_FALSE