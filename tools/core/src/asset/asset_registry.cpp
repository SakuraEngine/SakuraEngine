#include "asset/asset_registry.hpp"
#include "EASTL/vector.h"
#include "asset/cooker.hpp"
#include "ftl/task.h"
#include "ftl/task_scheduler.h"
#include "ghc/filesystem.hpp"
#include "platform/debug.h"
#include "platform/guid.h"
#include "simdjson.h"
#include "utils/log.h"
#include "platform/memory.h"
#include "utils/defer.hpp"
#include "json/reader.h"
#include "platform/vfs.h"

namespace skd::asset
{
SProject::~SProject() noexcept
{
    if (vfs) skr_free_vfs(vfs);
}

TOOL_API SAssetRegistry* GetAssetRegistry()
{
    static SAssetRegistry registry;
    return &registry;
}
} // namespace skd::asset

namespace skd::asset
{
SAssetRecord* SAssetRegistry::ImportAsset(SProject* project, ghc::filesystem::path path)
{
    if (path.is_relative())
        path = project->assetPath / path;
    auto metaPath = path;
    metaPath.replace_extension(".meta");
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
    SMutexLock lock(mutex);
    assets.insert(std::make_pair(record->guid, record));
    return record;
}
SAssetRecord* SAssetRegistry::GetAssetRecord(const skr_guid_t& guid)
{
    auto iter = assets.find(guid);
    return iter != assets.end() ? iter->second : nullptr;
}
void SAssetRegistry::AddProject(SProject* project)
{
    projects.push_back(project);
    ghc::filesystem::recursive_directory_iterator iter(project->assetPath);
    // discover assets.
    // TODO: async?
    eastl::vector<ghc::filesystem::path> paths;
    for (auto& entry : iter)
    {
        if (entry.is_regular_file())
        {
            auto path = entry.path();
            if (path.extension() == ".meta")
            {
                path.replace_extension();
                if (!path.has_extension()) // skip asset meta
                    continue;
            }
            else
            {
                auto metaPath = path;
                metaPath.replace_extension(".meta");
                if (!ghc::filesystem::exists(metaPath))
                    continue;
            }
            paths.push_back(*iter);
        }
    }
    SKR_LOG_INFO("Project dir scan finished.");
    using iter_t = typename decltype(paths)::iterator;
    skd::asset::ParallelFor(paths.begin(), paths.end(), 20,
    [&](iter_t begin, iter_t end) {
        for (auto i = begin; i != end; ++i)
            ImportAsset(project, *i);
    });
    SKR_LOG_INFO("Project asset import finished.");
    ghc::filesystem::create_directories(project->outputPath);
    ghc::filesystem::create_directories(project->dependencyPath);
    // TODO: asset watcher
}
SAssetRegistry::SAssetRegistry()
{
    skr_init_mutex(&mutex);
}
SAssetRegistry::~SAssetRegistry()
{
    skr_destroy_mutex(&mutex);
    for (auto& pair : assets)
        SkrDelete(pair.second);
    for (auto& project : projects)
        SkrDelete(project);
}
} // namespace skd::asset