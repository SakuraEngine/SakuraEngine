#include <EASTL/shared_ptr.h>
#include <ghc/filesystem.hpp>
#include "simdjson.h"
#include "asset/cooker.hpp"
#include "asset/importer.hpp"
#include "platform/memory.h"
#include "platform/debug.h"
#include "platform/guid.hpp"
#include "platform/vfs.h"
#include "platform/thread.h"
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "utils/log.hpp"
#include "utils/io.hpp"

#include "json/reader.h"
#include "json/writer.h"

namespace skd::asset
{
SCookSystem* GetCookSystem()
{
    static SCookSystem instance;
    return &instance;
}
SCookSystem::SCookSystem() noexcept
{
    skr_init_mutex(&ioMutex);
    skr_init_mutex(&assetMutex);
    for (auto& ioService : ioServices)
    {
        // all used up
        if (ioService == nullptr)
        {
            skr_ram_io_service_desc_t desc = {};
            desc.sleep_time = 1;
            desc.lockless = true;
            desc.sort_method = SKR_IO_SERVICE_SORT_METHOD_NEVER;
            desc.sleep_mode = SKR_IO_SERVICE_SLEEP_MODE_SLEEP;
            ioService = skr::io::RAMService::create(&desc);
        }
    }
}
SCookSystem::~SCookSystem() noexcept
{
    skr_destroy_mutex(&ioMutex);
    for (auto ioService : ioServices)
    {
        if (ioService)
            skr::io::RAMService::destroy(ioService);
    }

    skr_destroy_mutex(&assetMutex);
    for (auto& pair : assets)
        SkrDelete(pair.second);
}
SProject::~SProject() noexcept
{
    if (vfs) skr_free_vfs(vfs);
}
void SCookSystem::WaitForAll()
{
    mainCounter.wait(true);
}

#include <atomic>
skr::io::RAMService* SCookSystem::getIOService()
{
    SMutexLock lock(ioMutex);
    static std::atomic_uint32_t cursor = 0;
    cursor = (cursor % ioServicesMaxCount);
    return ioServices[cursor++];
}

skr::task::event_t SCookSystem::AddCookTask(skr_guid_t guid)
{
    SCookContext* jobContext;
    {
        skr::task::event_t result;
        cooking.lazy_emplace_l(
        guid, [&](SCookContext* ctx) { result = ctx->counter; },
        [&](const CookingMap::constructor& ctor) {
            jobContext = SkrNew<SCookContext>();
            ctor(guid, jobContext);
        });
        if(result)
            return result;
    }

    jobContext->record = GetAssetRecord(guid);
    jobContext->ioService = getIOService();
    skr::task::event_t counter;
    jobContext->counter = counter;
    auto guidName = fmt::format("Fiber{}", jobContext->record->guid);
    mainCounter.add(1);
    skr::task::schedule([jobContext]()
    {
        SKR_DEFER({
            auto system = GetCookSystem();
            auto guid = jobContext->record->guid;
            system->cooking.erase_if(guid, [](SCookContext* context) { SkrDelete(context); return true; });
            system->mainCounter.decrement();
        });
        auto metaAsset = jobContext->record;
        auto outputPath = metaAsset->project->outputPath;
        std::error_code ec = {};
        ghc::filesystem::create_directories(outputPath, ec);
        // TODO: platform dependent directory
        jobContext->output = outputPath / fmt::format("{}.bin", metaAsset->guid);
        auto system = GetCookSystem();
        auto iter = system->cookers.find(metaAsset->type);
        if (iter == system->cookers.end())
        {
            return;
        }
        // SKR_ASSERT(iter != system->cookers.end()); // TODO: error handling
        SKR_LOG_FMT_INFO("[CookTask] resource {} cook started!", metaAsset->guid);
        if (iter->second->Cook(jobContext))
        {
            SKR_LOG_FMT_INFO("[CookTask] resource {} cook finished! updating dependencies.", metaAsset->guid);
            // write dependencies
            auto dependencyPath = metaAsset->project->dependencyPath / fmt::format("{}.d", metaAsset->guid);
            skr_json_writer_t writer(2);
            writer.StartObject();
            writer.Key("files");
            writer.StartArray();
            for (auto& dep : jobContext->staticDependencies)
                skr::json::Write<const skr_guid_t&>(&writer, dep);
            writer.EndArray();
            writer.EndObject();
            auto file = fopen(dependencyPath.u8string().c_str(), "w");
            if (!file)
            {
                SKR_LOG_FMT_ERROR("[CookTask] failed to write dependency file for resource {}! path: {}", metaAsset->guid, metaAsset->path.u8string());
                return;
            }
            SKR_DEFER({ fclose(file); });
            fwrite(writer.buffer.data(), 1, writer.buffer.size(), file);
        }
    }, &counter, guidName.c_str());
    return counter;
}

void SCookSystem::RegisterCooker(skr_guid_t guid, SCooker* cooker)
{
    SKR_ASSERT(cooker->system == nullptr);
    cooker->system = this;
    cookers.insert(std::make_pair(guid, cooker));
}
void SCookSystem::UnregisterCooker(skr_guid_t guid)
{
    cookers.erase(guid);
}

skr::task::event_t SCookSystem::EnsureCooked(skr_guid_t guid)
{
    {
        skr::task::event_t result;
        cooking.if_contains(guid, [&](SCookContext* ctx) {
            result = ctx->counter;
        });
        if (result)
            return result;
    }
    auto metaAsset = GetAssetRecord(guid);
    if (!metaAsset)
    {
        SKR_LOG_FMT_ERROR("[SCookSystem::EnsureCooked] resource not exist! resource guid: {}", guid);
        return nullptr;
    }
    auto resourcePath = metaAsset->project->outputPath / fmt::format("{}.bin", metaAsset->guid);
    auto dependencyPath = metaAsset->project->dependencyPath / fmt::format("{}.d", metaAsset->guid);
    auto checkUpToDate = [&]() -> bool {
        std::error_code ec = {};
        if (!ghc::filesystem::is_regular_file(resourcePath, ec))
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] resource not exist! resource guid: {}", guid);
            return false;
        }
        if (!ghc::filesystem::is_regular_file(dependencyPath, ec))
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file not exist! resource guid: {}", guid);
            return false;
        }
        auto timestamp = ghc::filesystem::last_write_time(resourcePath, ec);
        if (ghc::filesystem::last_write_time(metaAsset->path, ec) > timestamp)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] meta file modified! resource guid: {}", guid);
            return false;
        }
        simdjson::ondemand::parser parser;
        auto json = simdjson::padded_string::load(dependencyPath.u8string());
        auto doc = parser.iterate(json);
        if (doc.error() != simdjson::SUCCESS)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file parse failed! resource guid: {}", guid);
            return false;
        }
        auto deps = doc["files"].get_array();
        if (deps.error() != simdjson::SUCCESS)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file parse failed! resource guid: {}", guid);
            return false;
        }
        {
            auto iter = cookers.find(metaAsset->type);
            SKR_ASSERT(iter != cookers.end());
            auto resourceFile = fopen(resourcePath.u8string().c_str(), "rb");
            SKR_DEFER({ fclose(resourceFile); });
            uint32_t version[2];
            fread(&version, 1, sizeof(uint32_t) * 2, resourceFile);
            if (version[1] != iter->second->Version())
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] cooker version changed! resource guid: {}", guid);
                return false;
            }
            if (iter->second->Version() == UINT32_MAX)
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dev cooker version (UINT32_MAX)! resource guid: {}", guid);
                return false;
            }
        }

        for (auto depFile : deps.value_unsafe())
        {
            skr_guid_t depGuid;
            skr::json::Read(std::move(depFile).value_unsafe(), depGuid);
            auto record = GetAssetRecord(depGuid);
            if (!record)
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file {} not exist! resource guid: {}", depGuid, guid);
                return false;
            }
            if (record->type == skr_guid_t{})
            {
                if (ghc::filesystem::last_write_time(record->path, ec) > timestamp)
                {
                    SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file {} modified! resource guid: {}", record->path.u8string(), guid);
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
void* SCookSystem::CookOrLoad(skr_guid_t resource)
{
    auto counter = EnsureCooked(resource);
    if (counter)
        counter.wait(false);
    SKR_UNIMPLEMENTED_FUNCTION();
    // LOAD
    return nullptr;
}

void* SCookContext::_Import()
{
    //-----load importer
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    auto importerJson = doc["importer"]; // import from asset
    if (importerJson.error() == simdjson::SUCCESS)
    {
        auto importer = GetImporterRegistry()->LoadImporter(record, std::move(importerJson).value_unsafe());
        //-----import raw data
        auto asset = GetCookSystem()->GetAssetRecord(importer->assetGuid);
        staticDependencies.push_back(asset->guid);
        auto rawData = importer->Import(ioService, asset);
        SKR_LOG_FMT_INFO("[SConfigCooker::Cook] asset imported for resource {}! path: {}", record->guid, asset->path.u8string());
        return rawData;
    }
    auto parentJson = doc["parent"]; // derived from resource
    if (parentJson.error() == simdjson::SUCCESS)
    {
        skr_guid_t parentGuid;
        skr::json::Read(std::move(parentJson).value_unsafe(), parentGuid);
        return AddStaticDependency(parentGuid);
    }
    return nullptr;
}
void SCookContext::AddRuntimeDependency(skr_guid_t resource)
{
    runtimeDependencies.push_back(resource);
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}
void* SCookContext::AddStaticDependency(skr_guid_t resource)
{
    staticDependencies.push_back(resource);
    return GetCookSystem()->CookOrLoad(resource);
}

SAssetRecord* SCookSystem::ImportAsset(SProject* project, ghc::filesystem::path path)
{
    std::error_code ec = {};
    if (path.is_relative())
        path = project->assetPath / path;
    auto metaPath = path;
    if(metaPath.extension() != ".meta")
        metaPath = path.string() + ".asset";
    if (!ghc::filesystem::exists(metaPath, ec))
    {
        SKR_LOG_ERROR("[SAssetRegistry::ImportAsset] meta file %s not exist", path.u8string().c_str());
        return nullptr;
    }
    auto record = SkrNew<SAssetRecord>();
    // TODO: replace file load with skr api
    record->meta = simdjson::padded_string::load(metaPath.u8string()).value_unsafe();
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    skr::json::Read(doc["guid"].value_unsafe(), record->guid);
    auto otype = doc["type"];
    if (otype.error() == simdjson::SUCCESS)
        skr::json::Read(std::move(otype).value_unsafe(), record->type);
    else
        std::memset(&record->type, 0, sizeof(skr_guid_t));
    record->path = path;
    record->project = project;
    SMutexLock lock(assetMutex);
    assets.insert(std::make_pair(record->guid, record));
    return record;
}
SAssetRecord* SCookSystem::GetAssetRecord(const skr_guid_t& guid)
{
    auto iter = assets.find(guid);
    return iter != assets.end() ? iter->second : nullptr;
}
} // namespace skd::asset