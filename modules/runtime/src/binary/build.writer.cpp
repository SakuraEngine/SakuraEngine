#include "resource/resource_handle.h"
#include "binary/writer.h"
#include "binary/blob.h"

namespace skr::binary
{
int WriteTrait<const bool&>::Write(skr_binary_writer_t* writer, bool value)
{
    return WriteTrait<const uint32_t&>::Write(writer, (uint32_t)value);
}

int WriteTrait<const uint32_t&>::Write(skr_binary_writer_t* writer, uint32_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const uint64_t&>::Write(skr_binary_writer_t* writer, uint64_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const int32_t&>::Write(skr_binary_writer_t* writer, int32_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const int64_t&>::Write(skr_binary_writer_t* writer, int64_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const float&>::Write(skr_binary_writer_t* writer, float value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const double&>::Write(skr_binary_writer_t* writer, double value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float2_t&>::Write(skr_binary_writer_t* writer, const skr_float2_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float3_t&>::Write(skr_binary_writer_t* writer, const skr_float3_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float4_t&>::Write(skr_binary_writer_t* writer, const skr_float4_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr_rotator_t&>::Write(skr_binary_writer_t* writer, const skr_rotator_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr_quaternion_t&>::Write(skr_binary_writer_t* writer, const skr_quaternion_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float4x4_t&>::Write(skr_binary_writer_t* writer, const skr_float4x4_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteTrait<const skr::string&>::Write(skr_binary_writer_t* writer, const skr::string& str)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)str.size());
    if (ret != 0)
        return ret;
    return WriteValue(writer, str.data(), str.size());
}

int WriteTrait<const skr::string_view&>::Write(skr_binary_writer_t* writer, const skr::string_view& str)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)str.size());
    if (ret != 0)
        return ret;
    return WriteValue(writer, str.data(), str.size());
}

int WriteTrait<const skr::string_view&>::Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::string_view& str)
{
    auto ptr = (char*)str.data();
    auto buffer = (char*)arena.get_buffer();
    SKR_ASSERT(ptr >= buffer);
    auto offset = (uint32_t)(ptr - buffer);
    SKR_ASSERT(offset < arena.get_size());
    int ret = skr::binary::Write(writer, offset);
    if (ret != 0) {
        return ret;
    }
    return skr::binary::Write(writer, (uint32_t)str.length());
}

int WriteTrait<const skr_guid_t&>::Write(skr_binary_writer_t* writer, const skr_guid_t& guid)
{
    return WriteValue(writer, &guid, sizeof(guid));
}

int WriteTrait<const skr_md5_t&>::Write(skr_binary_writer_t* writer, const skr_md5_t& md5)
{
    return WriteValue(writer, &md5, sizeof(md5));
}

int WriteTrait<const skr_resource_handle_t&>::Write(skr_binary_writer_t* writer, const skr_resource_handle_t& handle)
{
    return WriteTrait<const skr_guid_t&>::Write(writer, handle.get_serialized());
}

int WriteTrait<const skr_blob_t&>::Write(skr_binary_writer_t* writer, const skr_blob_t& blob)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)blob.size);
    if (ret != 0)
        return ret;
    return WriteValue(writer, blob.bytes, (uint32_t)blob.size);
}

int WriteTrait<const skr_blob_arena_t&>::Write(skr_binary_writer_t* writer, const skr_blob_arena_t& blob)
{
    int ret = WriteTrait<const uint64_t&>::Write(writer, (uint64_t)blob.get_buffer());
    if (ret != 0)
        return ret;
    ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)blob.get_size());
    if (ret != 0)
        return ret;
    ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)blob.get_align());
    if (ret != 0)
        return ret;
    return WriteValue(writer, blob.get_buffer(), (uint32_t)blob.get_size());
}

void BlobTrait<skr::string_view>::BuildArena(skr_blob_arena_builder_t& arena, skr::string_view& dst, const skr::string& src)
{
    auto offset = arena.allocate((src.size() + 1) * sizeof(skr::string::value_type), alignof(skr::string::value_type));
    auto buffer = (char*)arena.get_buffer() + offset;
    memcpy(buffer, src.data(), (src.size() + 1) * sizeof(skr::string::value_type));
    memset(buffer + src.size(), 0, sizeof(skr::string::value_type)); //tailing zero
    dst = skr::string_view((const skr::string::value_type*)offset, src.size());
}

void BlobTrait<skr::string_view>::Remap(skr_blob_arena_t& arena, skr::string_view& dst)
{
    dst = skr::string_view((const skr::string::value_type*)((char*)arena.get_buffer() + ((size_t)dst.data() - arena.base())), dst.size());
}
} // namespace skr::binary