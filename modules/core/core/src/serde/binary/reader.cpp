#include "SkrBase/misc/bit.hpp"
#include "SkrBase/misc/demangle.hpp"
#include "SkrCore/memory/memory.h"
#include "SkrContainers/sptr.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrCore/log.h"
#include "SkrSerde/binary/reader.h"
#include <cmath>

namespace skr::binary
{
// primitive types
bool ReadTrait<bool>::Read(SBinaryReader* reader, bool& value)
{
    uint32_t v;
    if (!ReadTrait<uint32_t>::Read(reader, v))
    {
        SKR_LOG_FATAL(u8"failed to read boolean value!");
        return false;
    }
    value = v != 0;
    return true;
}

bool ReadTrait<int8_t>::Read(SBinaryReader* reader, int8_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<int16_t>::Read(SBinaryReader* reader, int16_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<int32_t>::Read(SBinaryReader* reader, int32_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<int64_t>::Read(SBinaryReader* reader, int64_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint8_t>::Read(SBinaryReader* reader, uint8_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint16_t>::Read(SBinaryReader* reader, uint16_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint32_t>::Read(SBinaryReader* reader, uint32_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<uint64_t>::Read(SBinaryReader* reader, uint64_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<float>::Read(SBinaryReader* reader, float& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<double>::Read(SBinaryReader* reader, double& value)
{
    return reader->read(&value, sizeof(value));
}

// skr types
bool ReadTrait<skr_float2_t>::Read(SBinaryReader* reader, skr_float2_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_float3_t>::Read(SBinaryReader* reader, skr_float3_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_float4_t>::Read(SBinaryReader* reader, skr_float4_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_float4x4_t>::Read(SBinaryReader* reader, skr_float4x4_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_rotator_t>::Read(SBinaryReader* reader, skr_rotator_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_quaternion_t>::Read(SBinaryReader* reader, skr_quaternion_t& value)
{
    return reader->read(&value, sizeof(value));
}

bool ReadTrait<skr_md5_t>::Read(SBinaryReader* reader, skr_md5_t& md5)
{
    return reader->read(&md5, sizeof(md5));
}

bool ReadTrait<skr_guid_t>::Read(SBinaryReader* reader, skr_guid_t& guid)
{
    return reader->read(&guid, sizeof(guid));
}

bool ReadBlob(SBinaryReader* reader, skr::BlobId& out_id)
{
    // TODO: blob 应该特别处理
    uint64_t size;
    if (!skr::binary::Read(reader, size))
    {
        SKR_LOG_FATAL(u8"failed to read blob size!");
        return false;
    }
    auto blob = skr::IBlob::Create(nullptr, size, false);
    if (blob == nullptr)
    {
        SKR_LOG_FATAL(u8"failed to create blob!");
        return false;
    }

    if (!reader->read(blob->get_data(), blob->get_size()))
    {
        SKR_LOG_FATAL(u8"failed to read blob content!");
        return false;
    }

    out_id = blob;
    return true;
}

bool ReadTrait<skr::BlobId>::Read(SBinaryReader* reader, skr::BlobId& out_blob)
{
    auto success = ReadBlob(reader, out_blob);
    if ((!success) || (out_blob == nullptr))
    {
        SKR_LOG_FATAL(u8"failed to create blob!");
        return false;
    }
    return true;
}

bool ReadTrait<skr::String>::Read(SBinaryReader* reader, skr::String& str)
{
    uint32_t size;
    if (!ReadTrait<uint32_t>::Read(reader, size))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size!");
        return false;
    }
    skr::InlineVector<char8_t, 64> temp;
    temp.resize_default(size);
    if (!reader->read((void*)temp.data(), temp.size()))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size!");
        return false;
    }
    str = skr::String(skr::StringView((const char8_t*)temp.data(), (int32_t)temp.size()));
    return true;
}

} // namespace skr::binary