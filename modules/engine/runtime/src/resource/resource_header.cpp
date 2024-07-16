#include "SkrRT/resource/resource_header.hpp"

bool skr_resource_header_t::ReadWithoutDeps(SBinaryReader* reader)
{
    uint32_t function = 1;
    if (!skr::bin_read(reader, function))
        return false;
    if (!skr::bin_read(reader, version))
        return false;
    if (!skr::bin_read(reader, guid))
        return false;
    if (!skr::bin_read(reader, type))
        return false;
    return true;
}

namespace skr
{
bool BinSerde<skr_resource_header_t>::read(SBinaryReader* r, skr_resource_header_t& v)
{
    if (!v.ReadWithoutDeps(r))
        return false;
    uint32_t size = 0;
    if (!bin_read(r, size))
        return false;
    v.dependencies.resize_default(size);
    for (uint32_t i = 0; i < size; i++)
    {
        if (!bin_read(r, v.dependencies[i]))
            return false;
    }
    return true;
}

bool BinSerde<skr_resource_header_t>::write(SBinaryWriter* w, const skr_resource_header_t& v)
{
    uint32_t function = 1;
    if (!bin_write(w, function))
        return false;
    if (!bin_write(w, v.version))
        return false;
    if (!bin_write(w, v.guid))
        return false;
    if (!bin_write(w, v.type))
        return false;
    const auto dependencies_size = (uint32_t)v.dependencies.size();
    if (!bin_write(w, dependencies_size))
        return false;
    for (auto& dep : v.dependencies)
    {
        if (!bin_write(w, dep))
            return false;
    }
    return true;
}
} // namespace skr

uint32_t skr_resource_record_t::AddReference(uint64_t requester, ESkrRequesterType requesterType)
{
#if TRACK_RESOURCE_REQUESTS
    SMutexLock lock(mutex.mMutex);
    if (requesterType == SKR_REQUESTER_ENTITY)
    {
        entityRefCount++;
        auto iter = std::find_if(entityReferences.begin(), entityReferences.end(), [&](const entity_requester& id) { return id.storage == (void*)requester; });
        if (iter == entityReferences.end())
        {
            auto id = requesterCounter++;
            entityReferences.push_back(entity_requester{ id, (sugoi_storage_t*)requester, 1 });
            return id;
        }
        else
        {
            iter->entityRefCount++;
            return iter->id;
        }
    }
    else if (requesterType == SKR_REQUESTER_SCRIPT)
    {
        scriptRefCount++;
        auto iter = std::find_if(scriptReferences.begin(), scriptReferences.end(), [&](const script_requester& id) { return id.state == (void*)requester; });
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
#else
    ++referenceCount;
#endif
}
void skr_resource_record_t::RemoveReference(uint32_t id, ESkrRequesterType requesterType)
{
#if TRACK_RESOURCE_REQUESTS
    SMutexLock lock(mutex.mMutex);
    if (requesterType == SKR_REQUESTER_ENTITY)
    {
        entityRefCount--;
        auto iter = std::find_if(entityReferences.begin(), entityReferences.end(), [&](const entity_requester& re) { return re.id == id; });
        SKR_ASSERT(iter != entityReferences.end());
        if (--iter->entityRefCount == 0)
            entityReferences.erase(iter);
    }
    else if (requesterType == SKR_REQUESTER_SCRIPT)
    {
        scriptRefCount--;
        auto iter = std::find_if(scriptReferences.begin(), scriptReferences.end(), [&](const script_requester& re) { return re.id == id; });
        SKR_ASSERT(iter != scriptReferences.end());
        if (--iter->scriptRefCount == 0)
            scriptReferences.erase(iter);
    }
    else
    {
        auto iter = std::find_if(objectReferences.begin(), objectReferences.end(), [&](const object_requester& re) { return re.id == id; });
        SKR_ASSERT(iter != objectReferences.end());
        objectReferences.erase(iter);
    }
#else
    --referenceCount;
#endif
}
bool skr_resource_record_t::IsReferenced() const
{
#if TRACK_RESOURCE_REQUESTS
    return entityRefCount > 0 || objectReferences.size() > 0 || scriptRefCount > 0;
#else
    return referenceCount > 0;
#endif
}
void skr_resource_record_t::SetStatus(ESkrLoadingStatus newStatus)
{
    if (newStatus != loadingStatus)
    {
        SMutexLock lock(mutex.mMutex);
        loadingStatus = newStatus;
        if (!callbacks[newStatus].empty())
        {
            for (auto& callback : callbacks[newStatus])
                callback();
            callbacks[newStatus].clear();
        }
    }
}
void skr_resource_record_t::AddCallback(ESkrLoadingStatus status, void (*callback)(void*), void* userData)
{
    SMutexLock lock(mutex.mMutex);
    callbacks[status].push_back({ userData, callback });
}