#include "resource_request_impl.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/misc/defer.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrCore/log.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrRT/resource/resource_factory.h"

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
    if (!dataBlob)
    {
        return {};
    }
    return skr::span<const uint8_t>(dataBlob->get_data(), dataBlob->get_size());
}

#ifdef SKR_RESOURCE_DEV_MODE
skr::span<const uint8_t> SResourceRequestImpl::GetArtifactsData() const
{
    if (!artifactsBlob)
    {
        return {};
    }
    return skr::span<const uint8_t>(artifactsBlob->get_data(), artifactsBlob->get_size());
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
            dataBlob.reset();
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
    if (resourceUrl.is_empty() || vfs == nullptr)
    {
        SKR_LOG_FMT_ERROR(u8"Resource {} failed to load, file not found.", resourceRecord->header.guid);
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
        return;
    }
    else
    {
        currentPhase = SKR_LOADING_PHASE_IO;
        factory      = system->FindFactory(resourceRecord->header.type);
        if (factory == nullptr)
        {
            SKR_LOG_FMT_ERROR(u8"Resource {} failed to load, factory of type {} not found.",
                              resourceRecord->header.guid, resourceRecord->header.type);
            currentPhase = SKR_LOADING_PHASE_FINISHED;
            resourceRecord->SetStatus(SKR_LOADING_STATUS_ERROR);
            return;
        }
    }
    // schedule loading for all runtime dependencies
    if (requestInstall)
    {
        _LoadDependencies();
    }
}

void SResourceRequestImpl::OnRequestLoadFinished()
{
}

void SResourceRequestImpl::_LoadDependencies()
{
    if (dependenciesLoaded)
        return;
    dependenciesLoaded = true;
    auto& dependencies = resourceRecord->header.dependencies;
    for (auto& dep : dependencies)
        dep.resolve(true, resourceRecord->id, SKR_REQUESTER_DEPENDENCY);
}

void SResourceRequestImpl::_UnloadDependencies()
{
    if (!dependenciesLoaded)
        return;
    dependenciesLoaded = false;
    auto& dependencies = resourceRecord->header.dependencies;
    for (auto& dep : dependencies)
        dep.unload();
}

void SResourceRequestImpl::_LoadFinished()
{
    resourceRecord->SetStatus(SKR_LOADING_STATUS_LOADED);
    dataBlob.reset();
    auto& dependencies = resourceRecord->header.dependencies;
    if (!requestInstall) // only require data, we are done
    {
        _UnloadDependencies();
        currentPhase = SKR_LOADING_PHASE_FINISHED;
        return;
    }
    if (!dependencies.is_empty())
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
    auto ioService        = system->GetRAMService();
    SKR_LOG_BACKTRACE(u8"Current reosurce loading phase: %d!", (int32_t)currentPhase);
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
                {
                    auto rq = ioService->open_request();
                    rq->set_vfs(vfs);
                    rq->set_path(resourceUrl.c_str());
                    rq->add_block({}); // read all
                    SKR_ASSERT(dataFuture.status == 0);
                    dataBlob = ioService->request(rq, &dataFuture);
                }
#ifdef SKR_RESOURCE_DEV_MODE
                if (!artifactsUrl.is_empty())
                {
                    auto rq = ioService->open_request();
                    rq->set_vfs(vfs);
                    rq->set_path(artifactsUrl.c_str());
                    rq->add_block({}); // read all
                    SKR_ASSERT(artifactsFuture.status == 0);
                    artifactsBlob = ioService->request(rq, &artifactsFuture);
                }
#endif
                currentPhase = SKR_LOADING_PHASE_WAITFOR_IO;
            }
            else
            {
                {
                    auto file = skr_vfs_fopen(vfs, (const char8_t*)resourceUrl.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
                    SKR_DEFER({ skr_vfs_fclose(file); });
                    auto fsize = skr_vfs_fsize(file);
                    dataBlob   = skr::IBlob::Create(nullptr, fsize, false);
                    skr_vfs_fread(file, dataBlob->get_data(), 0, fsize);
                }
#ifdef SKR_RESOURCE_DEV_MODE
                if (!artifactsUrl.is_empty())
                {
                    auto file = skr_vfs_fopen(vfs, (const char8_t*)artifactsUrl.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
                    SKR_DEFER({ skr_vfs_fclose(file); });
                    auto fsize    = skr_vfs_fsize(file);
                    artifactsBlob = skr::IBlob::Create(nullptr, fsize, false);
                    skr_vfs_fread(file, artifactsBlob->get_data(), 0, fsize);
                }
#endif
                currentPhase = SKR_LOADING_PHASE_DESER_RESOURCE;
            }
            break;
        case SKR_LOADING_PHASE_WAITFOR_IO: {
            if (factory->AsyncIO())
            {
                const bool dataReady = dataFuture.is_ready();
#ifdef SKR_RESOURCE_DEV_MODE
                const bool artifactsReady = artifactsUrl.is_empty() || artifactsFuture.is_ready();
                if (dataReady && artifactsReady)
#else
                if (dataReady)
#endif
                {
                    currentPhase = SKR_LOADING_PHASE_DESER_RESOURCE;
                }
            }
            else
            {
                currentPhase = SKR_LOADING_PHASE_DESER_RESOURCE;
            }
        }
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
                    SKR_LOG_FMT_ERROR(u8"Resource {} failed to load, serde failed with error code {}.",
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
                    SKR_LOG_FMT_ERROR(u8"Resource {} failed to load, serde failed with error code {}.",
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
                    SKR_LOG_FMT_ERROR(u8"Resource {} failed to load dependency resource {}.", resourceRecord->header.guid, dep.get_serialized());
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
                SKR_LOG_FMT_ERROR(u8"Resource {} failed to install.", resourceRecord->header.guid);
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
                SKR_LOG_FMT_ERROR(u8"Resource {} failed to install.", resourceRecord->header.guid);
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
            SKR_ASSERT(isLoading && requestInstall >= (resourceRecord->loadingStatus != SKR_LOADING_STATUS_LOADED));
            _LoadFinished();
        }
        break;
        case SKR_LOADING_PHASE_CANCLE_WAITFOR_IO: {
            if (!dataFuture.is_ready())
            {
                // request cancle
                if (!skr_atomic_load_acquire(&dataFuture.request_cancel))
                {
                    ioService->cancel(&dataFuture);
                }
                else if (!dataFuture.is_cancelled())
                {
                    break; // continue to wait for cancel
                }
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
            dataBlob.reset();
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
    auto                        data = GetData();
    skr::archive::BinSpanReader reader{ data };
    SBinaryReader               archive{ reader };
#ifdef SKR_RESOURCE_DEV_MODE
    auto                        artifactsData   = GetArtifactsData();
    skr::archive::BinSpanReader artifacstReader = { artifactsData };
    SBinaryReader               artifactsArchive{ artifacstReader };
#endif
    if (factory->Deserialize(resourceRecord, &archive))
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

void SResourceRegistry::FillRequest(SResourceRequest* r, skr_resource_header_t header, skr_vfs_t* vfs, const char8_t* uri)
{
    auto request = static_cast<SResourceRequestImpl*>(r);
    if (request)
    {
        request->resourceRecord->header.type         = header.type;
        request->resourceRecord->header.version      = header.version;
        request->resourceRecord->header.dependencies = header.dependencies;
        request->vfs                                 = vfs;
        request->resourceUrl                         = uri;
    }
}

} // namespace resource
} // namespace skr