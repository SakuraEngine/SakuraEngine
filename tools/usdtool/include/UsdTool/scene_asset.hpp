#pragma once
#include <EASTL/functional.h>
#include "platform/configure.h"
#include "containers/hashmap.hpp"
#include "UsdTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#ifndef __meta__
    #include "UsdTool/scene_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
struct sreflect sattr(
    "guid" : "4F0E4239-A07F-4F48-B54F-FBF406C60DC3",
    "serialize" : "json"
)
USDTOOL_API SSceneImporter final : public SImporter
{
    eastl::string assetPath;
    // mapping from asset path to resource
    // by default importer will resolve the path to find resource if redirector is not exist
    skr::flat_hash_map<eastl::string, skr_guid_t, eastl::string_hash<eastl::string>> redirectors;
    void* Import(skr::io::RAMService*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();

struct sreflect
USDTOOL_API SSceneCooker final : public SCooker 
{ 
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker("EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A");

struct USDTOOL_API SSceneImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override { SKR_UNIMPLEMENTED_FUNCTION(); return false; } 
    skr_guid_t GetResourceType() override { SKR_UNIMPLEMENTED_FUNCTION(); return {}; }
    void CreateImporter(const SAssetRecord* record) override;
};
} // namespace asset
} // namespace skd