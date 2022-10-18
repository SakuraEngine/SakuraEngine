#include "binary/writer.h"

namespace skr::binary
{
template <>
int WriteValue(binary_writer_t* writer, uint32_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}
template <>
int WriteValue(binary_writer_t* writer, uint64_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}
template <>
int WriteValue(binary_writer_t* writer, int32_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}
template <>
int WriteValue(binary_writer_t* writer, int64_t value)
{
    return WriteValue(writer, &value, sizeof(value));
}
template <>
int WriteValue(binary_writer_t* writer, float value)
{
    return WriteValue(writer, &value, sizeof(value));
}
template <>
int WriteValue(binary_writer_t* writer, double value)
{
    return WriteValue(writer, &value, sizeof(value));
}
template <>
int WriteValue(binary_writer_t* writer, const eastl::string& str)
{
    int ret = WriteValue(writer, (uint32_t)str.size());
    if(ret != 0)
        return ret;
    return WriteValue(writer, str.data(), str.size());
}
template <>
int WriteValue(binary_writer_t* writer, const eastl::string_view& str)
{
    int ret = WriteValue(writer, (uint32_t)str.size());
    if(ret != 0)
        return ret;
    return WriteValue(writer, str.data(), str.size());
}
template <>
int WriteValue(binary_writer_t* writer, const skr_guid_t& guid)
{
    return WriteValue(writer, &guid, sizeof(guid));
}
template <>
int WriteValue(binary_writer_t* writer, const skr_resource_handle_t& handle)
{
    return WriteValue<const skr_guid_t&>(writer, handle.get_serialized());
}
}