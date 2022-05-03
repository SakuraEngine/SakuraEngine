#pragma once
#include "asset/asset_registry.hpp"
#include "platform/guid.h"
#include "utils/path.hpp"
#include "simdjson.h"

namespace skd
{
using namespace skr;
namespace asset
{
struct SAssetRecord {
    skr_guid_t guid;
    SPath path;
    simdjson::ondemand::object meta;
};
struct SAssetRegistry {
    const SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    void* ImportResource(const skr_guid_t& guid, skr_guid_t& resourceType);
};
} // namespace asset
} // namespace skd