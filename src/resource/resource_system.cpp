#include "resource/resource_system.h"
#include "platform/debug.h"

namespace skr::resource
{
    skr_resource_record_t* SResourceSystem::_GetRecord(const skr_guid_t& guid)
    {
        auto iter = resourceRecords.find(guid);
        return iter == resourceRecords.end() ? nullptr : iter->second;
    }
    skr_resource_record_t* SResourceSystem::_GetRecord(void* resource)
    {
        auto iter = resourceToRecord.find(resource);
        return iter == resourceToRecord.end() ? nullptr : iter->second;
    }
    void SResourceSystem::LoadResource(skr_guid_t &handle, bool requireInstalled, uint32_t requester)
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
    void SResourceSystem::Initialize(SResourceRegistry* provider)
    {
        resourceProvider = provider;
    }
    bool SResourceSystem::IsInitialized()
    {
        return resourceProvider != nullptr;
    }
    SResourceSystem* GetResourceSystem()
    {
        static SResourceSystem system;
        return &system;
    }
}