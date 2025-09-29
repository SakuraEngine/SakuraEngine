#include "SkrCore/serialize/binary_archive.hpp"
#include "resource_request_impl.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/misc/defer.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrCore/log.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/resource/resource_factory.hpp"

namespace skr
{
// resource request implementation
GUID SResourceRequestImpl::GetGuid() const
{
    return resourceRecord->header.guid;
}

skr::Span<const uint8_t> SResourceRequestImpl::GetData() const
{
    if (!dataBlob)
    {
        return {};
    }
    return skr::Span<const uint8_t>(dataBlob->get_data(), dataBlob->get_size());
}

skr::Span<const SResourceHandle> SResourceRequestImpl::GetDependencies() const
{
    auto& dependencies = resourceRecord->header.dependencies;
    return skr::Span<const SResourceHandle>(dependencies.data(), dependencies.size());
}

void SResourceRequestImpl::_UnloadDependencies()
{
    auto& dependencies = resourceRecord->header.dependencies;
    for (auto& dep : dependencies)
        dep.reset();
}

void SResourceRequestImpl::Update()
{
    SMutexLock lock(updateMutex.mMutex);
    auto resourceRegistry = system->GetRegistry();
    auto ioService = system->GetRAMService();
    auto currentStatus = resourceRecord->loadingStatus.load();
    SKR_LOG_BACKTRACE(u8"Current reosurce loading phase: %d!", (int32_t)currentStatus);
    if (currentStatus == EResourceLoadingStatus::Unloaded)
        return;

    switch (currentStatus)
    {
    case EResourceLoadingStatus::Loading: {
        if (dataBlob && factory->AsyncIO())
        {
            if (const bool dataReady = dataFuture.is_ready())
            {
                resourceRecord->SetStatus(EResourceLoadingStatus::Loaded);
            }
        }
        else if (auto fopened = resourceRegistry->RequestResourceFile(this))
        {
            if (factory->AsyncIO())
            {
                auto rq = ioService->open_request();
                rq->set_vfs(vfs);
                rq->set_path(resourceUrl.c_str());
                rq->add_block({}); // read all
                SKR_ASSERT(dataFuture.status == 0);
                dataBlob = ioService->request(rq, &dataFuture);
            }
            else
            {
                auto file = skr_vfs_fopen(vfs, (const char8_t*)resourceUrl.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
                SKR_DEFER({ skr_vfs_fclose(file); });
                auto fsize = skr_vfs_fsize(file);
                dataBlob = skr::IBlob::Create(nullptr, fsize, false);
                skr_vfs_fread(file, dataBlob->get_data(), 0, fsize);
                resourceRecord->SetStatus(EResourceLoadingStatus::Loaded);
            }
        }
        else
        {
            // TODO: Do something with this rude code
            resourceRecord->SetStatus(EResourceLoadingStatus::Error);
        }
    }
    break;
    case EResourceLoadingStatus::Loaded: {
        // send dependencies' requests
        auto& dependencies = resourceRecord->header.dependencies;
        if (!dependencies.is_empty())
        {
            auto& dependencies = resourceRecord->header.dependencies;
            for (auto& dep : dependencies)
            {
                dep.install();
            }
        }
        // no need of deserialization
        if (resourceRecord->resource != nullptr)
        {
            resourceRecord->SetStatus(EResourceLoadingStatus::WaitingDependencies);
        }
        // deserialize
        // TODO: USE ASYNC SERDE
        else if (bool asyncSerde = false && factory->AsyncSerdeLoadFactor() != 0.f)
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
        else
        {
            skr::ArReadBin reader;
            auto buffer = GetData();
            reader.use_buffer(buffer);
            if (factory->Deserialize(resourceRecord, &reader))
            {
                serdeEvent.signal();
                dataBlob.reset();
                if (!requestInstall) // only require data, we are done
                {
                    _UnloadDependencies();
                    return;
                }
                resourceRecord->SetStatus(EResourceLoadingStatus::WaitingDependencies);
            }
            else
            {
                SKR_LOG_FMT_ERROR(u8"Resource {} failed to load, deserialize failed! ", resourceRecord->header.guid);
                resourceRecord->SetStatus(EResourceLoadingStatus::Error);
            }
        }
    }
    break;
    case EResourceLoadingStatus::WaitingDependencies: {
        // wait dependencies
        bool dependencies_ready = true;
        for (auto& dep : resourceRecord->header.dependencies)
        {
            if (dep.get_status() == EResourceLoadingStatus::Error)
            {
                resourceRecord->SetStatus(EResourceLoadingStatus::Error);
                SKR_LOG_FMT_ERROR(u8"Resource {} failed to load dependency resource {}.", resourceRecord->header.guid, dep.get_guid());
                break;
            }
            else if (dep.get_status() != EResourceLoadingStatus::Installed)
            {
                dependencies_ready = false;
                break;
            }
        }
        // start install
        if (dependencies_ready)
        {
            auto installStatus = factory->Install(resourceRecord);
            if (installStatus == SKR_INSTALL_STATUS_FAILED)
            {
                SKR_LOG_FMT_ERROR(u8"Resource {} failed to install.", resourceRecord->header.guid);
                resourceRecord->SetStatus(EResourceLoadingStatus::Error);
            }
            else if (installStatus == SKR_INSTALL_STATUS_INPROGRESS)
            {
                resourceRecord->SetStatus(EResourceLoadingStatus::Installing);
            }
            else if (installStatus == SKR_INSTALL_STATUS_SUCCEED)
            {
                resourceRecord->SetStatus(EResourceLoadingStatus::Installed);
            }
        }
    }
    break;
    case EResourceLoadingStatus::Installing: {
        auto status = factory->UpdateInstall(resourceRecord);
        if (status == SKR_INSTALL_STATUS_FAILED)
        {
            SKR_LOG_FMT_ERROR(u8"Resource {} failed to install.", resourceRecord->header.guid);
            resourceRecord->SetStatus(EResourceLoadingStatus::Error);
        }
        else if (status == SKR_INSTALL_STATUS_SUCCEED)
        {
            resourceRecord->SetStatus(EResourceLoadingStatus::Installed);
        }
    }
    break;
    case EResourceLoadingStatus::Uninstalling: {
        factory->Uninstall(resourceRecord);
        resourceRecord->SetStatus(EResourceLoadingStatus::Unloading);
    }
    break;
    case EResourceLoadingStatus::Unloading: {
        dataBlob.reset();
        _UnloadDependencies();
        factory->Unload(resourceRecord);
        resourceRecord->SetStatus(EResourceLoadingStatus::Unloaded);
    }
    break;
    default:
        SKR_UNREACHABLE_CODE();
        break;
    }
}

bool SResourceRequestImpl::Okay()
{
    if (requestInstall)
        return resourceRecord->loadingStatus == EResourceLoadingStatus::Installed;
    const bool bLoaded = resourceRecord->loadingStatus == EResourceLoadingStatus::Loaded;;
    const bool bUnloaded = resourceRecord->loadingStatus == EResourceLoadingStatus::Unloaded;
    return bLoaded || bUnloaded;
}

bool SResourceRequestImpl::Failed()
{
    return !resourceRecord || (resourceRecord->loadingStatus == EResourceLoadingStatus::Error);
}

void ResourceRegistry::FillRequest(ResourceRequest* r, SResourceHeader header, skr_vfs_t* vfs, const char8_t* uri)
{
    auto request = static_cast<SResourceRequestImpl*>(r);
    if (request)
    {
        request->resourceRecord->header.type = header.type;
        request->resourceRecord->header.version = header.version;
        request->resourceRecord->header.dependencies = header.dependencies;
        request->vfs = vfs;
        request->resourceUrl = uri;
        request->system = request->system;
        request->factory = request->system->FindFactory(request->resourceRecord->header.type);
    }
}

} // namespace skr