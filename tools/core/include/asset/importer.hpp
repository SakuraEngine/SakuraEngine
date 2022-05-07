#pragma once
#include "platform/guid.h"
#include "resource/resource_header.h"
#include "platform/configure.h"
#include "simdjson.h"
#include "asset_registry.hpp"

namespace skd reflect
{
using namespace skr;
namespace asset reflect
{
struct reflect attr(
"guid" : "76044661-E2C9-43A7-A4DE-AEDD8FB5C847", 
"serialize" : true
)
TOOL_API SImporter
{
    skr_guid_t assetGuid;
    SImporter(skr_guid_t inAsset)
        : assetGuid(inAsset)
    {
    }
    virtual ~SImporter() {}
    virtual void* Import(const SAssetRecord* record) = 0;
};
struct SImporterFactory {
    virtual bool CanImport(const SAssetRecord* record) = 0;
    virtual skr_guid_t GetResourceType() = 0;
    virtual SImporter* CreateImporter(const SAssetRecord* record) = 0;
    virtual SImporter* LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object) = 0;
};
} // namespace reflect
} // namespace reflect