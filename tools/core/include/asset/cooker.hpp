#pragma once
#include "tool_configure.h"
#include "asset/asset_registry.hpp"
#include "ftl/task_counter.h"
#include "ftl/task_scheduler.h"
#include "platform/configure.h"
#include "platform/guid.h"

namespace skd::asset reflect
{
struct SCookSystem;
struct SCookContext { // context per job
    SAssetRecord* record;
    SCookSystem* system;
    ghc::filesystem::path output;
};
struct TOOL_API SCooker {
    virtual ~SCooker() {}
    virtual bool Cook(SCookContext* ctx) = 0;
    SCookSystem* system;
};
struct TOOL_API SCookSystem {
    ftl::TaskScheduler scheduler;
    void AddCookTask(SAssetRecord* metaAsset, ftl::TaskCounter* counter = nullptr);
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cookers;
};
} // namespace skd::assetreflect