#pragma once
#include "SkrToolCore/module.configure.h"
#include "platform/configure.h"
#include "utils/types.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#ifndef __meta__
#include "SkrToolCore/assets/config_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
struct TOOL_CORE_API SConfigTypeInfo {
    void (*Import)(simdjson::ondemand::value&& json, void* address);
};

struct TOOL_CORE_API SConfigRegistry {
    virtual ~SConfigRegistry() SKR_NOEXCEPT = default;
    virtual void RegisterConfigType(skr_guid_t type, const SConfigTypeInfo& info) = 0;
    virtual const SConfigTypeInfo* FindConfigType(skr_guid_t type) = 0;
};
TOOL_CORE_API struct SConfigRegistry* GetConfigRegistry();

sreflect_struct("guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6")
TOOL_CORE_API SJsonConfigImporter final : public SImporter
{
    skr::string assetPath;
    skr_guid_t configType;
    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sattr("serialize" : "json")
sregister_importer();

struct sreflect TOOL_CORE_API SConfigCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("8F2DE9A2-FE05-4EB7-A07F-A973E3E92B74");

struct TOOL_CORE_API SJsonConfigImporterFactory final : public SImporterFactory {
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
    GetConfigRegistry()->RegisterConfigType(guid, typeInfo);
}
#define sregister_config_asset() sstatic_ctor(skd::asset::RegisterConfig<$T>($guid));
} // namespace asset
} // namespace skd