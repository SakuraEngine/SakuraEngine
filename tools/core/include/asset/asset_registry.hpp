#pragma once
#include "platform/guid.h"
#include "ghc/filesystem.hpp"
#include "simdjson.h"
#include "phmap.h"

namespace skd
{
using namespace skr;
namespace asset
{
struct SAssetRecord {
    skr_guid_t guid;
    ghc::filesystem::path path;
    simdjson::padded_string meta;
};
struct SAssetRegistry {
    ~SAssetRegistry();
    SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    SAssetRecord* ImportAsset(ghc::filesystem::path path);
    void* ImportResource(const skr_guid_t& guid, skr_guid_t& resourceType);
    phmap::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash> assets;
};
} // namespace asset
} // namespace skd