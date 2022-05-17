#pragma once
#include "tool_configure.h"
#include "platform/guid.h"
#include "asset/importer.hpp"
#include "asset/cooker.hpp"
#include "platform/configure.h"
#include "utils/hashmap.hpp"

namespace skd reflect
{
namespace asset reflect
{
struct TOOL_API SConfigTypeInfo {
    void (*Import)(simdjson::ondemand::value&& json, void* address);
};
struct TOOL_API SConfigRegistry {
    skr::flat_hash_map<skr_guid_t, SConfigTypeInfo, skr::guid::hash> typeInfos;
};
TOOL_API struct SConfigRegistry* GetConfigRegistry();
struct reflect attr(
"guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6",
"serialize" : "json",
"importer" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74"
)
TOOL_API SJsonConfigImporter final : public SImporter
{
    skr_guid_t configType;
    using SImporter::SImporter;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override;
};

struct reflect attr(
"cooker" : "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74"
)
TOOL_API SConfigCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
};

struct TOOL_API SJsonConfigImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override;
    skr_guid_t GetResourceType() override;
    SImporter* CreateImporter(const SAssetRecord* record) override;
};
} // namespace reflect
} // namespace reflect