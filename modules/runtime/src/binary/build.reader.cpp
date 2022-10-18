#include "binary/reader.h"

namespace skr::binary
{

template <>
int ReadValue(binary_reader_t* reader, eastl::string& str)
{
    uint32_t size;
    int ret = ReadValue(reader, size);
    if (ret != 0)
        return ret;
    eastl::string temp;
    temp.resize(size);
    ret = ReadValue(reader, (void*)temp.data(), temp.size());
    if (ret != 0)
        return ret;
    str = std::move(temp);
    return ret;
}
template <>
int ReadValue(binary_reader_t* reader, skr_guid_t& guid)
{
    return ReadValue(reader, &guid, sizeof(guid));
}
template <>
int ReadValue(binary_reader_t* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    int ret = ReadValue(reader, guid);
    if (ret != 0)
        return ret;
    handle.set_guid(guid);
    return ret;
}

} // namespace skr::binary