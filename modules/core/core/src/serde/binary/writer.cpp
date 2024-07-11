#include "SkrBase/misc/bit.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrSerde/binary/writer.h"
#include "SkrBase/math/rtm/scalarf.h"
#include "SkrBase/math/rtm/scalard.h"
#include "SkrCore/log.h"

namespace skr::binary
{
// primitive types
bool WriteTrait<bool>::Write(SBinaryWriter* writer, bool value)
{
    return WriteTrait<uint32_t>::Write(writer, (uint32_t)value);
}

bool WriteTrait<int8_t>::Write(SBinaryWriter* writer, int8_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<int16_t>::Write(SBinaryWriter* writer, int16_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<int32_t>::Write(SBinaryWriter* writer, int32_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<int64_t>::Write(SBinaryWriter* writer, int64_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<uint8_t>::Write(SBinaryWriter* writer, uint8_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<uint16_t>::Write(SBinaryWriter* writer, uint16_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<uint32_t>::Write(SBinaryWriter* writer, uint32_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<uint64_t>::Write(SBinaryWriter* writer, uint64_t value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<float>::Write(SBinaryWriter* writer, float value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<double>::Write(SBinaryWriter* writer, double value)
{
    return writer->write(&value, sizeof(value));
}

// skr types
bool WriteTrait<skr_float2_t>::Write(SBinaryWriter* writer, const skr_float2_t& value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<skr_float3_t>::Write(SBinaryWriter* writer, const skr_float3_t& value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<skr_float4_t>::Write(SBinaryWriter* writer, const skr_float4_t& value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<skr_float4x4_t>::Write(SBinaryWriter* writer, const skr_float4x4_t& value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<skr_rotator_t>::Write(SBinaryWriter* writer, const skr_rotator_t& value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<skr_quaternion_t>::Write(SBinaryWriter* writer, const skr_quaternion_t& value)
{
    return writer->write(&value, sizeof(value));
}

bool WriteTrait<skr_guid_t>::Write(SBinaryWriter* writer, const skr_guid_t& guid)
{
    return writer->write(&guid, sizeof(guid));
}

bool WriteTrait<skr_md5_t>::Write(SBinaryWriter* writer, const skr_md5_t& md5)
{
    return writer->write(&md5, sizeof(md5));
}

bool WriteTrait<skr::IBlob*>::Write(SBinaryWriter* writer, const skr::IBlob*& blob)
{
    if (!WriteTrait<uint64_t>::Write(writer, (uint64_t)blob->get_size()))
        return false;
    return writer->write(blob->get_data(), (uint64_t)blob->get_size());
}

bool WriteTrait<skr::BlobId>::Write(SBinaryWriter* writer, const skr::BlobId& blob)
{
    if (!WriteTrait<uint64_t>::Write(writer, (uint64_t)blob->get_size()))
        return false;
    return writer->write(blob->get_data(), (uint64_t)blob->get_size());
}

// other skr types
bool WriteTrait<skr::String>::Write(SBinaryWriter* writer, const skr::String& str)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
        return false;
    return writer->write(str.u8_str(), str.size());
}

bool WriteTrait<skr::StringView>::Write(SBinaryWriter* writer, const skr::StringView& str)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
        return true;
    return writer->write(str.raw().data(), str.size());
}
} // namespace skr::binary
