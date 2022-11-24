#include "resource/resource_system.h"
#include "platform/debug.h"
#include "utils/defer.hpp"
#include "utils/io.hpp"
#include "utils/log.hpp"
#include "platform/vfs.h"
#include "resource/resource_factory.h"
#include "type/type_registry.h"

namespace skr
{
namespace resource
{

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
                currentPhase = SKR_LOADING_PHASE_FINISHED;
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
            bool asyncSerde = factory->AsyncSerdeLoadFactor() != 0.f;
            
            if (asyncSerde)
            {
                serdeScheduled = false;
                serdeEvent.clear();
                currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE;
            }
            else
            {
                LoadTask();
                if(serdeResult != 0)
                {
                    SKR_LOG_FMT_ERROR("Resource {} failed to load, serde failed with error code {}.", 
                        resourceRecord->header.guid, serdeResult);
                    currentPhase = SKR_LOADING_PHASE_FINISHED;
                    resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
                }
                else
                    _LoadFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE: {
            if(serdeEvent.test())
            {
                serdeEvent.clear();
                if(serdeResult != 0)
                {
                    SKR_LOG_FMT_ERROR("Resource {} failed to load, serde failed with error code {}.", 
                        resourceRecord->header.guid, serdeResult);
                    currentPhase = SKR_LOADING_PHASE_FINISHED;
                    resourceRecord->loadingStatus = SKR_LOADING_STATUS_ERROR;
                }
                else
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
        case SKR_LOADING_PHASE_FINISHED:
        {
            //special case when we are installing a resource that is already loaded
            SKR_ASSERT(isLoading && requestInstall > !(resourceRecord->loadingStatus == SKR_LOADING_STATUS_LOADED));
            _LoadFinished();
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

void SResourceRequest::LoadTask()
{
    if (auto type = skr_get_type(&resourceRecord->header.type))
    {
        auto obj = type->Malloc();
        type->Construct(obj, nullptr, 0);
        struct SpanReader
        {
            gsl::span<const uint8_t> data;
            size_t offset = 0;
            int read(void* dst, size_t size)
            {
                if (offset + size > data.size())
                    return -1;
                memcpy(dst, data.data() + offset, size);
                offset += size;
                return 0;
            }
        } reader = { GetData() };
        skr_binary_reader_t archive{reader};
        serdeResult = type->Deserialize(obj, &archive);
        if(serdeResult != 0)
        {
            type->Destruct(obj);
            type->Free(obj);
            obj = nullptr;
        }
        resourceRecord->resource = obj;
    }
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
}

bool SResourceRequest::Okay()
{
    bool installed = resourceRecord && !(resourceRecord->loadingStatus == SKR_LOADING_STATUS_LOADED);
    return (currentPhase == SKR_LOADING_PHASE_FINISHED) && (isLoading == requireLoading) && (requestInstall <= installed);
}

bool SResourceRequest::Failed()
{
    return !resourceRecord || (resourceRecord->loadingStatus == SKR_LOADING_STATUS_ERROR);
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
}
}