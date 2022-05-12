#include "asset/cooker.hpp"
#include "EASTL/shared_ptr.h"
#include "asset/asset_registry.hpp"
#include "asset/importer.hpp"
#include "ftl/task_counter.h"
#include "ghc/filesystem.hpp"
#include "platform/debug.h"
#include "platform/guid.h"
#include "platform/vfs.h"
#include "platform/thread.h"
#include "simdjson.h"
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "ftl/task_scheduler.h"
#include "platform/memory.h"
#include "platform/configure.h"
#include "utils/log.h"
#include "json/reader.h"
#include "json/writer.h"
#include <mutex>
#include <stdio.h>
#include "utils/io.hpp"
#include "platform/vfs.h"

namespace skd::asset
{
SCookSystem* GetCookSystem()
{
    static SCookSystem instance;
    return &instance;
}
SCookSystem::SCookSystem() noexcept
{
    skr_init_mutex(&taskMutex);
    for (auto& ioService : ioServices)
        ioService = nullptr;
}
void SCookSystem::Initialize()
{
    scheduler.Init();
}
SCookSystem::~SCookSystem() noexcept
{
    skr_destroy_mutex(&taskMutex);
    for (auto ioService : ioServices)
    {
        if (ioService) skr::io::RAMService::destroy(ioService);
    }
}

skr::io::RAMService* SCookSystem::getIOService()
{
    for (auto& ioService : ioServices)
    {
        // all used up
        if (ioService == nullptr)
        {
            skr_ram_io_service_desc_t desc = {};
            // cook system runs quick so there is no need to sleep long
            desc.sleep_time = 1;
            ioService = skr::io::RAMService::create(&desc);
            return ioService;
        }
        // find a sleep one
        else
        {
            if (ioService->get_service_status() == SKR_ASYNC_IO_SERVICE_STATUS_SLEEPING)
            {
                return ioService;
            }
            else
                continue;
        }
    }
    static uint32_t cursor = 0;
    cursor = (cursor % ioServicesMaxCount);
    return ioServices[cursor++];
}

eastl::shared_ptr<ftl::TaskCounter> SCookSystem::AddCookTask(skr_guid_t guid)
{
    SCookContext* jobContext;
    {
        SMutexLock lock(taskMutex);
        auto iter = cooking.find(guid);
        if (iter != cooking.end())
            return iter->second->counter;
        jobContext = SkrNew<SCookContext>();
        cooking.insert(std::make_pair(guid, jobContext));
    }

    jobContext->record = GetAssetRegistry()->GetAssetRecord(guid);
    jobContext->ioService = getIOService();
    auto counter = eastl::make_shared<ftl::TaskCounter>(&scheduler);
    jobContext->counter = counter;
    auto Task = +[](ftl::TaskScheduler* scheduler, void* userdata) {
        SCookContext* jobContext = (SCookContext*)userdata;
        auto metaAsset = jobContext->record;
        auto outputPath = metaAsset->project->outputPath;
        ghc::filesystem::create_directories(outputPath);
        // TODO: platform dependent directory
        jobContext->output = outputPath / fmt::format("{}.bin", metaAsset->guid);
        auto system = GetCookSystem();
        auto iter = system->cookers.find(metaAsset->type);
        SKR_ASSERT(iter != system->cookers.end()); // TODO: error handling
        iter->second->Cook(jobContext);
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
        SKR_DEFER({ fclose(file); });
        fwrite(writer.buffer.data(), 1, writer.buffer.size(), file);
    };
    auto TearDownTask = +[](ftl::TaskScheduler* scheduler, void* userdata) {
        SCookContext* jobContext = (SCookContext*)userdata;
        auto system = GetCookSystem();
        system->scheduler.WaitForCounter(jobContext->counter.get());
        auto guid = jobContext->record->guid;
        {
            SMutexLock lock(system->taskMutex);
            system->cooking.erase(guid);
        }
        SkrDelete(jobContext);
    };
    scheduler.AddTask({ Task, jobContext }, ftl::TaskPriority::Normal, counter.get());
    scheduler.AddTask({ TearDownTask, jobContext }, ftl::TaskPriority::Normal);
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
eastl::shared_ptr<ftl::TaskCounter> SCookSystem::EnsureCooked(skr_guid_t guid)
{
    {
        SMutexLock lock(taskMutex);
        auto iter = cooking.find(guid);
        if (iter != cooking.end())
            return iter->second->counter;
    }
    auto registry = GetAssetRegistry();
    auto metaAsset = registry->GetAssetRecord(guid);
    if (!metaAsset)
    {
        SKR_LOG_FMT_ERROR("[SCookSystem::EnsureCooked] resource not exist! resource guid: {}", guid);
        return nullptr;
    }
    auto resourcePath = metaAsset->project->outputPath / fmt::format("{}.bin", metaAsset->guid);
    auto dependencyPath = metaAsset->project->dependencyPath / fmt::format("{}.d", metaAsset->guid);
    auto checkUpToDate = [&]() -> bool {
        if (!ghc::filesystem::is_regular_file(resourcePath))
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] resource not exist! resource guid: {}", guid);
            return false;
        }
        if (!ghc::filesystem::is_regular_file(dependencyPath))
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file not exist! resource guid: {}", guid);
            return false;
        }
        auto timestamp = ghc::filesystem::last_write_time(resourcePath);
        if (ghc::filesystem::last_write_time(metaAsset->path) > timestamp)
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
        }

        for (auto depFile : deps.value_unsafe())
        {
            skr_guid_t depGuid;
            skr::json::Read(std::move(depFile).value_unsafe(), depGuid);
            auto record = registry->GetAssetRecord(depGuid);
            if (!record)
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file {} not exist! resource guid: {}", depGuid, guid);
                return false;
            }
            if (record->type == skr_guid_t{})
            {
                if (ghc::filesystem::last_write_time(record->path) > timestamp)
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
        scheduler.WaitForCounter(counter.get());
    SKR_UNIMPLEMENTED_FUNCTION();
    // LOAD
    return nullptr;
}

void* SCookContext::_Import()
{
    SAssetRegistry& registry = *GetAssetRegistry();
    //-----load importer
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(record->meta);
    auto importerJson = doc["importer"]; // import from asset
    if (importerJson.error() == simdjson::SUCCESS)
    {
        auto importer = GetImporterRegistry()->LoadImporter(record, std::move(importerJson).value_unsafe());
        //-----import raw data
        auto asset = registry.GetAssetRecord(importer->assetGuid);
        staticDependencies.push_back(asset->guid);
        auto rawData = importer->Import(ioService, asset);
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
} // namespace skd::asset