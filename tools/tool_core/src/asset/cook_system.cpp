#include <EASTL/shared_ptr.h>
#include <platform/filesystem.hpp>
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/project/project.hpp"
#include "platform/memory.h"
#include "platform/debug.h"
#include "platform/guid.hpp"
#include "platform/vfs.h"
#include "platform/thread.h"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "utils/io.hpp"

#include "json/reader.h"
#include "json/writer.h"
#include "binary/writer.h"

#include "tracy/Tracy.hpp"

struct TOOL_CORE_API SkrToolCoreModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
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
                ioService = skr::io::RAMService::create(&desc);
            }
        }
    }

    virtual void on_unload() override
    {
        skr_destroy_mutex(&cook_system.ioMutex);
        for (auto ioService : cook_system.ioServices)
        {
            if (ioService)
                skr::io::RAMService::destroy(ioService);
        }

        skr_destroy_mutex(&cook_system.assetMutex);
        for (auto& pair : cook_system.assets)
            SkrDelete(pair.second);
    }
    static skd::asset::SCookSystem cook_system;
};
IMPLEMENT_DYNAMIC_MODULE(SkrToolCoreModule, SkrToolCore);
skd::asset::SCookSystem SkrToolCoreModule::cook_system;

namespace skd::asset
{
SCookSystem* GetCookSystem()
{
    return &SkrToolCoreModule::cook_system;
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
        skr::task::event_t result{nullptr};
        cooking.lazy_emplace_l(guid,
         [&](SCookContext* ctx) { result = ctx->counter; },
        [&](const CookingMap::constructor& ctor) {
            jobContext = SkrNew<SCookContext>();
            ctor(guid, jobContext);
        });
        if(result) return result;
    }
    jobContext->record = GetAssetRecord(guid);
    jobContext->ioService = getIOService();
    skr::task::event_t counter;
    jobContext->counter = counter;
    auto guidName = fmt::format("Fiber{}", jobContext->record->guid);
    mainCounter.add(1);
    skr::task::schedule([jobContext]()
    {
        auto system = GetCookSystem();
        const auto metaAsset = jobContext->record;
        auto iter = system->cookers.find(metaAsset->type);
        // Trace
        ZoneScoped;
        const auto type = type::GetTypeRegistry()->get_type(metaAsset->type);
        const auto cookerTypeName = type ? type->Name() : "UnknownResource";
        const auto guidString = skr::format("Guid: {}", metaAsset->guid);
        const auto assetTypeGuidString = skr::format("TypeGuid: {}", metaAsset->type);
        const auto scopeName = skr::format("Cooker[{}].Cook", cookerTypeName);
        const auto assetString = skr::format("Asset: {}", metaAsset->path.u8string().c_str());
        ZoneName(scopeName.c_str(), scopeName.size());
        TracyMessage(guidString.c_str(), guidString.size());
        TracyMessage(assetTypeGuidString.c_str(), assetTypeGuidString.size());
        TracyMessage(assetString.c_str(), assetString.size());

        SKR_DEFER({
            auto system = GetCookSystem();
            auto guid = jobContext->record->guid;
            system->cooking.erase_if(guid, [](SCookContext* context) { SkrDelete(context); return true; });
            system->mainCounter.decrement();
        });

        // Create output dir
        auto outputPath = metaAsset->project->outputPath;
        std::error_code ec = {};
        skr::filesystem::create_directories(outputPath, ec);

        // TODO: platform dependent directory
        jobContext->outputPath = outputPath / fmt::format("{}.bin", metaAsset->guid);
        if (iter == system->cookers.end())
        {
            return;
        }

        // Cook
        jobContext->cookerVersion = iter->second->Version();
        // SKR_ASSERT(iter != system->cookers.end()); // TODO: error handling
        SKR_LOG_FMT_INFO("[CookTask] resource {} cook started!", metaAsset->guid);
        if (iter->second->Cook(jobContext))
        {
            // write resource header
            {
                SKR_LOG_FMT_INFO("[CookTask] resource {} cook finished! updating resource metas.", metaAsset->guid);
                auto headerPath = jobContext->outputPath;
                headerPath.replace_extension("rh");
                eastl::vector<uint8_t> buffer;
                struct VectorWriter
                {
                    eastl::vector<uint8_t>* buffer;
                    int write(const void* data, size_t size)
                    {
                        buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
                        return 0;
                    }
                } writer{&buffer};
                skr_binary_writer_t archive(writer);
                jobContext->WriteHeader(archive, iter->second);
                auto file = fopen(headerPath.u8string().c_str(), "wb");
                if (!file)
                {
                    SKR_LOG_FMT_ERROR("[CookTask] failed to write header file for resource {}! path: {}", metaAsset->guid, metaAsset->path.u8string());
                    return;
                }
                SKR_DEFER({ fclose(file); });
                fwrite(buffer.data(), 1, buffer.size(), file);
            }

            // write resource dependencies
            {
                SKR_LOG_FMT_INFO("[CookTask] resource {} cook finished! updating dependencies.", metaAsset->guid);
                // write dependencies
                auto dependencyPath = metaAsset->project->dependencyPath / fmt::format("{}.d", metaAsset->guid);
                skr_json_writer_t writer(2);
                writer.StartObject();
                writer.Key("importerVersion");
                writer.UInt64(jobContext->importerVersion);
                writer.Key("cookerVersion");
                writer.UInt64(jobContext->cookerVersion);
                writer.Key("files");
                writer.StartArray();
                for (auto& dep : jobContext->fileDependencies)
                {
                    auto str = dep.u8string();
                    skr::json::Write<const skr::string_view&>(&writer, {str.data(), str.size()});
                }
                writer.EndArray();
                writer.Key("dependencies");
                writer.StartArray();
                for (auto& dep : jobContext->staticDependencies)
                    skr::json::Write<const skr_resource_handle_t&>(&writer, dep);
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

#define SKR_CHECK_RESULT(result, name) \
    if (result.error() != simdjson::SUCCESS) \
    { \
        SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] " name " file parse failed! resource guid: {}, field: {}", guid); \
        return false; \
    }

skr::task::event_t SCookSystem::EnsureCooked(skr_guid_t guid)
{
    {
        skr::task::event_t result{nullptr};
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
        if (!skr::filesystem::is_regular_file(resourcePath, ec))
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] resource not exist! resource guid: {}", guid);
            return false;
        }
        if (!skr::filesystem::is_regular_file(dependencyPath, ec))
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file not exist! resource guid: {}", guid);
            return false;
        }
        auto timestamp = skr::filesystem::last_write_time(resourcePath, ec);
        if (skr::filesystem::last_write_time(metaAsset->path, ec) > timestamp)
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
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] meta file parse failed! resource guid: {}", guid);
            return false;
        }
        auto importerVersion = doc["importerVersion"].get_uint64();
        SKR_CHECK_RESULT(importerVersion, "dependency")
        auto currentImporterVersion = GetImporterRegistry()->GetImporterVersion(importerTypeGuid);
        if(importerVersion.value_unsafe() != currentImporterVersion)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] importer version changed! resource guid: {}", guid);
            return false;
        }
        if(currentImporterVersion == UINT32_MAX)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dev importer version (UINT32_MAX)! resource guid: {}", guid);
            return false;
        }
        {
            auto iter = cookers.find(metaAsset->type);
            SKR_ASSERT(iter != cookers.end());
            
            if (iter->second->Version() == UINT32_MAX)
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dev cooker version (UINT32_MAX)! resource guid: {}", guid);
                return false;
            }
            auto resourceFile = fopen(resourcePath.u8string().c_str(), "rb");
            SKR_DEFER({ fclose(resourceFile); });
            char buffer[sizeof(skr_resource_header_t)];
            fread(buffer, 0, sizeof(skr_resource_header_t), resourceFile);
            SKR_DEFER({ fclose(resourceFile); });
            struct SpanReader
            {
                gsl::span<char> data;
                size_t offset = 0;
                int read(void* dst, size_t size)
                {
                    if (offset + size > data.size())
                        return -1;
                    memcpy(dst, data.data() + offset, size);
                    offset += size;
                    return 0;
                }
            } reader = {buffer};
            skr_binary_reader_t archive{reader};
            skr_resource_header_t header;
            if(header.ReadWithoutDeps(&archive) != 0)
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] resource header read failed! resource guid: {}", guid);
                return false;
            }
            if (header.version != iter->second->Version())
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] cooker version changed! resource guid: {}", guid);
                return false;
            }
        }
        auto files = doc["files"].get_array();
        if (files.error() != simdjson::SUCCESS)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file parse failed! resource guid: {}", guid);
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
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] file not exist! resource guid: {}", guid);
                return false;
            }
            if (skr::filesystem::last_write_time(path, ec) > timestamp)
            {
                SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] file modified! resource guid: {}", guid);
                return false;
            }
        }
        auto deps = doc["dependencies"].get_array();
        if (deps.error() != simdjson::SUCCESS)
        {
            SKR_LOG_FMT_INFO("[SCookSystem::EnsureCooked] dependency file parse failed! resource guid: {}", guid);
            return false;
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
                if (skr::filesystem::last_write_time(record->path, ec) > timestamp)
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

void SCookContext::_Destroy(void* resource)
{
    if(!importer)
    {
        SKR_LOG_FMT_ERROR("[SCookContext::Cook] importer failed to load, resource {}! path: {}", record->guid, record->path.u8string());
    }
    SKR_DEFER({ SkrDelete(importer); });
    //-----import raw data
    importer->Destroy(resource);
    SKR_LOG_FMT_INFO("[SCookContext::Cook] asset freed for resource {}! path: {}", record->guid, record->path.u8string());
}

void* SCookContext::_Import()
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
            SKR_LOG_FMT_ERROR("[SCookContext::Cook] importer failed to load, resource {}! path: {}", record->guid, record->path.u8string());
            return nullptr;
        }
        importerVersion = importer->Version();
        importerType = importerTypeGuid;
        //-----import raw data
        auto rawData = importer->Import(ioService, this);
        SKR_LOG_FMT_INFO("[SCookContext::Cook] asset imported for resource {}! path: {}", record->guid, record->path.u8string());
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

skr::filesystem::path SCookContext::GetOutputPath() const
{
    return outputPath;
}

SImporter* SCookContext::GetImporter() const
{
    return importer;
}

skr_guid_t SCookContext::GetImporterType() const
{
    return importerType;
}

uint32_t SCookContext::GetImporterVersion() const
{
    return importerVersion;
}

uint32_t SCookContext::GetCookerVersion() const
{
    return cookerVersion;
}

const SAssetRecord* SCookContext::GetAssetRecord() const
{
    return record;
}

skr::filesystem::path SCookContext::AddFileDependency(const skr::filesystem::path &inPath)
{
    auto iter = std::find_if(fileDependencies.begin(), fileDependencies.end(), [&](const auto &dep) { return dep == inPath; });
    if (iter == fileDependencies.end())
        fileDependencies.push_back(inPath);
    return record->path.parent_path() / inPath;
}

void SCookContext::AddRuntimeDependency(skr_guid_t resource)
{
    auto iter = std::find_if(runtimeDependencies.begin(), runtimeDependencies.end(), [&](const auto &dep) { return dep == resource; });
    if (iter == runtimeDependencies.end())
        runtimeDependencies.push_back(resource);
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}

void SCookContext::AddSoftRuntimeDependency(skr_guid_t resource)
{
    GetCookSystem()->EnsureCooked(resource); // try launch new cook task, non blocking
}

skr::span<const skr_guid_t> SCookContext::GetRuntimeDependencies() const
{
    return skr::span<const skr_guid_t>(runtimeDependencies.data(), runtimeDependencies.size());
}

skr::span<const skr_resource_handle_t> SCookContext::GetStaticDependencies() const
{
    return skr::span<const skr_resource_handle_t>(staticDependencies.data(), staticDependencies.size());
}

const skr_resource_handle_t& SCookContext::GetStaticDependency(uint32_t index) const
{
    return staticDependencies[index];
}

uint32_t SCookContext::AddStaticDependency(skr_guid_t resource)
{
    auto iter = std::find_if(staticDependencies.begin(), staticDependencies.end(), [&](const auto &dep) { return dep.get_serialized() == resource; });
    if (iter == staticDependencies.end())
    {
        auto counter = GetCookSystem()->EnsureCooked(resource);
        if (counter) counter.wait(false);
        skr_resource_handle_t handle{resource};
        handle.resolve(false, (uint64_t)this, SKR_REQUESTER_SYSTEM);

        skr::task::wait(false, [&]
        {
            auto status = handle.get_status();
            return status == SKR_LOADING_STATUS_INSTALLED || status == SKR_LOADING_STATUS_ERROR;
        });

        staticDependencies.push_back(std::move(handle));
        return staticDependencies.size() - 1;
    }
    return (uint32_t)(staticDependencies.end() - iter);
}

SAssetRecord* SCookSystem::ImportAsset(SProject* project, skr::filesystem::path path)
{
    std::error_code ec = {};
    if (path.is_relative())
        path = project->assetPath / path;
    auto record = SkrNew<SAssetRecord>();
    // TODO: replace file load with skr api
    record->meta = simdjson::padded_string::load(path.u8string()).value_unsafe();
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