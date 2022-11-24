#include "binary/reader.h"
#include "platform/memory.h"

namespace skr::binary
{
int ReadHelper<bool>::Read(skr_binary_reader_t* reader, bool& value)
{
    uint32_t v;
    int ret = ReadHelper<uint32_t>::Read(reader, v);
    if (ret != 0)
        return ret;
    value = v != 0;
    return ret;
}

int ReadHelper<skr::string>::Read(skr_binary_reader_t* reader, skr::string& str)
{
    uint32_t size;
    int ret = ReadHelper<uint32_t>::Read(reader, size);
    if (ret != 0)
        return ret;
    skr::string temp;
    temp.resize(size);
    ret = ReadValue(reader, (void*)temp.data(), temp.size());
    if (ret != 0)
        return ret;
    str = std::move(temp);
    return ret;
}

int ReadHelper<skr_guid_t>::Read(skr_binary_reader_t* reader, skr_guid_t& guid)
{
    return ReadValue(reader, &guid, sizeof(guid));
}

int ReadHelper<skr_resource_handle_t>::Read(skr_binary_reader_t* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    int ret = ReadHelper<skr_guid_t>::Read(reader, guid);
    if (ret != 0)
        return ret;
    handle.set_guid(guid);
    return ret;
}

int ReadHelper<skr_blob_t>::Read(skr_binary_reader_t* reader, skr_blob_t& blob)
{
    // TODO: blob 应该特别处理
    skr_blob_t temp;
    int ret = skr::binary::Read(reader, temp.size);
    if (ret != 0)
        return ret;
    temp.bytes = (uint8_t*)sakura_malloc(temp.size);
    ret = ReadValue(reader, temp.bytes, temp.size);
    if (ret != 0)
        return ret;
    blob = std::move(temp);
    return ret;
}

} // namespace skr::binary