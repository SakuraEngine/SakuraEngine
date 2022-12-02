#include "resource/resource_header.hpp"
#include "binary/writer.h"
#include "binary/reader.h"

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
int ReadHelper<skr_resource_header_t>::Read(skr_binary_reader_t *reader, skr_resource_header_t &header)
{
    namespace bin = skr::binary;
    int ret = header.ReadWithoutDeps(reader);
    uint32_t size = 0;
    ret = bin::Archive(reader, size);
    if (ret != 0)
        return ret;
    header.dependencies.resize(size);
    for (auto &dep : header.dependencies)
    {
        ret = bin::Archive(reader, dep);
        if (ret != 0)
            return ret;
    }
    return ret;
}

int WriteHelper<const skr_resource_header_t&>::Write(skr_binary_writer_t *writer, const skr_resource_header_t &header)
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
    for (auto &dep : header.dependencies)
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
    else if(requesterType == SKR_REQUESTER_SCRIPT)
    {
        scriptRefCount++;
        auto iter = eastl::find_if(scriptReferences.begin(), scriptReferences.end(), [&](const script_requester& id) { return id.state == (void*)requester; });
        if (iter == scriptReferences.end())
        {
            auto id = requesterCounter++;
            scriptReferences.push_back(script_requester{ id, (lua_State*)requester, 1 });
            return id;
        }
        else
        {
            iter->scriptRefCount++;
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
    else if(requesterType == SKR_REQUESTER_SCRIPT)
    {
        scriptRefCount--;
        auto iter = eastl::find_if(scriptReferences.begin(), scriptReferences.end(), [&](const script_requester& re) { return re.id == id; });
        SKR_ASSERT(iter != scriptReferences.end());
        if (--iter->scriptRefCount == 0)
            scriptReferences.erase_unsorted(iter);
    }
    else
    {
        auto iter = eastl::find_if(objectReferences.begin(), objectReferences.end(), [&](const object_requester& re) { return re.id == id; });
        SKR_ASSERT(iter != objectReferences.end());
        objectReferences.erase_unsorted(iter);
    }
}
bool skr_resource_record_t::IsReferenced() const
{
    return entityRefCount > 0 || objectReferences.size() > 0;
}
void skr_resource_record_t::SetStatus(ESkrLoadingStatus newStatus)
{
    if(newStatus != loadingStatus)
    {
        SMutexLock lock(mutex.mMutex);
        loadingStatus = newStatus;
        if(!callbacks[newStatus].empty())
        {
            for(auto& callback : callbacks[newStatus])
                callback();
            callbacks[newStatus].clear();
        }
    }
}
void skr_resource_record_t::AddCallback(ESkrLoadingStatus status, void (*callback)(void *), void *userData)
{
    SMutexLock lock(mutex.mMutex);
    callbacks[status].push_back({ userData, callback });
}