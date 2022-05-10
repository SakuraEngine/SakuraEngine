#pragma once
#include "tool_configure.h"
#include "platform/guid.h"
#include "ghc/filesystem.hpp"
#include "simdjson.h"
#include "utils/hashmap.hpp"

namespace skd
{
using namespace skr;
namespace asset
{
TOOL_API struct SAssetRegistry* GetAssetRegistry();

struct SAssetRecord {
    skr_guid_t guid;
    ghc::filesystem::path path;
    simdjson::padded_string meta;
};
struct TOOL_API SAssetRegistry {
    ~SAssetRegistry();
    SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    SAssetRecord* ImportAsset(ghc::filesystem::path path);
    void* ImportResource(const skr_guid_t& guid, skr_guid_t& resourceType);
    skr::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash> assets;
    skr::flat_hash_map<skr_guid_t, struct SImporterFactory*, skr::guid::hash> importerFactories;
};
} // namespace asset
} // namespace skd