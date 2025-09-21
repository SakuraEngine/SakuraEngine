#include "SkrRuntime/resource/resource_header.hpp"

namespace skr
{
void Serialize<SResourceHeader>::read(ArchiveRead& r, SResourceHeader& v)
{
    Archive::ObjectScope obj_scope(r);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    uint32_t function = 0;

    SKR_FAST_CHECK(r.key_value(u8"function", function), );
    SKR_FAST_CHECK(r.key_value(u8"version", v.version), );
    SKR_FAST_CHECK(r.key_value(u8"guid", v.guid), );
    SKR_FAST_CHECK(r.key_value(u8"type", v.type), );

    SKR_FAST_CHECK(r.key(u8"dependencies"), );
    {
        Archive::ArrayScope arr_scope(r);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // read dependencies count
        uint64_t dependencies_size = 0;
        SKR_FAST_CHECK(r.array_size<uint64_t>(dependencies_size), );
        v.dependencies.resize_unsafe((size_t)dependencies_size);

        // read each dependency guid
        for (uint64_t i = 0; i < dependencies_size; i++)
        {
            SKR_FAST_CHECK(r.value(v.dependencies[i]), );
        }
    }
}
void Serialize<SResourceHeader>::write(ArchiveWrite& w, const SResourceHeader& v)
{
    Archive::ObjectScope obj_scope(w);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    uint32_t function = 1;

    SKR_FAST_CHECK(w.key_value(u8"function", function), );
    SKR_FAST_CHECK(w.key_value(u8"version", v.version), );
    SKR_FAST_CHECK(w.key_value(u8"guid", v.guid), );
    SKR_FAST_CHECK(w.key_value(u8"type", v.type), );

    SKR_FAST_CHECK(w.key(u8"dependencies"), );
    {
        Archive::ArrayScope arr_scope(w);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write dependencies count
        SKR_FAST_CHECK(w.array_size<uint64_t>((uint64_t)v.dependencies.size()), );

        // write each dependency guid
        for (const auto& dep : v.dependencies)
        {
            SKR_FAST_CHECK(w.value(dep), );
        }
    }
}
} // namespace skr

uint32_t SResourceRecord::AddReference(uint64_t requester, ESkrRequesterType requesterType)
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
void SResourceRecord::RemoveReference(uint32_t id, ESkrRequesterType requesterType)
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
bool SResourceRecord::IsReferenced() const
{
#if TRACK_RESOURCE_REQUESTS
    return entityRefCount > 0 || objectReferences.size() > 0 || scriptRefCount > 0;
#else
    return referenceCount > 0;
#endif
}
void SResourceRecord::SetStatus(ESkrLoadingStatus newStatus)
{
    if (newStatus != loadingStatus)
    {
        SMutexLock lock(mutex.mMutex);
        loadingStatus = newStatus;
        if (!callbacks[newStatus].is_empty())
        {
            for (auto& callback : callbacks[newStatus])
                callback();
            callbacks[newStatus].clear();
        }
    }
}
void SResourceRecord::AddCallback(ESkrLoadingStatus status, void (*callback)(void*), void* userData)
{
    SMutexLock lock(mutex.mMutex);
    callbacks[status].push_back({ userData, callback });
}