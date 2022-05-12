#include "asset/asset_registry.hpp"
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
    if(vfs) skr_free_vfs(vfs);    
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
            ImportAsset(project, entry.path());
        }
    }
    ghc::filesystem::create_directories(project->outputPath);
    ghc::filesystem::create_directories(project->dependencyPath);
    // TODO: asset watcher
}
SAssetRegistry::~SAssetRegistry()
{
    for (auto& pair : assets)
        SkrDelete(pair.second);
    for (auto& project : projects)
        SkrDelete(project);
}
} // namespace skd::asset