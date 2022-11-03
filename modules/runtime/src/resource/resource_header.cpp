#include "resource/resource_header.h"

int skr_resource_header_t::ReadWithoutDeps(skr_binary_reader_t *reader)
{
    namespace bin = skr::binary;
    uint32_t function = 1;
    int ret = bin::Archive(reader, function);
    if (ret != 0)
        return ret;
    ret = bin::Archive(reader, version);
    if (ret != 0)
        return ret;
    ret = bin::Archive(reader, guid);
    if (ret != 0)
        return ret;
    ret = bin::Archive(reader, type);
    if (ret != 0)
        return ret;
    return 0;
}

namespace skr::binary
{
    template <>
    int ReadValue(skr_binary_reader_t* reader, skr_resource_header_t& header)
    {
        namespace bin = skr::binary;
        int ret = header.ReadWithoutDeps(reader);
        uint32_t size = 0;
        ret = bin::Archive(reader, size);
        if (ret != 0)
                return ret;
        header.dependencies.resize(size);
        for(auto& dep : header.dependencies)
        {
            ret = bin::Archive(reader, dep);
            if (ret != 0)
                return ret;
        }
        return ret;
    }
    template <>
    int WriteValue(skr_binary_writer_t* writer, const skr_resource_header_t& header)
    {
        namespace bin = skr::binary;
        uint32_t function = 1;
        int ret = bin::Archive(writer, function);
        if (ret != 0)
            return ret;
        ret = bin::Archive(writer, header.version);
        if (ret != 0)
            return ret;
        ret = bin::Archive(writer, header.guid);
        if (ret != 0)
            return ret;
        ret = bin::Archive(writer, header.type);
        if (ret != 0)
            return ret;
        ret = bin::Archive(writer, (uint32_t)header.dependencies.size());
        for(auto& dep : header.dependencies)
        {
            ret = bin::Archive(writer, dep);
            if (ret != 0)
                return ret;
        }
        return ret;
    }
} // namespace skr::binary