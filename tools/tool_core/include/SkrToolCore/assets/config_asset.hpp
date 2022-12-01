#pragma once
#include "SkrToolCore/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "utils/types.h"
#include "platform/guid.hpp"
#ifndef __meta__
#include "SkrToolCore/assets/config_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
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
} // namespace asset
} // namespace skd