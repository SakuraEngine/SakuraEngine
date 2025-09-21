#include "SkrProfile/profile.h"
#include "SkrRTTR/type.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"

namespace skd::asset
{
struct CookContextImpl : public CookContext
{
    Importer* GetImporter() const override;
    skr::GUID GetImporterType() const override;
    uint32_t GetImporterVersion() const override;
    uint32_t GetCookerVersion() const override;

    // TODO: REMOVE THIS
    skr::String GetAssetPath() const override;

    URI AddSourceFile(const URI& path) override;
    URI AddSourceFileAndLoad(skr::io::IRAMService* ioService, const URI& path, skr::BlobId& destination) override;
    skr::Span<const URI> GetSourceFiles() const override;

    void AddRuntimeDependency(skr::GUID resource) override;
    void AddSoftRuntimeDependency(skr::GUID resource) override;
    uint32_t AddStaticDependency(skr::GUID resource, bool install) override;
    skr::Span<const skr::GUID> GetRuntimeDependencies() const override;
    skr::Span<const SResourceHandle> GetStaticDependencies() const override;
    const SResourceHandle& GetStaticDependency(uint32_t index) const override;

    const skr::task::event_t& GetCounter() override
    {
        return counter;
    }

    skr::RC<AssetMetaFile> GetAssetMetaFile() override
    {
        return metafile;
    }

    skr::RC<const AssetMetaFile> GetAssetMetaFile() const override
    {
        return metafile.cast_const<const AssetMetaFile>();
    }

    void SetCounter(skr::task::event_t& ct) override
    {
        counter = ct;
    }

    void SetIOService(skr::io::IRAMService* service) override
    {
        ioService = service;
    }

    void SetCookerVersion(uint32_t version) override
    {
        cookerVersion = version;
    }

    void* _Import() override;
    void _Destroy(void*) override;

    skr::RC<AssetMetaFile> metafile = nullptr;
    uint32_t importerVersion = 0;
    uint32_t cookerVersion = 0;

    skr::io::IRAMService* ioService = nullptr;

    // Job system wait counter
    skr::task::event_t counter;

    skr::Vector<SResourceHandle> staticDependencies;
    skr::Vector<skr::GUID> runtimeDependencies;
    skr::Vector<URI> fileDependencies;

    CookContextImpl(skr::RC<AssetMetaFile> metafile)
        : metafile(metafile)
    {
        
    }
};

CookContext* CookContext::Create(skr::RC<AssetMetaFile> metafile)
{
    return SkrNew<CookContextImpl>(metafile);
}

void CookContext::Destroy(CookContext* ctx)
{
    SkrDelete(ctx);
}

void CookContextImpl::_Destroy(void* resource)
{
    if (!metafile->GetImporter())
    {
        SKR_LOG_ERROR(u8"[CookContext::Cook] importer failed to load, asset path path: %s", metafile->GetURI().string().c_str());
    }
    //-----import raw data
    metafile->GetImporter()->Destroy(resource);
    SKR_LOG_INFO(u8"[CookContext::Cook] asset freed for asset: %s", metafile->GetURI().string().c_str());
}

void* CookContextImpl::_Import()
{
    SkrZoneScoped;
    //-----load importer
    if (auto importer = metafile->GetImporter())
    {
        auto rawData = importer->Import(ioService, this);
        SKR_LOG_INFO(u8"[CookContext::Cook] asset imported for asset: %s", metafile->GetURI().string().c_str());
        return rawData;
    }
    return nullptr;
}


Importer* CookContextImpl::GetImporter() const
{
    return metafile->GetImporter().get();
}

skr::GUID CookContextImpl::GetImporterType() const
{
    return metafile->GetImporter()->GetType();
}

uint32_t CookContextImpl::GetImporterVersion() const
{
    return importerVersion;
}

uint32_t CookContextImpl::GetCookerVersion() const
{
    return cookerVersion;
}

skr::String CookContextImpl::GetAssetPath() const
{
    return metafile->GetURI().string();
}

URI CookContextImpl::AddSourceFile(const URI& inPath)
{
    auto iter = std::find_if(fileDependencies.begin(), fileDependencies.end(), [&](const auto& dep) { return dep == inPath; });
    if (iter == fileDependencies.end())
        fileDependencies.add(inPath);
    
    // Construct the full path by combining the asset's parent directory with the input path
    skr::Path uri_path{metafile->GetURI()};
    auto parent_dir = uri_path.parent_directory();
    auto full_path = parent_dir / inPath;
    
    return full_path.string();
}

URI CookContextImpl::AddSourceFileAndLoad(skr::io::IRAMService* ioService, const URI& path, skr::BlobId& destination)
{
    auto outPath = AddSourceFile(path.string());
    const auto assetMetaFile = GetAssetMetaFile();
    // load file
    skr::task::event_t counter;
    auto rq = ioService->open_request();
    rq->set_vfs(assetMetaFile->GetProject()->GetAssetVFS());
    rq->set_path(outPath.string().c_str());
    rq->add_block({}); // read all
    rq->add_callback(SKR_IO_STAGE_COMPLETED, +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
        SkrZoneScopedN("SignalCounter");
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal(); }, &counter);
    skr_io_future_t future = {};
    destination = ioService->request(rq, &future);
    counter.wait(false);
    return outPath;
}

skr::Span<const URI> CookContextImpl::GetSourceFiles() const
{
    return fileDependencies;
}

void CookContextImpl::AddRuntimeDependency(skr::GUID resource)
{
    auto iter = std::find_if(runtimeDependencies.begin(), runtimeDependencies.end(), [&](const auto& dep) { return dep == resource; });
    if (iter == runtimeDependencies.end())
        runtimeDependencies.add(resource);
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}

void CookContextImpl::AddSoftRuntimeDependency(skr::GUID resource)
{
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}

skr::Span<const skr::GUID> CookContextImpl::GetRuntimeDependencies() const
{
    return skr::Span<const skr::GUID>(runtimeDependencies.data(), runtimeDependencies.size());
}

skr::Span<const SResourceHandle> CookContextImpl::GetStaticDependencies() const
{
    return skr::Span<const SResourceHandle>(staticDependencies.data(), staticDependencies.size());
}

const SResourceHandle& CookContextImpl::GetStaticDependency(uint32_t index) const
{
    return staticDependencies[index];
}

uint32_t CookContextImpl::AddStaticDependency(skr::GUID resource, bool install)
{
    auto iter = std::find_if(staticDependencies.begin(), staticDependencies.end(), [&](const auto& dep) { return dep.get_serialized() == resource; });
    if (iter == staticDependencies.end())
    {
        auto counter = GetCookSystem()->EnsureCooked(resource);
        if (counter) counter.wait(false);
        SResourceHandle handle{ resource };
        handle.resolve(install, (uint64_t)this, SKR_REQUESTER_SYSTEM);
        if (!handle.get_resolved())
        {
            auto record = handle.get_record();
            task::event_t event;
            auto callback = [&]() {
                event.signal();
            };
            record->AddCallback(SKR_LOADING_STATUS_ERROR, callback);
            record->AddCallback(install ? SKR_LOADING_STATUS_INSTALLED : SKR_LOADING_STATUS_LOADED, callback);
            if (!handle.get_resolved())
            {
                event.wait(false);
            }
        }
        SKR_ASSERT(handle.is_resolved());
        staticDependencies.add(std::move(handle));
        return (uint32_t)(staticDependencies.size() - 1);
    }
    return (uint32_t)(staticDependencies.end() - iter);
}
} // namespace skd::asset