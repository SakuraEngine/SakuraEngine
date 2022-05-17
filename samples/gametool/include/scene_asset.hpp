#pragma once
#include "EASTL/functional.h"
#include "gametool_configure.h"
#include "platform/guid.h"
#include "asset/importer.hpp"
#include "asset/cooker.hpp"
#include "platform/configure.h"
#include "utils/hashmap.hpp"

namespace game reflect
{
namespace asset reflect
{
using namespace skd::asset;
struct reflect attr(
"guid" : "4F0E4239-A07F-4F48-B54F-FBF406C60DC3",
"serialize" : "json",
"importer" : "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A"
)
TOOL_API SSceneImporter final : public SImporter
{
    eastl::string sceneRoot;
    // mapping from asset path to resource
    // by default importer will resolve the path to find resource if redirector is not exist
    skr::flat_hash_map<eastl::string, skr_guid_t, eastl::string_hash<eastl::string>> redirectors;
    using SImporter::SImporter;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override;
};

struct reflect attr(
"cooker" : "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A"
)
TOOL_API SSceneCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
};

struct TOOL_API SSceneImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override { return false; }
    skr_guid_t GetResourceType() override { return {}; }
    SImporter* CreateImporter(const SAssetRecord* record) override { return nullptr; }
};
} // namespace reflect
} // namespace reflect