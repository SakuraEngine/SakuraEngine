#include "resource_request_impl.hpp"
#include "platform/debug.h"
#include "utils/defer.hpp"
#include "utils/io.h"
#include "utils/log.hpp"
#include "platform/vfs.h"
#include "resource/resource_factory.h"
#include "binary/reader.h"

namespace skr
{
namespace resource
{
// resource request implementation
skr_guid_t SResourceRequestImpl::GetGuid() const
{
    return resourceRecord->header.guid;
}

skr::span<const uint8_t> SResourceRequestImpl::GetData() const
{
    return skr::span<const uint8_t>(data, size);
}

#ifdef SKR_RESOURCE_DEV_MODE
skr::span<const uint8_t> SResourceRequestImpl::GetArtifactsData() const
{
    return skr::span<const uint8_t>(artifactsData, artifactsSize);
}
#endif

skr::span<const skr_guid_t> SResourceRequestImpl::GetDependencies() const
{
    return skr::span<const skr_guid_t>(dependencies.data(), dependencies.size());
}

void SResourceRequestImpl::UpdateLoad(bool requestInstall)
{
    if (isLoading)
        return;
    isLoading = true;
    resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADING);
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
            resourceRecord->SetStatus(SKR_LOADING_STATUS_INSTALLED);
        }
        break;

        case SKR_LOADING_PHASE_CANCLE_WAITFOR_IO: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_IO;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADING);
        }
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADING);
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_INSTALLING);
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
                resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADED);
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

void SResourceRequestImpl::UpdateUnload()
{
    if (!isLoading)
        return;
    isLoading = false;

    resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADING);

    switch (currentPhase)
    {
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES;
        }
        break;
        case SKR_LOADING_PHASE_IO:
        case SKR_LOADING_PHASE_DESER_RESOURCE: {
            if (data)
                sakura_free(data);
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADED);
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_IO: {
            currentPhase = SKR_LOADING_PHASE_CANCLE_WAITFOR_IO;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADING);
        }
        break;

        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADING);
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
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNINSTALLING);
        }
        break;

        case SKR_LOADING_PHASE_WAITFOR_INSTALL_RESOURCE: {
            currentPhase = SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNINSTALLING);
        }

        default:
            SKR_HALT();
            break;
    }
}

void SResourceRequestImpl::OnRequestFileFinished()
{
    if (resourceUrl.empty() || vfs == nullptr)
    {
        SKR_LOG_FMT_ERROR("Resource {} failed to load, file not found.", resourceRecord->header.guid);
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
        return;
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
            resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
            return;
        }
    }
    // schedule loading for all runtime dependencies
    if(requestInstall)
    {
        _LoadDependencies();
    }
}

void SResourceRequestImpl::OnRequestLoadFinished()
{
    
}

void SResourceRequestImpl::_LoadDependencies()
{
    if(dependenciesLoaded)
        return;
    dependenciesLoaded = true;
    auto& dependencies = resourceRecord->header.dependencies;
    for (auto& dep : dependencies)
        dep.resolve(true, resourceRecord->id, SKR_REQUESTER_DEPENDENCY);
}

void SResourceRequestImpl::_UnloadDependencies()
{
    if(!dependenciesLoaded)
        return;
    dependenciesLoaded = false;
    auto& dependencies = resourceRecord->header.dependencies;
    for(auto& dep : dependencies)
        dep.unload();
}

void SResourceRequestImpl::_LoadFinished()
{
    resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADED);
    if (data)
        sakura_free(data);
    data = nullptr;
    auto& dependencies = resourceRecord->header.dependencies;
    if (!requestInstall) // only require data, we are done
    {
        _UnloadDependencies();
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        return;
    }
    if (!dependencies.empty())
    {
        _LoadDependencies();
        currentPhase = SKR_LOADING_PHASE_WAITFOR_LOAD_DEPENDENCIES;
    }
    else
        currentPhase = SKR_LOADING_PHASE_INSTALL_RESOURCE;
}

void SResourceRequestImpl::_InstallFinished()
{
    resourceRecord->SetStatus(SKR_LOADING_STATUS_INSTALLED);
    currentPhase = SKR_LOADING_PHASE_FINISHED;
    return;
}

void SResourceRequestImpl::_UnloadResource()
{

}

void SResourceRequestImpl::Update()
{
    SMutexLock lock(updateMutex.mMutex);
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
                resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_RESOURCE_REQUEST:
            break;
        case SKR_LOADING_PHASE_IO:
            resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADING);
            if (factory->AsyncIO())
            {
                skr_ram_io_t ramIO = {};
                ramIO.offset = 0;
                ramIO.path = resourceUrl.c_str();
                ioService->request(vfs, &ramIO, &ioRequest, &ioDestination);
#ifdef SKR_RESOURCE_DEV_MODE
                if (!artifactsUrl.empty())
                {
                    ramIO.offset = 0;
                    ramIO.path = artifactsUrl.c_str();
                    ioService->request(vfs, &ramIO, &artifactsIoRequest, &artifactsIoDestination);
                }
#endif
                currentPhase = SKR_LOADING_PHASE_WAITFOR_IO;
            }
            else
            {
                {
                    auto file = skr_vfs_fopen(vfs, resourceUrl.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
                    SKR_DEFER({ skr_vfs_fclose(file); });
                    auto fsize = skr_vfs_fsize(file);
                    eastl::vector<uint8_t> buffer(fsize);
                    skr_vfs_fread(file, buffer.data(), 0, fsize);
                    data = buffer.data();
                    size = buffer.size();
                    buffer.reset_lose_memory();
                }
#ifdef SKR_RESOURCE_DEV_MODE
                if (!artifactsUrl.empty())
                {
                    auto file = skr_vfs_fopen(vfs, artifactsUrl.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
                    SKR_DEFER({ skr_vfs_fclose(file); });
                    auto fsize = skr_vfs_fsize(file);
                    eastl::vector<uint8_t> buffer(fsize);
                    skr_vfs_fread(file, buffer.data(), 0, fsize);
                    artifactsData = buffer.data();
                    size = buffer.size();
                    buffer.reset_lose_memory();
                }
#endif
                currentPhase = SKR_LOADING_PHASE_DESER_RESOURCE;
            }
            break;
        case SKR_LOADING_PHASE_WAITFOR_IO:
            if (ioRequest.is_ready())
            {
                data = ioDestination.bytes;
                size = ioDestination.size;
            }
#ifdef SKR_RESOURCE_DEV_MODE
            if (!artifactsUrl.empty())
            {
                if (artifactsIoRequest.is_ready())
                {
                    artifactsData = artifactsIoDestination.bytes;
                    artifactsSize = artifactsIoDestination.size;
                }
            }
            if (data && (artifactsUrl.empty() || artifactsData))
                currentPhase = SKR_LOADING_PHASE_DESER_RESOURCE;
#else
            if (data)
                currentPhase = SKR_LOADING_PHASE_DESER_RESOURCE;
#endif
            break;
        case SKR_LOADING_PHASE_DESER_RESOURCE: {
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
                if (serdeResult != 0)
                {
                    SKR_LOG_FMT_ERROR("Resource {} failed to load, serde failed with error code {}.",
                    resourceRecord->header.guid, serdeResult);
                    currentPhase = SKR_LOADING_PHASE_FINISHED;
                    resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
                }
                else
                    _LoadFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE: {
            if (serdeEvent.test())
            {
                serdeEvent.clear();
                if (serdeResult != 0)
                {
                    SKR_LOG_FMT_ERROR("Resource {} failed to load, serde failed with error code {}.",
                    resourceRecord->header.guid, serdeResult);
                    currentPhase = SKR_LOADING_PHASE_FINISHED;
                    resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
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
                _UnloadDependencies();
                resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
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
            resourceRecord->SetStatus(SKR_LOADING_STATUS_INSTALLING);
            auto status = factory->Install(resourceRecord);
            if (status == SKR_INSTALL_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR("Resource {} failed to install.", resourceRecord->header.guid);
                currentPhase = SKR_LOADING_PHASE_FINISHED;
                resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
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
                resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
            }
            else if (status == SKR_INSTALL_STATUS_SUCCEED)
            {
                _InstallFinished();
            }
        }
        break;
        case SKR_LOADING_PHASE_FINISHED: {
            // special case when we are installing a resource that is already loaded
            SKR_ASSERT(isLoading && requestInstall > !(resourceRecord->loadingStatus == SKR_LOADING_STATUS_LOADED));
            _LoadFinished();
        }
        break;
        case SKR_LOADING_PHASE_CANCLE_WAITFOR_IO: {
            if (!ioRequest.is_ready())
            {
                ioService->defer_cancel(&ioRequest);
            }
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADED);
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_INSTALL_RESOURCE:
        case SKR_LOADING_PHASE_UNINSTALL_RESOURCE: {
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNINSTALLING);
            factory->Uninstall(resourceRecord);
            resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADED);
            currentPhase = SKR_LOADING_PHASE_UNLOAD_RESOURCE;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_RESOURCE:
        case SKR_LOADING_PHASE_CANCEL_WAITFOR_LOAD_DEPENDENCIES:
        case SKR_LOADING_PHASE_UNLOAD_RESOURCE: {
            if (data)
                sakura_free(data);
            _UnloadDependencies();
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADING);
            factory->Unload(resourceRecord);
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADED);
            currentPhase = SKR_LOADING_PHASE_FINISHED;
        }
        break;
        case SKR_LOADING_PHASE_CANCEL_RESOURCE_REQUEST: {
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADING);
            resourceRegistry->CancelRequestFile(this);
            resourceRecord->SetStatus(SKR_LOADING_STATUS_UNLOADED);
            currentPhase = SKR_LOADING_PHASE_FINISHED;
        }
        break;
        default:
            SKR_UNREACHABLE_CODE();
            break;
    }
}

void SResourceRequestImpl::LoadTask()
{
    skr::binary::SpanReader reader{ GetData() };
    skr_binary_reader_t archive{ reader };
#ifdef SKR_RESOURCE_DEV_MODE
    skr::binary::SpanReader artifacstReader = { GetArtifactsData() };
    skr_binary_reader_t artifactsArchive{ artifacstReader };
#endif
    serdeResult = factory->Deserialize(resourceRecord, &archive);
    if (serdeResult == 0)
        factory->DerserializeArtifacts(resourceRecord, &artifactsArchive);
    serdeEvent.signal();
}

bool SResourceRequestImpl::Okay()
{
    bool installed = resourceRecord && !(resourceRecord->loadingStatus == SKR_LOADING_STATUS_LOADED);
    return (currentPhase == SKR_LOADING_PHASE_FINISHED) && (isLoading == requireLoading) && (requestInstall <= installed);
}

bool SResourceRequestImpl::Failed()
{
    return !resourceRecord || (resourceRecord->loadingStatus == SKR_LOADING_STATUS_ERROR);
}

bool SResourceRequestImpl::AsyncSerde()
{
    return currentPhase == SKR_LOADING_PHASE_WAITFOR_LOAD_RESOURCE && !serdeScheduled;
}

bool SResourceRequestImpl::Yielded()
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

void SResourceRegistry::FillRequest(SResourceRequest* r, skr_resource_header_t header, skr_vfs_t* vfs, const char* uri)
{
    auto request = static_cast<SResourceRequestImpl*>(r);
    if (request)
    {
        request->resourceRecord->header.type = header.type;
        request->resourceRecord->header.version = header.version;
        request->resourceRecord->header.dependencies = header.dependencies;
        request->vfs = vfs;
        request->resourceUrl = uri;
    }
}

} // namespace resource
} // namespace skr