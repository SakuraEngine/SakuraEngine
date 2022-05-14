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
void SResourceSystem::LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint32_t requester, ESkrRequesterType requesterType)
{
    // TODO: lock
    SKR_ASSERT(!handle.is_resolved());
    auto record = _GetOrCreateRecord(handle.get_guid());
    handle.set_resolved(record, requester, requesterType);
    record->references.push_back({ requester, requesterType });
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
    auto record = handle.get_record();
    SKR_ASSERT(record->loadingStatus != SKR_LOADING_STATUS_UNLOADED);
    auto iter = std::find(record->references.begin(), record->references.end(), skr_resource_record_t::requester_id{ handle.get_requester(), handle.get_requester_type() });
    if (iter != record->references.end())
        record->references.erase_unsorted(iter);
    auto guid = record->header.guid;
    handle.set_guid(guid);

    if (record->references.empty()) // unload
    {
        if (record->loadingStatus == SKR_LOADING_STATUS_ERROR || record->loadingStatus == SKR_LOADING_STATUS_UNLOADED)
        {
            _DestroyRecord(guid, record);
            return;
        }
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
            if (record->loadingStatus == SKR_LOADING_STATUS_INSTALLED)
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
        resourceIds.new_entities(&record->id, 1);
        record->header.guid = guid;
        resourceRecords.insert(std::make_pair(guid, record));
    }
    return record;
}

void SResourceSystem::_DestroyRecord(const skr_guid_t& guid, skr_resource_record_t* record)
{
    auto request = record->activeRequest;
    if (request)
    {
        // TODO: lock
        requests.erase_first_unsorted(request);
        SkrDelete(request);
    }
    resourceRecords.erase(guid);
    if (record->resource)
        resourceToRecord.erase(record->resource);
    resourceIds.free_entities(&record->id, 1);
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
        case SKR_LOADING_PHASE_FINISHED:
            currentPhase = SKR_LOADING_PHASE_REQUEST_RESOURCE;
            break;
        case SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST:
            currentPhase = SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST;
            break;
        case SKR_LOADING_PHASE_UNINSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_INSTALLED;
        }
        break;

        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADING;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_INSTALLING;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES;
        }
        break;

        case SKR_LOADING_PHASE_UNLOAD_RESOURCE: {
            if (!requestInstall)
            {
                currentPhase = SKR_LOADING_PHASE_FINISHED;
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
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
        }
        break;

        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADING;
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

        case SKR_LOADING_PHASE_FINISHED: {
            currentPhase = SKR_LOADING_PHASE_UNINSTALL_RESOURCE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNINSTALLING;
        }
        break;

        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNINSTALLING;
        }

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
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
    }
    else
    {
        currentPhase = SKR_LOADING_PHASE_LOAD_RESOURCE;
    }
}

void SResourceRequest::_LoadFinished()
{
    resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADED;
    if (!requestInstall) // only require data, we are done
    {
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        return;
    }
    // schedule loading for all runtime dependencies
    for (auto& dep : resourceRecord->header.dependencies)
        system->LoadResource(dep, requestInstall, resourceRecord->id, SKR_REQUESTER_DEPENDENCY);
    currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES;
}

void SResourceRequest::_InstallFinished()
{
    resourceRecord->loadingStatus = SKR_LOADING_STATUS_INSTALLED;
    currentPhase = SKR_LOADING_PHASE_FINISHED;
    return;
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
                SKR_LOG_FMT_ERROR("Resource {} failed to load, factory not found.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            factory = iter->second;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADING;
            auto status = factory->Load(resourceRecord);
            if (status == SKR_LOAD_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to load.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            else if (status == SKR_LOAD_STATUS_INPROGRESS)
            {
                currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE;
            }
            else if (status == SKR_LOAD_STATUS_SUCCEED)
            {
                _LoadFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE: {
            auto status = factory->UpdateLoad(resourceRecord);
            if (status == SKR_LOAD_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to load.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            else if (status == SKR_LOAD_STATUS_SUCCEED)
            {
                _LoadFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES: {
            // pass 1 - check for error
            bool failed = false;
            for (auto& dep : resourceRecord->header.dependencies)
            {
                if (dep.get_status() == ESkrLoadingStatus::SKR_LOADING_STATUS_ERROR)
                {
                    SKR_LOG_FMT_ERROR("Resource {} failed to load dependency resource {}.", resourceRecord->header.guid, dep.get_serialized());
                    failed = true;
                    break;
                }
            }
            if (failed)
            {
                for (auto& dep : resourceRecord->header.dependencies)
                    system->UnloadResource(dep);
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
                factory->Unload(resourceRecord);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                break;
            }
            // pass 2 - check for loading
            bool completed = true;
            for (auto& dep : resourceRecord->header.dependencies)
            {
                if (dep.get_status() != ESkrLoadingStatus::SKR_LOADING_STATUS_INSTALLED)
                {
                    completed = false;
                    break;
                }
            }
            if (completed)
            {
                currentPhase = SKR_LOADING_PHASE_INSTALL_RESOURCE;
            }
        }
        break;
        case SKR_LOADING_PHASE_INSTALL_RESOURCE: {
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_INSTALLING;
            auto status = factory->Install(resourceRecord);
            if (status == SKR_INSTALL_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to install.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            else if (status == SKR_INSTALL_STATUS_INPROGRESS)
            {
                currentPhase = SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE;
            }
            else if (status == SKR_INSTALL_STATUS_SUCCEED)
            {
                _InstallFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE: {
            auto status = factory->UpdateInstall(resourceRecord);
            if (status == SKR_INSTALL_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to install.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
            else if (status == SKR_INSTALL_STATUS_SUCCEED)
            {
                _InstallFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE:
        case SKR_LOADING_PHASE_UNINSTALL_RESOURCE: {
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNINSTALLING;
            factory->Uninstall(resourceRecord);
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADED;
            currentPhase = SKR_LOADING_PHASE_UNLOAD_RESOURCE;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE:
        case SKR_LOADING_PHASE_UNLOAD_RESOURCE: {
            for (auto& dep : resourceRecord->header.dependencies)
                system->UnloadResource(dep);
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADING;
            factory->Unload(resourceRecord);
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
            currentPhase = SKR_LOADING_PHASE_FINISHED;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST: {
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADING;
            system->resourceProvider->CancelRequestFile(this);
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
            currentPhase = SKR_LOADING_PHASE_FINISHED;
        }
        break;
        default:
            SKR_UNREACHABLE_CODE();
            break;
    }
}

bool SResourceRequest::Yielded()
{
    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST:
        case SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE:
        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE:
            return true;
        default:
            return false;
    }
}

SResourceSystem* GetResourceSystem()
{
    static SResourceSystem system;
    return &system;
}
} // namespace skr::resource