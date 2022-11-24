#include "binary/writer.h"

namespace skr::binary
{
int WriteHelper<bool>::Write(skr_binary_writer_t* writer, bool value)
{
    return WriteHelper<uint32_t>::Write(writer, (uint32_t)value);
}

int WriteHelper<uint32_t>::Write(skr_binary_writer_t* writer, uint32_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<uint64_t>::Write(skr_binary_writer_t* writer, uint64_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<int32_t>::Write(skr_binary_writer_t* writer, int32_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<int64_t>::Write(skr_binary_writer_t* writer, int64_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<float>::Write(skr_binary_writer_t* writer, float value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<double>::Write(skr_binary_writer_t* writer, double value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr_float2_t&>::Write(skr_binary_writer_t* writer, const skr_float2_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr_float3_t&>::Write(skr_binary_writer_t* writer, const skr_float3_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr_float4_t&>::Write(skr_binary_writer_t* writer, const skr_float4_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr_rotator_t&>::Write(skr_binary_writer_t* writer, const skr_rotator_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr_quaternion_t&>::Write(skr_binary_writer_t* writer, const skr_quaternion_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr_float4x4_t&>::Write(skr_binary_writer_t* writer, const skr_float4x4_t& value)
{
    return WriteValue(writer, &value, sizeof(value));
}

int WriteHelper<const skr::string&>::Write(skr_binary_writer_t* writer, const skr::string& str)
{
    int ret = WriteHelper<uint32_t>::Write(writer, (uint32_t)str.size());
    if (ret != 0)
        return ret;
    return WriteValue(writer, str.data(), str.size());
}

int WriteHelper<const skr::string_view&>::Write(skr_binary_writer_t* writer, const skr::string_view& str)
{
    int ret = WriteHelper<uint32_t>::Write(writer, (uint32_t)str.size());
    if (ret != 0)
        return ret;
    return WriteValue(writer, str.data(), str.size());
}

int WriteHelper<const skr_guid_t&>::Write(skr_binary_writer_t* writer, const skr_guid_t& guid)
{
    return WriteValue(writer, &guid, sizeof(guid));
}

int WriteHelper<const skr_resource_handle_t&>::Write(skr_binary_writer_t* writer, const skr_resource_handle_t& handle)
{
    return WriteHelper<const skr_guid_t&>::Write(writer, handle.get_serialized());
}

int WriteHelper<const skr_blob_t&>::Write(skr_binary_writer_t* writer, const skr_blob_t& blob)
{
    int ret = WriteHelper<uint32_t>::Write(writer, (uint32_t)blob.size);
    if (ret != 0)
        return ret;
    return WriteValue(writer, blob.bytes, (uint32_t)blob.size);
}
} // namespace skr::binary