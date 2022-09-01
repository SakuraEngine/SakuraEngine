#include "asset/cooker.hpp"
#include "EASTL/shared_ptr.h"
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
void SCookSystem::Initialize()
{
    scheduler = SkrNew<ftl::TaskScheduler>();
    mainCounter = SkrNew<ftl::TaskCounter>(scheduler);
    ftl::TaskSchedulerInitOptions options = {};
    options.Behavior = ftl::EmptyQueueBehavior::Sleep;
    scheduler->Init(options);
}
void SCookSystem::Shutdown()
{
    SkrDelete(mainCounter);
    SkrDelete(scheduler);
    scheduler = nullptr;
    mainCounter = nullptr;
}
ftl::TaskScheduler& SCookSystem::GetScheduler()
{
    return *scheduler;
}
SCookSystem::~SCookSystem() noexcept
{
    SKR_ASSERT(scheduler == nullptr);
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
    scheduler->WaitForCounter(mainCounter);
}

#include <atomic>
skr::io::RAMService* SCookSystem::getIOService()
{
    SMutexLock lock(ioMutex);
    static std::atomic_uint32_t cursor = 0;
    cursor = (cursor % ioServicesMaxCount);
    return ioServices[cursor++];
}

eastl::shared_ptr<ftl::TaskCounter> SCookSystem::AddCookTask(skr_guid_t guid)
{
    SCookContext* jobContext;
    {
        eastl::shared_ptr<ftl::TaskCounter> result;
        cooking.lazy_emplace_l(
        guid, [&](SCookContext* ctx) { result = ctx->counter; },
        [&](const CookingMap::constructor& ctor) {
            jobContext = SkrNew<SCookContext>();
            ctor(guid, jobContext);
        });
        if (result)
            return result;
    }

    jobContext->record = GetAssetRecord(guid);
    jobContext->ioService = getIOService();
    auto counter = eastl::make_shared<ftl::TaskCounter>(scheduler);
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
    };
    auto TearDown = +[](void* userdata) {
        SCookContext* jobContext = (SCookContext*)userdata;
        auto system = GetCookSystem();
        auto guid = jobContext->record->guid;
        system->cooking.erase_if(guid, [](SCookContext* context) { SkrDelete(context); return true; });
        system->mainCounter->Decrement();
    };
    mainCounter->Add(1);
    auto guidName = fmt::format("Fiber{}", jobContext->record->guid);
    const ftl::Task task = { Task, jobContext, TearDown }; 
    scheduler->AddTask(task, ftl::TaskPriority::High, counter.get() FTL_TASK_NAME(, guidName.c_str()));
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
        eastl::shared_ptr<ftl::TaskCounter> result;
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
        scheduler->WaitForCounter(counter.get());
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
    if (path.is_relative())
        path = project->assetPath / path;
    auto metaPath = path;
    if(metaPath.extension() != ".meta")
        metaPath = path.string() + ".asset";
    if (!ghc::filesystem::exists(metaPath))
    {
        SKR_LOG_ERROR("[SAssetRegistry::ImportAsset] meta file %s not exist", path.u8string().c_str());
        return nullptr;
    }
    auto record = SkrNew<SAssetRecord>();
    // TODO: replace file load with skr api
    record->meta = simdjson::padded_string::load(metaPath.u8string());
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