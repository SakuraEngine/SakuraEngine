#include "SkrCore/module/module.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrBase/misc/defer.hpp"
#include "SkrContainers/string.hpp"
#include "SkrRT/io/ram_io.hpp"
#include "SkrCore/async/thread_job.hpp"

#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"

#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/project/project.hpp"

#include <atomic>

#include "SkrProfile/profile.h"

namespace skd::asset
{
struct SCookSystemImpl : public skd::asset::SCookSystem {
    friend struct ::SkrToolCoreModule;
    using AssetMap   = skr::FlatHashMap<skr_guid_t, SAssetRecord*, skr::Hash<skr_guid_t>>;
    using CookingMap = skr::ParallelFlatHashMap<skr_guid_t, SCookContext*, skr::Hash<skr_guid_t>>;

    skr::task::event_t AddCookTask(skr_guid_t resource) override;
    skr::task::event_t EnsureCooked(skr_guid_t resource) override;
    void               WaitForAll() override;
    bool               AllCompleted() const override;

    void     RegisterCooker(bool isDefault, skr_guid_t cooker, skr_guid_t type, SCooker* instance) override;
    void     UnregisterCooker(skr_guid_t type) override;
    SCooker* GetCooker(SAssetRecord* record) const
    {
        if (record->cooker == skr_guid_t{})
        {
            auto it = defaultCookers.find(record->type);
            if (it != defaultCookers.end()) return it->second;
            return nullptr;
        }
        auto it = cookers.find(record->cooker);
        if (it != cookers.end()) return it->second;
        return nullptr;
    }
    SAssetRecord* GetAssetRecord(skr_guid_t guid) const override
    {
        auto it = assets.find(guid);
        if (it != assets.end()) return it->second;
        return nullptr;
    }

    SAssetRecord*         GetAssetRecord(const skr_guid_t& guid) override;
    SAssetRecord*         ImportAsset(SProject* project, skr::filesystem::path path) override;
    skr_io_ram_service_t* getIOService() override;

    template <class F, class Iter>
    void ParallelFor(Iter begin, Iter end, size_t batch, F f)
    {
        skr::parallel_for(std::move(begin), std::move(end), batch, std::move(f));
    }
    void ParallelForEachAsset(uint32_t batch, skr::FunctionRef<void(skr::span<SAssetRecord*>)> f) override
    {
        ParallelFor(assets.begin(), assets.end(), batch,
                    [f, batch](auto begin, auto end) {
                        skr::Vector<SAssetRecord*> records;
                        records.reserve(batch);
                        for (auto it = begin; it != end; ++it)
                        {
                            records.add(it->second);
                        }
                        f(records);
                    });
    }

protected:
    AssetMap   assets;
    CookingMap cooking;
    SMutex     ioMutex;

    skr::task::counter_t mainCounter;

    skr::FlatHashMap<skr_guid_t, SCooker*, skr::Hash<skr_guid_t>> defaultCookers;
    skr::FlatHashMap<skr_guid_t, SCooker*, skr::Hash<skr_guid_t>> cookers;
    SMutex                                                  assetMutex;
    skr_io_ram_service_t*                                   ioServices[ioServicesMaxCount];
};
} // namespace skd::asset

struct TOOL_CORE_API SkrToolCoreModule : public skr::IDynamicModule {
    skr::JobQueue* io_job_queue          = nullptr;
    skr::JobQueue* io_callback_job_queue = nullptr;
    virtual void   on_load(int argc, char8_t** argv) override
    {
        auto cook_system = (skd::asset::SCookSystemImpl*)skd::asset::GetCookSystem();
        skr_init_mutex(&cook_system->ioMutex);
        skr_init_mutex(&cook_system->assetMutex);

        auto jqDesc         = make_zeroed<skr::JobQueueDesc>();
        jqDesc.thread_count = 1;
        jqDesc.priority     = SKR_THREAD_ABOVE_NORMAL;
        jqDesc.name         = u8"Tool-IOJobQueue";
        io_job_queue        = SkrNew<skr::JobQueue>(jqDesc);

        jqDesc.name           = u8"Tool-IOCallbackJobQueue";
        io_callback_job_queue = SkrNew<skr::JobQueue>(jqDesc);

        for (auto& ioService : cook_system->ioServices)
        {
            // all used up
            if (ioService == nullptr)
            {
                skr_ram_io_service_desc_t desc = {};
                desc.sleep_time                = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
                desc.awake_at_request          = true;
                desc.name                      = u8"Tool-IOService";
                desc.io_job_queue              = io_job_queue;
                desc.callback_job_queue        = io_callback_job_queue;
                ioService                      = skr_io_ram_service_t::create(&desc);
                ioService->run();
            }
        }
    }

    virtual void on_unload() override
    {
        auto cook_system = (skd::asset::SCookSystemImpl*)skd::asset::GetCookSystem();
        skr_destroy_mutex(&cook_system->ioMutex);
        for (auto ioService : cook_system->ioServices)
        {
            if (ioService)
                skr_io_ram_service_t::destroy(ioService);
        }

        skr_destroy_mutex(&cook_system->assetMutex);
        for (auto& pair : cook_system->assets)
            SkrDelete(pair.second);

        SkrDelete(io_callback_job_queue);
        SkrDelete(io_job_queue);
    }
};
IMPLEMENT_DYNAMIC_MODULE(SkrToolCoreModule, SkrToolCore);

namespace skd::asset
{
SCookSystem* GetCookSystem()
{
    static skd::asset::SCookSystemImpl cook_system;
    return &cook_system;
}

void RegisterCookerToSystem(SCookSystem* system, bool isDefault, skr_guid_t cooker, skr_guid_t type, SCooker* instance)
{
    system->RegisterCooker(isDefault, cooker, type, instance);
}

void SCookSystemImpl::WaitForAll()
{
    mainCounter.wait(true);
}

bool SCookSystemImpl::AllCompleted() const
{
    return mainCounter.test();
}

skr_io_ram_service_t* SCookSystemImpl::getIOService()
{
    SMutexLock                  lock(ioMutex);
    static std::atomic_uint32_t cursor = 0;
    cursor                             = (cursor % ioServicesMaxCount);
    return ioServices[cursor++];
}

skr::task::event_t SCookSystemImpl::AddCookTask(skr_guid_t guid)
{
    SCookContext* jobContext;
    {
        skr::task::event_t result{ nullptr };
        cooking.lazy_emplace_l(
        guid,
        [&](const auto& ctx_kv) { result = ctx_kv.second->GetCounter(); },
        [&](const CookingMap::constructor& ctor) {
            jobContext = SCookContext::Create(getIOService());
            ctor(guid, jobContext);
        });
        if (result) return result;
    }
    jobContext->record = GetAssetRecord(guid);
    skr::task::event_t counter;
    jobContext->SetCounter(counter);
    auto guidName = skr::format(u8"Fiber{}", jobContext->record->guid);
    mainCounter.add(1);
    skr::task::schedule([jobContext]() {
        auto       system    = static_cast<SCookSystemImpl*>(GetCookSystem());
        const auto metaAsset = jobContext->record;
        auto       cooker    = system->GetCooker(metaAsset);
        SKR_ASSERT(cooker);
        // Trace
        SkrZoneScopedN("CookingTask");
        const auto rtti_type           = skr::rttr::get_type_from_guid(metaAsset->type);
        const auto cookerTypeName      = rtti_type ? rtti_type->name().raw().c_str() : (const ochar_t*)u8"UnknownResource";
        const auto guidString          = skr::format(u8"Guid: {}", metaAsset->guid);
        const auto assetTypeGuidString = skr::format(u8"TypeGuid: {}", metaAsset->type);
        const auto scopeName           = skr::format(u8"Cook.[{}]", (const ochar8_t*)cookerTypeName);
        const auto assetString         = skr::format(u8"Asset: {}", metaAsset->path.u8string().c_str());
        ZoneName(scopeName.c_str(), scopeName.size());
        SkrMessage(guidString.c_str(), guidString.size());
        SkrMessage(assetTypeGuidString.c_str(), assetTypeGuidString.size());
        SkrMessage(assetString.c_str(), assetString.size());

        SKR_DEFER({
            auto system = static_cast<SCookSystemImpl*>(GetCookSystem());
            auto guid   = jobContext->record->guid;
            system->cooking.erase_if(guid, [](const auto& ctx_kv) { SCookContext::Destroy(ctx_kv.second); return true; });
            system->mainCounter.decrement();
        });

        // Create output dir
        auto            outputPath = metaAsset->project->GetOutputPath();
        std::error_code ec         = {};
        skr::filesystem::create_directories(outputPath, ec);

        // TODO: platform dependent directory
        jobContext->SetOutputPath(outputPath / skr::format(u8"{}.bin", metaAsset->guid).c_str());
        if (!cooker)
        {
            return;
        }

        // Cook
        jobContext->SetCookerVersion(cooker->Version());
        // SKR_ASSERT(iter != system->cookers.end()); // TODO: error handling
        SKR_LOG_INFO(u8"[CookTask] resource %s cook started!", metaAsset->path.u8string().c_str());
        if (cooker->Cook(jobContext))
        {
            // write resource header
            {
                SKR_LOG_INFO(u8"[CookTask] resource %s cook finished! updating resource metas.", metaAsset->path.u8string().c_str());
                auto headerPath = jobContext->GetOutputPath();
                headerPath.replace_extension("rh");
                skr::Vector<uint8_t>      buffer;
                skr::binary::VectorWriter writer{ &buffer };
                SBinaryWriter       archive(writer);
                jobContext->WriteHeader(archive, cooker);
                auto file = fopen(headerPath.string().c_str(), "wb");
                if (!file)
                {
                    SKR_LOG_ERROR(u8"[CookTask] failed to write header file for resource %s!", metaAsset->path.u8string().c_str());
                    return;
                }
                SKR_DEFER({ fclose(file); });
                fwrite(buffer.data(), 1, buffer.size(), file);
            }

            // write resource dependencies
            {
                SKR_LOG_INFO(u8"[CookTask] resource %s cook finished! updating dependencies.", metaAsset->path.u8string().c_str());
                // write dependencies
                auto              dependencyPath = metaAsset->project->GetDependencyPath() / skr::format(u8"{}.d", metaAsset->guid).c_str();
                skr::archive::JsonWriter writer(2);
                writer.StartObject();
                writer.Key(u8"importerVersion");
                writer.UInt64(jobContext->GetImporterVersion());
                writer.Key(u8"cookerVersion");
                writer.UInt64(jobContext->GetCookerVersion());
                writer.Key(u8"files");
                writer.StartArray();
                for (auto& dep : jobContext->GetFileDependencies())
                {
                    auto str = dep.string();
                    skr::json::Write<skr::StringView>(&writer, { (const char8_t*)str.data(), str.size() });
                }
                writer.EndArray();
                writer.Key(u8"dependencies");
                writer.StartArray();
                for (auto& dep : jobContext->GetStaticDependencies())
                    skr::json::Write<skr_resource_handle_t>(&writer, dep);
                writer.EndArray();
                writer.EndObject();
                auto file = fopen(dependencyPath.string().c_str(), "w");
                if (!file)
                {
                    SKR_LOG_ERROR(u8"[CookTask] failed to write dependency file for resource %s!", metaAsset->path.u8string().c_str());
                    return;
                }
                SKR_DEFER({ fclose(file); });
                auto jString = writer.Write();
                fwrite(jString.raw().data(), 1, jString.raw().size(), file);
            }
        }
    },
                        &counter, guidName.c_str());
    return counter;
}

void SCookSystemImpl::RegisterCooker(bool isDefault, skr_guid_t cooker, skr_guid_t type, SCooker* instance)
{
    SKR_ASSERT(instance->system == nullptr);
    instance->system = this;
    cookers.insert(std::make_pair(cooker, instance));
    if (isDefault)
    {
        auto result = defaultCookers.insert(std::make_pair(type, instance));
        SKR_ASSERT(result.second);
        (void)result;
    }
}

void SCookSystemImpl::UnregisterCooker(skr_guid_t guid)
{
    cookers.erase(guid);
}

#define SKR_CHECK_RESULT(result, name)                                                                                                       \
    if (result.error() != simdjson::SUCCESS)                                                                                                 \
    {                                                                                                                                        \
        SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] " name " file parse failed! resource guid: %s", metaAsset->path.u8string().c_str()); \
        return false;                                                                                                                        \
    }

skr::task::event_t SCookSystemImpl::EnsureCooked(skr_guid_t guid)
{
    SkrZoneScoped;
    {
        skr::task::event_t result{ nullptr };
        cooking.if_contains(guid, [&](const auto& ctx_kv) {
            result = ctx_kv.second->GetCounter();
        });
        if (result)
            return result;
    }
    auto metaAsset = GetAssetRecord(guid);
    if (!metaAsset)
    {
        SKR_LOG_ERROR(u8"[SCookSystemImpl::EnsureCooked] resource not exist! asset path: %s", metaAsset->path.u8string().c_str());
        return nullptr;
    }
    auto resourcePath   = metaAsset->project->GetOutputPath() / skr::format(u8"{}.bin", metaAsset->guid).u8_str();
    auto dependencyPath = metaAsset->project->GetDependencyPath() / skr::format(u8"{}.d", metaAsset->guid).u8_str();
    auto checkUpToDate  = [&]() -> bool {
        auto cooker = GetCooker(metaAsset);
        if (!cooker)
        {
            SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] cooker not found! asset path: %s", metaAsset->path.u8string().c_str());
            return true;
        }
        std::error_code ec = {};
        if (!skr::filesystem::is_regular_file(resourcePath, ec))
        {
            SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] resource not exist! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        if (!skr::filesystem::is_regular_file(dependencyPath, ec))
        {
            SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] dependency file not exist! asset path: %s}", dependencyPath.string().c_str());
            return false;
        }
        auto timestamp = skr::filesystem::last_write_time(resourcePath, ec);
        if (skr::filesystem::last_write_time(metaAsset->path, ec) > timestamp)
        {
            SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] meta file modified! resource path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        // TODO: refactor this
        skr::String depFileContent;
        {
            auto dependencyFile = fopen(dependencyPath.string().c_str(), "rb");
            if (!dependencyFile)
            {
                SKR_LOG_ERROR(u8"Failed to open dependency file: %s", dependencyPath.string().c_str());
                return false;
            }
            fseek(dependencyFile, 0, SEEK_END);
            auto fileSize = ftell(dependencyFile);
            fseek(dependencyFile, 0, SEEK_SET);
            depFileContent.append(u8'0', fileSize);
            fread(depFileContent.raw().data(), 1, fileSize, dependencyFile);
            fclose(dependencyFile);  
        }
        skr::archive::JsonReader depReader(depFileContent.view());
        skr::archive::JsonReader metaReader(metaAsset->meta.view());
        depReader.StartObject(); 
        metaReader.StartObject();
        SKR_DEFER({ depReader.EndObject(); metaReader.EndObject(); });
        skr_guid_t importerTypeGuid;
        {
            metaReader.Key(u8"importer");
            metaReader.StartObject();
            metaReader.Key(u8"importerType");
            if (!skr::json::Read(&metaReader, importerTypeGuid))
            {
                SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] meta file parse failed! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            metaReader.EndObject();
        }
        uint64_t importerVersion;
        {
            depReader.Key(u8"importerVersion");
            depReader.UInt64(importerVersion);
        }
        auto currentImporterVersion = GetImporterRegistry()->GetImporterVersion(importerTypeGuid);
        if (importerVersion != currentImporterVersion)
        {
            SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] importer version changed! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        if (currentImporterVersion == UINT32_MAX)
        {
            SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] dev importer version (UINT32_MAX)! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        {
            if (cooker->Version() == UINT32_MAX)
            {
                SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] dev cooker version (UINT32_MAX)! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            auto resourceFile = fopen(resourcePath.string().c_str(), "rb");
            SKR_DEFER({ fclose(resourceFile); });
            uint8_t buffer[sizeof(skr_resource_header_t)];
            fread(buffer, 0, sizeof(skr_resource_header_t), resourceFile);
            skr::binary::SpanReader reader = { buffer };
            SBinaryReader     archive{ reader };
            skr_resource_header_t   header;
            if (header.ReadWithoutDeps(&archive) != 0)
            {
                SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] resource header read failed! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            if (header.version != cooker->Version())
            {
                SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] cooker version changed! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
        }
        // analyze dep files
        {
            size_t filesSize = 0;
            depReader.Key(u8"files");
            depReader.StartArray(filesSize);
            for (size_t i = 0; i < filesSize; i++)
            {
                skr::String pathStr;
                skr::json::Read(&depReader, pathStr);
                skr::filesystem::path path(pathStr.c_str());
                path               = metaAsset->path.parent_path() / (path);
                std::error_code ec = {};
                if (!skr::filesystem::exists(path, ec))
                {
                    SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] file not exist! asset path: %s", metaAsset->path.u8string().c_str());
                    return false;
                }
                if (skr::filesystem::last_write_time(path, ec) > timestamp)
                {
                    SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] file modified! asset path: %s", metaAsset->path.u8string().c_str());
                    return false;
                }
            }
            depReader.EndArray();
        }
        // analyze dependencies
        {
            size_t depsSize = 0;
            depReader.Key(u8"dependencies");
            depReader.StartArray(depsSize);
            for (size_t i = 0; i < depsSize; i++)
            {
                skr_guid_t depGuid;
                skr::json::Read(&depReader, depGuid);
                auto record = GetAssetRecord(depGuid);
                if (!record)
                {
                    SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] dependency file not exist! asset path: %s", metaAsset->path.u8string().c_str());
                    return false;
                }
                if (record->type == skr_guid_t{})
                {
                    if (skr::filesystem::last_write_time(record->path, ec) > timestamp)
                    {
                        SKR_LOG_INFO(u8"[SCookSystemImpl::EnsureCooked] dependency file %s modified! asset path: %s", record->path.u8string().c_str(), metaAsset->path.u8string().c_str());
                        return false;
                    }
                }
                else
                {
                    if (EnsureCooked(depGuid))
                        return false;
                }
            }
            depReader.EndArray();
        }

        return true;
    };
    if (!checkUpToDate())
        return AddCookTask(guid);
    return nullptr;
}

SAssetRecord* SCookSystemImpl::ImportAsset(SProject* project, skr::filesystem::path path)
{
    SkrZoneScoped;
    std::error_code ec     = {};
    auto            record = SkrNew<SAssetRecord>();
    // TODO: replace file load with skr api
    {
        auto assetFile = fopen(path.string().c_str(), "rb");
        SKR_DEFER({ fclose(assetFile); });
        fseek(assetFile, 0, SEEK_END);
        auto fileSize = ftell(assetFile);
        fseek(assetFile, 0, SEEK_SET);
        record->meta.append(u8'0', fileSize);
        fread(record->meta.raw().data(), 1, fileSize, assetFile);
        fclose(assetFile);
    }
    skr::archive::JsonReader reader(record->meta.view());
    reader.StartObject();
    SKR_DEFER({ reader.EndObject(); });
    // read guid
    {
        std::memset(&record->guid, 0, sizeof(skr_guid_t));
        reader.Key(u8"guid");
        skr::json::Read(&reader, record->guid);
    }
    // read type
    {
        std::memset(&record->type, 0, sizeof(skr_guid_t));
        reader.Key(u8"type");
        skr::json::Read(&reader, record->type);
    }
    // read cookerType
    /*
    {
        std::memset(&record->cooker, 0, sizeof(skr_guid_t));
        reader.Key(u8"cookerType");
        skr::json::Read(&reader, record->cooker);
    }
    */
    record->path    = skr::filesystem::relative(path, project->GetAssetPath());
    record->project = project;
    SMutexLock lock(assetMutex);
    assets.insert(std::make_pair(record->guid, record));
    return record;
}
SAssetRecord* SCookSystemImpl::GetAssetRecord(const skr_guid_t& guid)
{
    auto iter = assets.find(guid);
    return iter != assets.end() ? iter->second : nullptr;
}
} // namespace skd::asset