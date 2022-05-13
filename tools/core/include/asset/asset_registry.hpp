#pragma once
#include "tool_configure.h"
#include "platform/guid.h"
#include "ghc/filesystem.hpp"
#include "simdjson.h"
#include "utils/hashmap.hpp"
#include "EASTL/vector.h"
#include "platform/thread.h"

struct skr_vfs_t;
namespace skd
{
using namespace skr;
namespace asset
{
TOOL_API struct SAssetRegistry* GetAssetRegistry();
struct SProject {
    ghc::filesystem::path assetPath;
    ghc::filesystem::path outputPath;
    ghc::filesystem::path dependencyPath;
    skr_vfs_t* vfs = nullptr;
    ~SProject() noexcept;
};

struct SAssetRecord {
    skr_guid_t guid;
    skr_guid_t type;
    ghc::filesystem::path path;
    SProject* project;
    simdjson::padded_string meta;
};

struct TOOL_API SAssetRegistry {
    SAssetRegistry();
    ~SAssetRegistry();
    SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    SAssetRecord* ImportAsset(SProject* project, ghc::filesystem::path path);
    eastl::vector<SAssetRecord*> PullChangedAssets();
    bool HasChangedAssets();
    void AddProject(SProject* project);
    eastl::vector<SProject*> projects;
    skr::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash> assets;
    SMutex mutex;
    eastl::vector<SAssetRecord*> changedAssets;
    skr::flat_hash_map<skr_guid_t, struct SImporterFactory*, skr::guid::hash> importerFactories;
};
} // namespace asset
} // namespace skd