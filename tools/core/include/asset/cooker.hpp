#pragma once
#include "asset/asset_registry.hpp"
#include "utils/hashmap.hpp"
#include "platform/configure.h"
#include "platform/guid.h"
#include "tool_configure.h"

namespace skd::asset reflect
{
struct SCookSystem;
struct TOOL_API SCooker {
    virtual ~SCooker() {}
    virtual bool Cook(SAssetRecord* record);
    SCookSystem* system;
};
struct SCookSystem {
    SAssetRegistry* registry;
    ghc::filesystem::path outputPath;
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cooker;
};
} // namespace skd::assetreflect