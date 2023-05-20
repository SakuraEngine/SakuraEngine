#include "async/fib_task.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "misc/io.h"
#include "serde/json/reader.h"

namespace skd::asset
{
struct SCookContextImpl : public SCookContext
{
    skr::filesystem::path GetOutputPath() const override;
    
    SImporter* GetImporter() const override;
    skr_guid_t GetImporterType() const override;
    uint32_t GetImporterVersion() const override;
    uint32_t GetCookerVersion() const override;
    const SAssetRecord* GetAssetRecord() const override;
    skr::string GetAssetPath() const override;

    skr::filesystem::path AddFileDependency(const skr::filesystem::path& path) override;
    skr::filesystem::path AddFileDependencyAndLoad(skr_io_ram_service_t* ioService, const skr::filesystem::path& path, skr_async_ram_destination_t& destination) override;

    void AddRuntimeDependency(skr_guid_t resource) override;
    void AddSoftRuntimeDependency(skr_guid_t resource) override;
    uint32_t AddStaticDependency(skr_guid_t resource, bool install) override;
    skr::span<const skr_guid_t> GetRuntimeDependencies() const override;
    skr::span<const skr_resource_handle_t> GetStaticDependencies() const override;
    skr::span<const skr::filesystem::path> GetFileDependencies() const override;
    const skr_resource_handle_t& GetStaticDependency(uint32_t index) const override;

    const skr::task::event_t& GetCounter() override
    {
        return counter;
    }
    
    void SetCounter(skr::task::event_t& ct) override
    {
        counter = ct;
    }

    void SetCookerVersion(uint32_t version) override
    {
        cookerVersion = version;
    }

    void SetOutputPath(const skr::filesystem::path& path) override
    {
        outputPath = path;
    }

    void* _Import() override;
    void _Destroy(void*) override;

    skr_guid_t importerType;
    uint32_t importerVersion = 0;
    uint32_t cookerVersion = 0;

    SImporter* importer = nullptr;
    skr_io_ram_service_t* ioService = nullptr;

    // Job system wait counter
    skr::task::event_t counter;

    skr::filesystem::path outputPath;
    skr::vector<skr_resource_handle_t> staticDependencies;
    skr::vector<skr_guid_t> runtimeDependencies;
    skr::vector<skr::filesystem::path> fileDependencies;

    SCookContextImpl(skr_io_ram_service_t* ioService)
        : ioService(ioService)
    {

    }
};

SCookContext* SCookContext::Create(skr_io_ram_service_t* service)
{
    return SkrNew<SCookContextImpl>(service);
}

void SCookContext::Destroy(SCookContext* ctx)
{
    SkrDelete(ctx);
}

void SCookContextImpl::_Destroy(void* resource)
{
    if(!importer)
    {
        SKR_LOG_ERROR("[SCookContext::Cook] importer failed to load, asset path path: %s", record->path.u8string().c_str());
    }
    SKR_DEFER({ SkrDelete(importer); });
    //-----import raw data
    importer->Destroy(resource);
    SKR_LOG_INFO("[SCookContext::Cook] asset freed for asset: %s", record->path.u8string().c_str());
}

void* SCookContextImpl::_Import()
{
    //-----load importer
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    auto importerJson = doc["importer"]; // import from asset
    if (importerJson.error() == simdjson::SUCCESS)
    {
        skr_guid_t importerTypeGuid = {};
        importer = GetImporterRegistry()->LoadImporter(record, std::move(importerJson).value_unsafe(), &importerTypeGuid);
        if(!importer)
        {
            SKR_LOG_ERROR("[SCookContext::Cook] importer failed to load, asset: %s", record->path.u8string().c_str());
            return nullptr;
        }
        importerVersion = importer->Version();
        importerType = importerTypeGuid;
        //-----import raw data
        auto rawData = importer->Import(ioService, this);
        SKR_LOG_INFO("[SCookContext::Cook] asset imported for asset: %s", record->path.u8string().c_str());
        return rawData;
    }
    // auto parentJson = doc["parent"]; // derived from resource
    // if (parentJson.error() == simdjson::SUCCESS)
    // {
    //     skr_guid_t parentGuid;
    //     skr::json::Read(std::move(parentJson).value_unsafe(), parentGuid);
    //     return AddStaticDependency(parentGuid);
    // }
    return nullptr;
}

skr::filesystem::path SCookContextImpl::GetOutputPath() const
{
    return outputPath;
}

SImporter* SCookContextImpl::GetImporter() const
{
    return importer;
}

skr_guid_t SCookContextImpl::GetImporterType() const
{
    return importerType;
}

uint32_t SCookContextImpl::GetImporterVersion() const
{
    return importerVersion;
}

uint32_t SCookContextImpl::GetCookerVersion() const
{
    return cookerVersion;
}

const SAssetRecord* SCookContextImpl::GetAssetRecord() const
{
    return record;
}

skr::string SCookContextImpl::GetAssetPath() const
{
    return record->path.u8string().c_str();
}

skr::filesystem::path SCookContextImpl::AddFileDependency(const skr::filesystem::path &inPath)
{
    auto iter = std::find_if(fileDependencies.begin(), fileDependencies.end(), [&](const auto &dep) { return dep == inPath; });
    if (iter == fileDependencies.end())
        fileDependencies.push_back(inPath);
    return record->path.parent_path() / inPath;
}

skr::filesystem::path SCookContextImpl::AddFileDependencyAndLoad(skr_io_ram_service_t* ioService, const skr::filesystem::path& path,
    skr_async_ram_destination_t& destination)
{
    auto outPath = AddFileDependency(path.c_str());
    auto u8Path = outPath.u8string();
    const auto assetRecord = GetAssetRecord();
    // load file
    skr::task::event_t counter;
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
        auto pCounter = (skr::task::event_t*)data;
        pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_request_t ioRequest = {};
    ioService->request(assetRecord->project->asset_vfs, &ramIO, &ioRequest, &destination);
    counter.wait(false);
    return outPath;
}

void SCookContextImpl::AddRuntimeDependency(skr_guid_t resource)
{
    auto iter = std::find_if(runtimeDependencies.begin(), runtimeDependencies.end(), [&](const auto &dep) { return dep == resource; });
    if (iter == runtimeDependencies.end())
        runtimeDependencies.push_back(resource);
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}

void SCookContextImpl::AddSoftRuntimeDependency(skr_guid_t resource)
{
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}

skr::span<const skr_guid_t> SCookContextImpl::GetRuntimeDependencies() const
{
    return skr::span<const skr_guid_t>(runtimeDependencies.data(), runtimeDependencies.size());
}

skr::span<const skr::filesystem::path> SCookContextImpl::GetFileDependencies() const
{
    return fileDependencies;
}

skr::span<const skr_resource_handle_t> SCookContextImpl::GetStaticDependencies() const
{
    return skr::span<const skr_resource_handle_t>(staticDependencies.data(), staticDependencies.size());
}

const skr_resource_handle_t& SCookContextImpl::GetStaticDependency(uint32_t index) const
{
    return staticDependencies[index];
}

uint32_t SCookContextImpl::AddStaticDependency(skr_guid_t resource, bool install)
{
    auto iter = std::find_if(staticDependencies.begin(), staticDependencies.end(), [&](const auto &dep) { return dep.get_serialized() == resource; });
    if (iter == staticDependencies.end())
    {
        auto counter = GetCookSystem()->EnsureCooked(resource);
        if (counter) counter.wait(false);
        skr_resource_handle_t handle{resource};
        handle.resolve(install, (uint64_t)this, SKR_REQUESTER_SYSTEM);
        if(!handle.get_resolved())
        {
            auto record = handle.get_record();
            task::event_t event;
            auto callback = [&]() { event.signal(); };
            record->AddCallback(SKR_LOADING_STATUS_ERROR, callback);
            record->AddCallback(install ? SKR_LOADING_STATUS_INSTALLED : SKR_LOADING_STATUS_LOADED, callback);
            if(!handle.get_resolved())
            {
                event.wait(false);
            }
        }
        staticDependencies.push_back(std::move(handle));
        return (uint32_t)(staticDependencies.size() - 1);
    }
    return (uint32_t)(staticDependencies.end() - iter);
}
}