#pragma once
#include "SkrTool/module.configure.h"
#include "platform/configure.h"
#include "utils/hashmap.hpp"
#include "utils/types.h"
#include "asset/importer.hpp"
#include "asset/cooker.hpp"

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

sreflect_struct("guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6")
TOOL_API SJsonConfigImporter final : public SImporter
{
    eastl::string assetPath;
    skr_guid_t configType;
    void* Import(skr::io::RAMService*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sattr("serialize" : "json")
sregister_importer();

struct sreflect TOOL_API SConfigCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74");

struct TOOL_API SJsonConfigImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override;
    skr_guid_t GetResourceType() override;
    void CreateImporter(const SAssetRecord* record) override;
};


template<class T>
inline static void RegisterConfig(skr_guid_t guid)
{
    SConfigTypeInfo typeInfo {
        +[](simdjson::ondemand::value&& json, void* address)
        {
            skr::json::Read(std::move(json), *static_cast<T*>(address));
        }
    };
    GetConfigRegistry()->typeInfos.insert(std::make_pair(guid, typeInfo));
}
#define sregister_config_asset() sstatic_ctor(skd::asset::RegisterConfig<$T>($guid));
} // namespace asset
} // namespace skd