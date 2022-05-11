#pragma once
#include "tool_configure.h"
#include "platform/guid.h"
#include "ghc/filesystem.hpp"
#include "simdjson.h"
#include "utils/hashmap.hpp"
#include "EASTL/vector.h"

namespace skd
{
using namespace skr;
namespace asset
{
TOOL_API struct SAssetRegistry* GetAssetRegistry();
struct SProject {
    ghc::filesystem::path assetPath;
    ghc::filesystem::path outputPath;
};

struct SAssetRecord {
    skr_guid_t guid;
    skr_guid_t type;
    ghc::filesystem::path path;
    SProject* project;
    simdjson::padded_string meta;
};

struct TOOL_API SAssetRegistry {
    ~SAssetRegistry();
    SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    SAssetRecord* ImportAsset(SProject* project, ghc::filesystem::path path);
    void AddProject(SProject* project);
    eastl::vector<SProject*> projects;
    skr::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash> assets;
    skr::flat_hash_map<skr_guid_t, struct SImporterFactory*, skr::guid::hash> importerFactories;
};
} // namespace asset
} // namespace skd