#include "resource/resource_system.h"
#include "platform/debug.h"
#include "platform/thread.h"
#include "utils/defer.hpp"
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "resource/resource_handle.h"
#include "resource/resource_header.h"
#include "platform/vfs.h"
#include "resource/resource_factory.h"

namespace skr::resource
{
SResourceSystem::SResourceSystem()
{
    skr_init_mutex(&recordMutex);
}

SResourceSystem::~SResourceSystem()
{
    skr_destroy_mutex(&recordMutex);
}

skr_resource_record_t* SResourceSystem::_GetOrCreateRecord(const skr_guid_t& guid)
{
    auto record = _GetRecord(guid);
    if (!record)
    {
        record = SkrNew<skr_resource_record_t>();
        resourceIds.new_entities(&record->id, 1);
        record->header.guid = guid;
        // record->header.type = type;
        resourceRecords.insert(std::make_pair(guid, record));
    }
    return record;
}

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

void SResourceSystem::_DestroyRecord(const skr_guid_t& guid, skr_resource_record_t* record)
{
    //SMutexLock lock(recordMutex);
    auto request = record->activeRequest;
    if (request)
        request->resourceRecord = nullptr;
    resourceRecords.erase(guid);
    if (record->resource)
        resourceToRecord.erase(record->resource);
    resourceIds.free_entities(&record->id, 1);
    SkrDelete(record);
}

SResourceFactory* SResourceSystem::FindFactory(skr_type_id_t type) const
{
    auto iter = resourceFactories.find(type);
    if (iter != resourceFactories.end()) return iter->second;
    return nullptr;
}

void SResourceSystem::RegisterFactory(skr_type_id_t type, SResourceFactory* factory)
{
    auto iter = resourceFactories.find(type);
    SKR_ASSERT(iter == resourceFactories.end());
    resourceFactories.insert(std::make_pair(type, factory));
}

SResourceRegistry* SResourceSystem::GetRegistry() const
{
    return resourceRegistry;
}

skr::io::RAMService* SResourceSystem::GetRAMService() const
{
    return ioService;
}

void SResourceSystem::UnregisterFactory(skr_type_id_t type)
{
    auto iter = resourceFactories.find(type);
    SKR_ASSERT(iter != resourceFactories.end());
    resourceFactories.erase(iter);
}

void SResourceSystem::LoadResource(skr_resource_handle_t& handle, bool requireInstalled, uint32_t requester, ESkrRequesterType requesterType)
{
    SMutexLock lock(recordMutex);
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
        request->requireLoading = true;
    }
    else
    {
        auto request = SkrNew<SResourceRequest>();
        request->requestInstall = requireInstalled;
        request->resourceRecord = record;
        request->isLoading = request->requireLoading = true;
        request->system = this;
        request->currentPhase = SKR_LOADING_PHASE_REQUEST_RESOURCE;
        request->factory = nullptr;
        request->vfs = nullptr;
        record->activeRequest = request;
        record->loadingStatus = SKR_LOADING_STATUS_LOADING;
        requests.push_back(request);
    }
}

void SResourceSystem::UnloadResource(skr_resource_handle_t& handle)
{
    SMutexLock lock(recordMutex);
    SKR_ASSERT(handle.is_resolved() && !handle.is_null());
    auto record = handle.get_record();
    SKR_ASSERT(record->loadingStatus != SKR_LOADING_STATUS_UNLOADED);
    record->references.erase_first_unsorted(skr_resource_record_t::requester_id{ handle.get_requester(), handle.get_requester_type() });
    auto guid = handle.guid = record->header.guid; // force flush handle to guid

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
            request->requireLoading = false;
        }
        else // new unload
        {
            auto request = SkrNew<SResourceRequest>();
            request->requestInstall = false;
            request->resourceRecord = record;
            request->isLoading = request->requireLoading = false;
            request->system = this;
            request->vfs = nullptr;
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
            request->factory = this->FindFactory(record->header.type);
            record->activeRequest = request;
            requests.push_back(request);
        }
    }
}


ESkrLoadingStatus SResourceSystem::GetResourceStatus(const skr_guid_t& handle)
{
    SMutexLock lock(recordMutex);
    auto record = _GetRecord(handle);
    if (!record) return SKR_LOADING_STATUS_UNLOADED;
    return record->loadingStatus;
}

void SResourceSystem::Initialize(SResourceRegistry* provider, skr::io::RAMService* service)
{
    SKR_ASSERT(provider);
    resourceRegistry = provider;
    ioService = service;
}

bool SResourceSystem::IsInitialized()
{
    return resourceRegistry != nullptr;
}

void SResourceSystem::Shutdown()
{
    resourceRegistry = nullptr;
}

void SResourceSystem::Update()
{
    {
        SMutexLock lock(recordMutex);
        requests.erase(std::remove_if(requests.begin(), requests.end(), [&](SResourceRequest* request) {
            if (request->Okay())
            {
                if (request->resourceRecord)
                {
                    request->resourceRecord->activeRequest = nullptr;
                    if (!request->isLoading)
                    {
                        auto guid = request->resourceRecord->header.guid;
                        _DestroyRecord(guid, request->resourceRecord);
                    }
                }
                SkrDelete(request);
                return true;
            }
            return false;
        }),
        requests.end());
    }
    auto currentRequests = requests;
    // TODO: time limit
    for (auto request : currentRequests)
    {
        uint32_t spinCounter = 0;
        ESkrLoadingPhase LastPhase;
        while(!request->Failed() && !request->Okay() && spinCounter < 16)
        {
            LastPhase = request->currentPhase;
            request->Update();
            if(LastPhase == request->currentPhase)
                spinCounter++;
            else
                spinCounter = 0;
        };
    }
}

// resource request implementation

skr_guid_t SResourceRequest::GetGuid() const
{
    return resourceRecord->header.guid;
}

gsl::span<const uint8_t> SResourceRequest::GetData() const
{
    return gsl::span<const uint8_t>(data, size); 
}

gsl::span<const skr_guid_t> SResourceRequest::GetDependencies() const
{
    return gsl::span<const skr_guid_t>(dependencies.data(), dependencies.size());
}

void SResourceRequest::UpdateLoad(bool requestInstall)
{
    if (isLoading)
        return;
    isLoading = true;
    resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADING;
    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_FAILED:
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            break;
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

        case SKR_LOADING_PHASE_CANCLE_WAITFOR_IO: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_IO;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADING;
        }
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
        case SKR_LOADING_PHASE_IO:
        case SKR_LOADING_PHASE_LOAD_RESOURCE: {
            if(data)
                sakura_free(data);
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_IO:
        {
            currentPhase = SKR_LOADING_PHASE_CANCLE_WAITFOR_IO;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADING;
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
    if (resourceUrl.empty() || vfs == nullptr)
    {
        SKR_LOG_FMT_ERROR("Resource {} failed to load, file not found.", resourceRecord->header.guid);
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
    }
    else
    {
        currentPhase = SKR_LOADING_PHASE_IO;
        factory = system->FindFactory(resourceRecord->header.type);
        if (factory == nullptr)
        {
            SKR_LOG_FMT_ERROR("Resource {} failed to load, factory of type {} not found.", 
                resourceRecord->header.guid, resourceRecord->header.type);
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
        }
    }
}

void SResourceRequest::_LoadFinished()
{
    resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADED;
    if(data)
        sakura_free(data);
    data = nullptr;
    if (!requestInstall) // only require data, we are done
    {
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        return;
    }
    // schedule loading for all runtime dependencies
    const auto& dependencies = resourceRecord->header.dependencies;
    if (!dependencies.empty())
    {
        for (auto& dep : resourceRecord->header.dependencies)
            system->LoadResource(dep, requestInstall, resourceRecord->id, SKR_REQUESTER_DEPENDENCY);
        currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES;
    }
    else
    {
        currentPhase = SKR_LOADING_PHASE_INSTALL_RESOURCE;
    }
}

void SResourceRequest::_InstallFinished()
{
    resourceRecord->loadingStatus = SKR_LOADING_STATUS_INSTALLED;
    currentPhase = SKR_LOADING_PHASE_FINISHED;
    return;
}

void SResourceRequest::Update()
{
    if (requireLoading != isLoading)
    {
        if (requireLoading)
            UpdateLoad(requestInstall);
        else
            UpdateUnload();
    }
    auto resourceRegistry = system->GetRegistry();
    auto ioService = system->GetRAMService();
    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_REQUEST_RESOURCE: {
            auto fopened = resourceRegistry->RequestResourceFile(this);
            if (fopened)
                currentPhase = SKR_LOADING_PHASE_IO;
            else
            {
                currentPhase = SKR_LOADING_PHASE_FAILED;
                // TODO: Do something with this rude code
                resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST:
            break;
        case SKR_LOADING_PHASE_IO:
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_LOADING;
            if(factory->AsyncIO())
            {
                skr_ram_io_t ramIO = {};
                //ramIO.bytes = nullptr;
                ramIO.offset = 0;
                //ramIO.size = 0;
                ramIO.path = resourceUrl.c_str();
                ioService->request(vfs, &ramIO, &ioRequest, &ioDestination);
                currentPhase = SKR_LOADING_PHASE_WAITFOR_IO;
            }
            else 
            {
                auto file = skr_vfs_fopen(vfs, resourceUrl.c_str(), SKR_FM_READ, SKR_FILE_CREATION_OPEN_EXISTING);
                SKR_DEFER({ skr_vfs_fclose(file); });
                auto size = skr_vfs_fsize(file);
                eastl::vector<uint8_t> buffer(size);
                skr_vfs_fread(file, buffer.data(), 0, size);
                data = buffer.data();
                size = buffer.size();
                buffer.reset_lose_memory();
                currentPhase = SKR_LOADING_PHASE_LOAD_RESOURCE;
            }
            break;
        case SKR_LOADING_PHASE_WAITFOR_IO:
            if(ioRequest.is_ready())
            {
                data = ioDestination.bytes;
                size = ioDestination.size;
                currentPhase = SKR_LOADING_PHASE_LOAD_RESOURCE;
            }
            break;
        case SKR_LOADING_PHASE_LOAD_RESOURCE: {
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
        case SKR_LOADING_PHASE_CANCLE_WAITFOR_IO:
        {
            if(!ioRequest.is_ready())
            {
                ioService->defer_cancel(&ioRequest);
            }
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
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
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE:
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_UNLOAD_RESOURCE: {
            if(data)
                sakura_free(data);
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
            resourceRegistry->CancelRequestFile(this);
            resourceRecord->loadingStatus = SKR_LOADING_STATUS_UNLOADED;
            currentPhase = SKR_LOADING_PHASE_FINISHED;
        }
        break;
        default:
            SKR_UNREACHABLE_CODE();
            break;
    }
}

bool SResourceRequest::Okay()
{
    return (currentPhase == SKR_LOADING_PHASE_FINISHED) && (isLoading == requireLoading);
}

bool SResourceRequest::Failed()
{
    return (currentPhase == SKR_LOADING_PHASE_FAILED);
}

bool SResourceRequest::Yielded()
{
    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST:
        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE:
        case SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_WAITFOR_IO:
        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE:
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