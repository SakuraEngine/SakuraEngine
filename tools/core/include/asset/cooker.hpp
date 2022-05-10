#pragma once
#include "asset/asset_registry.hpp"
#include "ftl/task_scheduler.h"
#include "utils/hashmap.hpp"
#include "platform/configure.h"
#include "platform/guid.h"
#include "tool_configure.h"
#include "EASTL/vector.h"

namespace skd::asset reflect
{
struct SCookSystem;
struct SProject {
    ghc::filesystem::path outputPath;
};
struct SCookContext { // context per job
    size_t projectIndex;
    SAssetRecord* record;
};
struct TOOL_API SCooker {
    virtual ~SCooker() {}
    virtual bool Cook(SCookContext* ctx);
    SCookSystem* system;
};
struct SCookSystem {
    SAssetRegistry* registry;
    ftl::TaskScheduler scheduler;
    eastl::vector<SProject> projects;
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cooker;
};
} // namespace skd::assetreflect