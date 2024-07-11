#include "SkrBase/misc/bit.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrSerde/binary/writer.h"
#include "SkrSerde/binary/blob.h"
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
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<int16_t>::Write(SBinaryWriter* writer, int16_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<int32_t>::Write(SBinaryWriter* writer, int32_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<int64_t>::Write(SBinaryWriter* writer, int64_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<uint8_t>::Write(SBinaryWriter* writer, uint8_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<uint16_t>::Write(SBinaryWriter* writer, uint16_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<uint32_t>::Write(SBinaryWriter* writer, uint32_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<uint64_t>::Write(SBinaryWriter* writer, uint64_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<float>::Write(SBinaryWriter* writer, float value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<double>::Write(SBinaryWriter* writer, double value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

// skr types
bool WriteTrait<skr_float2_t>::Write(SBinaryWriter* writer, const skr_float2_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<skr_float3_t>::Write(SBinaryWriter* writer, const skr_float3_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<skr_float4_t>::Write(SBinaryWriter* writer, const skr_float4_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<skr_float4x4_t>::Write(SBinaryWriter* writer, const skr_float4x4_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<skr_rotator_t>::Write(SBinaryWriter* writer, const skr_rotator_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<skr_quaternion_t>::Write(SBinaryWriter* writer, const skr_quaternion_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

bool WriteTrait<skr_guid_t>::Write(SBinaryWriter* writer, const skr_guid_t& guid)
{
    return WriteBytes(writer, &guid, sizeof(guid));
}

bool WriteTrait<skr_md5_t>::Write(SBinaryWriter* writer, const skr_md5_t& md5)
{
    return WriteBytes(writer, &md5, sizeof(md5));
}

bool WriteTrait<skr::IBlob*>::Write(SBinaryWriter* writer, const skr::IBlob*& blob)
{
    if (!WriteTrait<uint64_t>::Write(writer, (uint64_t)blob->get_size()))
        return false;
    return WriteBytes(writer, blob->get_data(), (uint64_t)blob->get_size());
}

bool WriteTrait<skr::BlobId>::Write(SBinaryWriter* writer, const skr::BlobId& blob)
{
    if (!WriteTrait<uint64_t>::Write(writer, (uint64_t)blob->get_size()))
        return false;
    return WriteBytes(writer, blob->get_data(), (uint64_t)blob->get_size());
}

bool WriteTrait<skr_blob_arena_t>::Write(SBinaryWriter* writer, const skr_blob_arena_t& blob)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)blob.get_size()))
        return false;
    if (blob.get_size() == 0)
        return true;
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)blob.get_align()))
        return false;
    return true;
}

// other skr types
bool WriteTrait<skr::String>::Write(SBinaryWriter* writer, const skr::String& str)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
        return false;
    return WriteBytes(writer, str.u8_str(), str.size());
}

bool WriteTrait<skr::StringView>::Write(SBinaryWriter* writer, const skr::StringView& str)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
        return true;
    return WriteBytes(writer, str.raw().data(), str.size());
}

bool WriteTrait<skr::StringView>::Write(SBinaryWriter* writer, skr_blob_arena_t& arena, const skr::StringView& str)
{
    auto ptr    = (ochar8_t*)str.raw().data();
    auto buffer = (ochar8_t*)arena.get_buffer();
    SKR_ASSERT(ptr >= buffer);
    auto offset = (uint32_t)(ptr - buffer);
    SKR_ASSERT(offset < arena.get_size());
    if (!skr::binary::Write(writer, (uint32_t)str.size()))
    {
        return false;
    }
    if (str.size() == 0)
        return true;
    if (!skr::binary::Write(writer, offset))
    {
        return true;
    }
    return WriteBytes(writer, str.raw().data(), str.size());
}

void BlobTrait<skr::StringView>::BuildArena(skr_blob_arena_builder_t& arena, skr::StringView& dst, const skr::String& src)
{
    auto raw    = src.raw();
    auto offset = arena.allocate((raw.size() + 1) * sizeof(ochar8_t), alignof(ochar8_t));
    auto buffer = (ochar8_t*)arena.get_buffer() + offset;
    memcpy(buffer, raw.c_str(), (raw.size() + 1) * sizeof(ochar8_t));
    memset(buffer + raw.size(), 0, sizeof(ochar8_t)); // tailing zero
    dst = skr::StringView((const ochar8_t*)offset, raw.size());
}

void BlobTrait<skr::StringView>::Remap(skr_blob_arena_t& arena, skr::StringView& dst)
{
    const auto unmapped_addr   = (std::uintptr_t)dst.raw().data();
    const auto offset_in_arena = (std::uintptr_t)arena.base();
    const auto base_ptr        = arena.get_buffer();
    const auto str_ptr         = reinterpret_cast<const char8_t*>(base_ptr);
    const auto _size           = dst.raw().size();
    const auto final_addr      = str_ptr + (unmapped_addr - offset_in_arena);
    dst                        = skr::StringView((char8_t*)final_addr, _size);
}

} // namespace skr::binary
