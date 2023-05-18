#include "module/module.hpp"
#include "misc/parallel_for.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/project/project.hpp"
#include "platform/guid.hpp"
#include "containers/string.hpp"
#include "misc/defer.hpp"
#include "misc/io.h"

#include "serde/json/reader.h"
#include "serde/json/writer.h"
#include "serde/binary/writer.h"
#include <atomic>

#include "tracy/Tracy.hpp"

namespace skd::asset
{
struct SCookSystemImpl : public skd::asset::SCookSystem
{
    friend struct ::SkrToolCoreModule;
    using AssetMap = skr::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash>;
    using CookingMap = skr::parallel_flat_hash_map<skr_guid_t, SCookContext*, skr::guid::hash>;

    skr::task::event_t AddCookTask(skr_guid_t resource) override;
    skr::task::event_t EnsureCooked(skr_guid_t resource) override;
    void WaitForAll() override;
    bool AllCompleted() const override;

    void RegisterCooker(bool isDefault, skr_guid_t cooker, skr_guid_t type, SCooker* instance) override;
    void UnregisterCooker(skr_guid_t type) override;
    SCooker* GetCooker(SAssetRecord* record) const
    {
        if(record->cooker == skr_guid_t{})
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

    SAssetRecord* GetAssetRecord(const skr_guid_t& guid) override;
    SAssetRecord* ImportAsset(SProject* project, skr::filesystem::path path) override;
    skr_io_ram_service_t* getIOService() override;

    template <class F, class Iter>
    void ParallelFor(Iter begin, Iter end, size_t batch, F f)
    {
        skr::parallel_for(std::move(begin), std::move(end), batch, std::move(f));
    }
    void ParallelForEachAsset(uint32_t batch, skr::function_ref<void(skr::span<SAssetRecord*>)> f) override
    {
        ParallelFor(assets.begin(), assets.end(), batch, 
        [f, batch](auto begin, auto end){
            skr::vector<SAssetRecord*> records;
            records.reserve(batch);
            for (auto it = begin; it != end; ++it)
            {
                records.emplace_back(it->second);
            }
            f(records);
        });
    }
protected:
    AssetMap assets;
    CookingMap cooking;
    SMutex ioMutex;

    skr::task::counter_t mainCounter;

    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> defaultCookers;
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cookers;
    SMutex assetMutex;
    skr_io_ram_service_t* ioServices[ioServicesMaxCount];
};
}

struct TOOL_CORE_API SkrToolCoreModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        skr_init_mutex(&cook_system.ioMutex);
        skr_init_mutex(&cook_system.assetMutex);
        for (auto& ioService : cook_system.ioServices)
        {
            // all used up
            if (ioService == nullptr)
            {
                skr_ram_io_service_desc_t desc = {};
                desc.sleep_time = 1;
                desc.lockless = true;
                desc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_NEVER;
                desc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
                ioService = skr_io_ram_service_t::create(&desc);
            }
        }
    }

    virtual void on_unload() override
    {
        skr_destroy_mutex(&cook_system.ioMutex);
        for (auto ioService : cook_system.ioServices)
        {
            if (ioService)
                skr_io_ram_service_t::destroy(ioService);
        }

        skr_destroy_mutex(&cook_system.assetMutex);
        for (auto& pair : cook_system.assets)
            SkrDelete(pair.second);
    }
    static skd::asset::SCookSystemImpl cook_system;
};
IMPLEMENT_DYNAMIC_MODULE(SkrToolCoreModule, SkrToolCore);
skd::asset::SCookSystemImpl SkrToolCoreModule::cook_system;

namespace skd::asset
{
SCookSystem* GetCookSystem()
{
    return &SkrToolCoreModule::cook_system;
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
    SMutexLock lock(ioMutex);
    static std::atomic_uint32_t cursor = 0;
    cursor = (cursor % ioServicesMaxCount);
    return ioServices[cursor++];
}

skr::task::event_t SCookSystemImpl::AddCookTask(skr_guid_t guid)
{
    SCookContext* jobContext;
    {
        skr::task::event_t result{nullptr};
        cooking.lazy_emplace_l(guid,
        [&](SCookContext* ctx) { result = ctx->GetCounter(); },
        [&](const CookingMap::constructor& ctor) {
            jobContext = SCookContext::Create(getIOService());
            ctor(guid, jobContext);
        });
        if(result) return result;
    }
    jobContext->record = GetAssetRecord(guid);
    skr::task::event_t counter;
    jobContext->SetCounter(counter);
    auto guidName = skr::format(u8"Fiber{}", jobContext->record->guid);
    mainCounter.add(1);
    skr::task::schedule([jobContext]()
    {
        auto system = static_cast<SCookSystemImpl*>(GetCookSystem());
        const auto metaAsset = jobContext->record;
        auto cooker = system->GetCooker(metaAsset);
        SKR_ASSERT(cooker);
        // Trace
        ZoneScoped;
        const auto rtti_type = type::GetTypeRegistry()->get_type(metaAsset->type);
        const auto type_name = skr_get_type_name(&metaAsset->type);
        const auto cookerTypeName = rtti_type ? rtti_type->Name() : type_name ? type_name : u8"UnknownResource";
        const auto guidString = skr::format(u8"Guid: {}", metaAsset->guid);
        const auto assetTypeGuidString = skr::format(u8"TypeGuid: {}", metaAsset->type);
        const auto scopeName = skr::format(u8"Cook.[{}]", (const ochar8_t*)cookerTypeName);
        const auto assetString = skr::format(u8"Asset: {}", metaAsset->path.u8string().c_str());
        ZoneName(scopeName.c_str(), scopeName.size());
        TracyMessage(guidString.c_str(), guidString.size());
        TracyMessage(assetTypeGuidString.c_str(), assetTypeGuidString.size());
        TracyMessage(assetString.c_str(), assetString.size());

        SKR_DEFER({
            auto system = static_cast<SCookSystemImpl*>(GetCookSystem());
            auto guid = jobContext->record->guid;
            system->cooking.erase_if(guid, [](SCookContext* context) { SCookContext::Destroy(context); return true; });
            system->mainCounter.decrement();
        });

        // Create output dir
        auto outputPath = metaAsset->project->outputPath;
        std::error_code ec = {};
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
        SKR_LOG_INFO("[CookTask] resource %s cook started!", metaAsset->path.u8string().c_str());
        if (cooker->Cook(jobContext))
        {
            // write resource header
            {
                SKR_LOG_INFO("[CookTask] resource %s cook finished! updating resource metas.", metaAsset->path.u8string().c_str());
                auto headerPath = jobContext->GetOutputPath();
                headerPath.replace_extension("rh");
                eastl::vector<uint8_t> buffer;
                skr::binary::VectorWriter writer{&buffer};
                skr_binary_writer_t archive(writer);
                jobContext->WriteHeader(archive, cooker);
                auto file = fopen(headerPath.string().c_str(), "wb");
                if (!file)
                {
                    SKR_LOG_ERROR("[CookTask] failed to write header file for resource %s!", metaAsset->path.u8string().c_str());
                    return;
                }
                SKR_DEFER({ fclose(file); });
                fwrite(buffer.data(), 1, buffer.size(), file);
            }

            // write resource dependencies
            {
                SKR_LOG_INFO("[CookTask] resource %s cook finished! updating dependencies.", metaAsset->path.u8string().c_str());
                // write dependencies
                auto dependencyPath = metaAsset->project->dependencyPath / skr::format(u8"{}.d", metaAsset->guid).c_str();
                skr_json_writer_t writer(2);
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
                    skr::json::Write<const skr::string_view&>(&writer, {(const char8_t*)str.data(), static_cast<int32_t>(str.size()) });
                }
                writer.EndArray();
                writer.Key(u8"dependencies");
                writer.StartArray();
                for (auto& dep : jobContext->GetStaticDependencies())
                    skr::json::Write<const skr_resource_handle_t&>(&writer, dep);
                writer.EndArray();
                writer.EndObject();
                auto file = fopen(dependencyPath.string().c_str(), "w");
                if (!file)
                {
                    SKR_LOG_ERROR("[CookTask] failed to write dependency file for resource %s!", metaAsset->path.u8string().c_str());
                    return;
                }
                SKR_DEFER({ fclose(file); });
                fwrite(writer.buffer.c_str(), 1, writer.buffer.size(), file);
            }
        }
    }, &counter, guidName.c_str());
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

#define SKR_CHECK_RESULT(result, name) \
    if (result.error() != simdjson::SUCCESS) \
    { \
        SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] " name " file parse failed! resource guid: %s", metaAsset->path.u8string().c_str()); \
        return false; \
    }

skr::task::event_t SCookSystemImpl::EnsureCooked(skr_guid_t guid)
{
    {
        skr::task::event_t result{nullptr};
        cooking.if_contains(guid, [&](SCookContext* ctx) {
            result = ctx->GetCounter();
        });
        if (result)
            return result;
    }
    auto metaAsset = GetAssetRecord(guid);
    if (!metaAsset)
    {
        SKR_LOG_ERROR("[SCookSystemImpl::EnsureCooked] resource not exist! asset path: %s", metaAsset->path.u8string().c_str());
        return nullptr;
    }
    auto resourcePath = metaAsset->project->outputPath / skr::format(u8"{}.bin", metaAsset->guid).u8_str();
    auto dependencyPath = metaAsset->project->dependencyPath / skr::format(u8"{}.d", metaAsset->guid).u8_str();
    auto checkUpToDate = [&]() -> bool {
        auto cooker = GetCooker(metaAsset);
        if(!cooker)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] cooker not found! asset path: %s", metaAsset->path.u8string().c_str());
            return true;
        }
        std::error_code ec = {};
        if (!skr::filesystem::is_regular_file(resourcePath, ec))
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] resource not exist! asset path: %s" , metaAsset->path.u8string().c_str());
            return false;
        }
        if (!skr::filesystem::is_regular_file(dependencyPath, ec))
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dependency file not exist! asset path: %s}", dependencyPath.string().c_str());
            return false;
        }
        auto timestamp = skr::filesystem::last_write_time(resourcePath, ec);
        if (skr::filesystem::last_write_time(metaAsset->path, ec) > timestamp)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] meta file modified! resource path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        simdjson::ondemand::parser parser;
        auto json = simdjson::padded_string::load(dependencyPath.string());
        auto doc = parser.iterate(json);
        if (doc.error() != simdjson::SUCCESS)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dependency file parse failed! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        simdjson::ondemand::parser metaParser;
        auto metaDoc = metaParser.iterate(metaAsset->meta);
        SKR_CHECK_RESULT(metaDoc, "meta")
        auto importer = metaDoc["importer"];
        SKR_CHECK_RESULT(importer, "meta")
        auto importerType = importer["importerType"];
        SKR_CHECK_RESULT(importerType, "meta")
        skr_guid_t importerTypeGuid;
        if(skr::json::Read(std::move(importerType).value_unsafe(), importerTypeGuid) != skr::json::SUCCESS)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] meta file parse failed! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        auto importerVersion = doc["importerVersion"].get_uint64();
        SKR_CHECK_RESULT(importerVersion, "dependency")
        auto currentImporterVersion = GetImporterRegistry()->GetImporterVersion(importerTypeGuid);
        if(importerVersion.value_unsafe() != currentImporterVersion)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] importer version changed! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        if(currentImporterVersion == UINT32_MAX)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dev importer version (UINT32_MAX)! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        {
            
            if (cooker->Version() == UINT32_MAX)
            {
                SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dev cooker version (UINT32_MAX)! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            auto resourceFile = fopen(resourcePath.string().c_str(), "rb");
            SKR_DEFER({ fclose(resourceFile); });
            uint8_t buffer[sizeof(skr_resource_header_t)];
            fread(buffer, 0, sizeof(skr_resource_header_t), resourceFile);
            SKR_DEFER({ fclose(resourceFile); });
            skr::binary::SpanReader reader = {buffer};
            skr_binary_reader_t archive{reader};
            skr_resource_header_t header;
            if(header.ReadWithoutDeps(&archive) != 0)
            {
                SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] resource header read failed! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            if (header.version != cooker->Version())
            {
                SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] cooker version changed! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
        }
        auto files = doc["files"].get_array();
        if (files.error() != simdjson::SUCCESS)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dependency file parse failed! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        for (auto file : files.value_unsafe())
        {
            skr::string pathStr;
            skr::json::Read(std::move(file).value_unsafe(), pathStr);
            skr::filesystem::path path(pathStr.c_str());
            path = metaAsset->path.parent_path() / (path);
            std::error_code ec = {};
            if(!skr::filesystem::exists(path, ec))
            {
                SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] file not exist! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            if (skr::filesystem::last_write_time(path, ec) > timestamp)
            {
                SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] file modified! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
        }
        auto deps = doc["dependencies"].get_array();
        if (deps.error() != simdjson::SUCCESS)
        {
            SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dependency file parse failed! asset path: %s", metaAsset->path.u8string().c_str());
            return false;
        }
        for (auto depFile : deps.value_unsafe())
        {
            skr_guid_t depGuid;
            skr::json::Read(std::move(depFile).value_unsafe(), depGuid);
            auto record = GetAssetRecord(depGuid);
            if (!record)
            {
                SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dependency file not exist! asset path: %s", metaAsset->path.u8string().c_str());
                return false;
            }
            if (record->type == skr_guid_t{})
            {
                if (skr::filesystem::last_write_time(record->path, ec) > timestamp)
                {
                    SKR_LOG_INFO("[SCookSystemImpl::EnsureCooked] dependency file %s modified! asset path: %s", record->path.u8string().c_str(), metaAsset->path.u8string().c_str());
                    return false;
                }
            }
            else
            {
                if (EnsureCooked(depGuid))
                    return false;
            }
        }
        return true;
    };
    if (!checkUpToDate())
        return AddCookTask(guid);
    return nullptr;
}

SAssetRecord* SCookSystemImpl::ImportAsset(SProject* project, skr::filesystem::path path)
{
    std::error_code ec = {};
    if (path.is_relative())
        path = project->assetPath / path;
    auto record = SkrNew<SAssetRecord>();
    // TODO: replace file load with skr api
    record->meta = simdjson::padded_string::load(path.string()).value_unsafe();
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    skr::json::Read(doc["guid"].value_unsafe(), record->guid);
    auto otype = doc["type"];
    if (otype.error() == simdjson::SUCCESS)
        skr::json::Read(std::move(otype).value_unsafe(), record->type);
    else
        std::memset(&record->type, 0, sizeof(skr_guid_t));
    auto ctype = doc["cookerType"];
    if (ctype.error() == simdjson::SUCCESS)
        skr::json::Read(std::move(ctype).value_unsafe(), record->cooker);
    else
        std::memset(&record->cooker, 0, sizeof(skr_guid_t));
    record->path = path;
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