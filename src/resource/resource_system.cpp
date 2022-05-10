#include "resource/resource_system.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "resource/resource_handle.h"
#include "resource/resource_header.h"
#include "platform/vfs.h"
#include "resource/resource_factory.h"

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
void SResourceSystem::LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint32_t requester)
{
    // TODO: lock
    SKR_ASSERT(!handle.is_resolved());
    auto record = _GetOrCreateRecord(handle.get_guid());
    handle.reset();
    handle.pointer = (int64_t)record;
    handle.requester = requester;
    record->references.push_back(requester);
    if ((!requireInstalled && record->loadingStatus == SKR_LOADING_STATUS_LOADED) ||
        (requireInstalled && record->loadingStatus == SKR_LOADING_STATUS_INSTALLED)) // already loaded
        return;
    auto request = record->activeRequest;
    if (request)
    {
        request->UpdateLoad(requireInstalled);
    }
    else
    {
        auto request = SkrNew<SResourceRequest>();
        request->requestInstall = requireInstalled;
        request->resourceRecord = record;
        request->isLoading = true;
        request->system = this;
        request->currentPhase = SKR_LOADING_PHASE_REQUEST_RESOURCE;
        request->isHotReload = false;
        request->factory = nullptr;
        record->activeRequest = request;
        record->loadingStatus = SKR_LOADING_STATUS_LOADING;
        requests.push_back(request);
    }
}
void SResourceSystem::UnloadResource(skr_resource_handle_t& handle)
{
    // TODO: lock
    SKR_ASSERT(handle.is_resolved() && !handle.is_null());
    auto record = (skr_resource_record_t*)handle.pointer;
    SKR_ASSERT(record->loadingStatus != SKR_LOADING_STATUS_UNLOADED);
    auto iter = std::find(record->references.begin(), record->references.end(), handle.requester);
    if (iter != record->references.end())
        record->references.erase_unsorted(iter);
    handle.set_guid(record->header.guid);

    if (record->references.empty()) // unload
    {
        auto request = record->activeRequest;
        if (request)
        {
            request->UpdateUnload();
        }
        else // new unload
        {
            auto request = SkrNew<SResourceRequest>();
            request->requestInstall = false;
            request->resourceRecord = record;
            request->isLoading = false;
            request->system = this;
            if (record->loadingStatus == SKR_LOADING_STATUS_ERROR)
                request->currentPhase = SKR_LOADING_PHASE_UNLOAD_FAILED_RESOURCE;
            else if (record->loadingStatus == SKR_LOADING_STATUS_INSTALLED)
            {
                request->currentPhase = SKR_LOADING_PHASE_UNINSTALL_RESOURCE;
                record->loadingStatus = SKR_LOADING_STATUS_UNINSTALLING;
            }
            else if (record->loadingStatus == SKR_LOADING_STATUS_LOADED)
            {
                request->currentPhase = SKR_LOADING_PHASE_UNLOAD_RESOURCE;
                record->loadingStatus = SKR_LOADING_STATUS_UNLOADING;
            }
            else
                SKR_UNREACHABLE_CODE();
            request->isHotReload = false;
            request->factory = nullptr;
            record->activeRequest = request;
            requests.push_back(request);
        }
    }
}
void SResourceSystem::Initialize(SResourceRegistry* provider)
{
    resourceProvider = provider;
}
bool SResourceSystem::IsInitialized()
{
    return resourceProvider != nullptr;
}

skr_resource_record_t* SResourceSystem::_GetOrCreateRecord(const skr_guid_t& guid)
{
    // TODO: lock
    auto record = _GetRecord(guid);
    if (!record)
    {
        record = SkrNew<skr_resource_record_t>();
        record->header.guid = guid;
        resourceRecords.insert(std::make_pair(guid, record));
    }
    return record;
}

void SResourceSystem::_DestroyRecord(const skr_guid_t& guid, skr_resource_record_t* record)
{
    resourceRecords.erase(guid);
    if (record->resource)
        resourceToRecord.erase(record->resource);
    SkrDelete(record);
}

void SResourceRequest::UpdateLoad(bool requestInstall)
{
    if (isLoading)
        return;
    isLoading = true;
    resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADING;
    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_COMPLETE:
            currentPhase = SKR_LOADING_PHASE_REQUEST_RESOURCE;
            break;
        case SKR_LOADING_PHASE_UNINSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_COMPLETE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_INSTALLED;
        }
        break;

        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES;
        }
        break;

        case SKR_LOADING_PHASE_UNLOAD_RESOURCE: {
            if (!requestInstall)
            {
                currentPhase = SKR_LOADING_PHASE_COMPLETE;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADED;
            }
            else
                currentPhase = SKR_LOADING_PHASE_INSTALL_RESOURCE;
        }
        break;

        default:
            SKR_HALT();
            break;
    }
}

void SResourceRequest::UpdateUnload()
{
    if (!isLoading)
        return;
    isLoading = false;

    resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADING;

    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES;
        }
        break;

        case SKR_LOADING_PHASE_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_COMPLETE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
        }
        break;

        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE;
        }
        break;

        case SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES;
        }
        break;

        case SKR_LOADING_PHASE_INSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_UNLOAD_RESOURCE;
        }
        break;

        case SKR_LOADING_PHASE_COMPLETE:
        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_UNINSTALL_RESOURCE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNINSTALLING;
        }
        break;

        default:
            SKR_HALT();
            break;
    }
}

void SResourceRequest::OnRequestFileFinished()
{
    if (path.empty())
    {
        SKR_LOG_FMT_ERROR("Resource {} failed to load, file not found.", resourceRecord->header.guid);
        currentPhase = SKR_LOADING_PHASE_COMPLETE;
        resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
    }
    else
    {
        currentPhase = SKR_LOADING_PHASE_LOAD_RESOURCE;
    }
}

void SResourceRequest::Update()
{
    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_REQUEST_RESOURCE: {
            system->resourceProvider->RequestResourceFile(this);
            currentPhase = SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST;
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST:
            break;
        case SKR_LOADING_PHASE_LOAD_RESOURCE: {
            auto iter = system->resourceFactories.find(resourceRecord->header.type);
            if (iter == system->resourceFactories.end())
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to load, unknown type.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_COMPLETE;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            factory = iter->second;
            auto status = factory->Load(resourceRecord);
            if (status == SKR_LOAD_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to load.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_COMPLETE;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            else if (status == SKR_LOAD_STATUS_INPROGRESS)
            {
                currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE;
            }
            else if (status == SKR_LOAD_STATUS_SUCCEED)
            {
                currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES;
            }
        }
        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE:
        case SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_INSTALL_RESOURCE:
        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE:
        case SKR_LOADING_PHASE_UNINSTALL_RESOURCE:
        case SKR_LOADING_PHASE_UNLOAD_RESOURCE:
        case SKR_LOADING_PHASE_UNLOAD_FAILED_RESOURCE:
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST:
        case SKR_LOADING_PHASE_COMPLETE:
        default:
            SKR_UNREACHABLE_CODE();
            break;
    }
}

SResourceSystem* GetResourceSystem()
{
    static SResourceSystem system;
    return &system;
}
} // namespace skr::resource