#pragma once
#include "SkrTool/module.configure.h"
#include "platform/configure.h"
#include "utils/hashmap.hpp"
#include "utils/types.h"
#include "asset/importer.hpp"
#include "asset/cooker.hpp"
#if !defined(__meta__) && defined(__cplusplus)
    #include "SkrTool/json_serialize.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
struct TOOL_API SConfigTypeInfo {
    void (*Import)(simdjson::ondemand::value&& json, void* address);
};
struct TOOL_API SConfigRegistry {
    skr::flat_hash_map<skr_guid_t, SConfigTypeInfo, skr::guid::hash> typeInfos;
};
TOOL_API struct SConfigRegistry* GetConfigRegistry();
struct sreflect sattr(
"guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6",
"serialize" : "json"
)
TOOL_API SJsonConfigImporter final : public SImporter
{
    skr_guid_t configType;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override;
}
sregister_importer(0);

struct sreflect

TOOL_API SConfigCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker(0, "8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74");

struct TOOL_API SJsonConfigImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override;
    skr_guid_t GetResourceType() override;
    void CreateImporter(const SAssetRecord* record) override;
};
} // namespace sreflect
} // namespace sreflect