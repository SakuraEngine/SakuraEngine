#pragma once
#include "EASTL/functional.h"
#include "usdtool_configure.h"
#include "asset/importer.hpp"
#include "asset/cooker.hpp"
#include "platform/configure.h"
#include "utils/hashmap.hpp"

#if !defined(__meta__) && defined(__cplusplus)
    #include "UsdTool/json_reader.generated.h"
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
    // mapping from asset path to resource
    // by default importer will resolve the path to find resource if redirector is not exist
    skr::flat_hash_map<eastl::string, skr_guid_t, eastl::string_hash<eastl::string>> redirectors;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override;
}
sregister_importer(0);
struct sreflect
USDTOOL_API SSceneCooker final : public SCooker 
{ 
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
}
sregister_cooker(0, "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A");

struct USDTOOL_API SSceneImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override { return false; } 
    skr_guid_t GetResourceType() override { return {}; }
    void CreateImporter(const SAssetRecord* record) override;
};
} // namespace sreflect
} // namespace sreflect