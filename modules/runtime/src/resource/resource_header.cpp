#include "resource/resource_header.h"

int skr_resource_header_t::ReadWithoutDeps(skr_binary_reader_t* reader)
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
    for (auto& dep : header.dependencies)
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
    for (auto& dep : header.dependencies)
    {
        ret = bin::Archive(writer, dep);
        if (ret != 0)
            return ret;
    }
    return ret;
}
} // namespace skr::binary

uint32_t skr_resource_record_t::AddReference(uint64_t requester, ESkrRequesterType requesterType)
{
    if (requesterType == SKR_REQUESTER_ENTITY)
    {
        entityRefCount++;
        auto iter = eastl::find_if(entityReferences.begin(), entityReferences.end(), [&](const entity_requester& id) { return id.storage == (void*)requester; });
        if(iter == entityReferences.end())
        {
            auto id = requesterCounter++;
            entityReferences.push_back(entity_requester{ id, (dual_storage_t*)requester, 1 });
            return id;
        }
        else 
        {
            iter->entityRefCount++;
            return iter->id;
        }
    }
    else
    {
        auto id = requesterCounter++;
        objectReferences.push_back(object_requester{ id, (void*)requester, requesterType });
        return id;
    }
}
void skr_resource_record_t::RemoveReference(uint32_t id, ESkrRequesterType requesterType)
{
    if (requesterType == SKR_REQUESTER_ENTITY)
    {
        entityRefCount--;
        auto iter = eastl::find_if(entityReferences.begin(), entityReferences.end(), [&](const entity_requester& re) { return re.id == id; });
        SKR_ASSERT(iter != entityReferences.end());
        if(--iter->entityRefCount == 0)
            entityReferences.erase_unsorted(iter);
    }
    else
        objectReferences.erase_first_unsorted(object_requester{ id, 0, requesterType });
}
bool skr_resource_record_t::IsReferenced() const
{
    return entityRefCount > 0 || objectReferences.size() > 0;
}