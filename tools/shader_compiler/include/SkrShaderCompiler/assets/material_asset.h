#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrRenderer/resources/material_resource.hpp"

sreflect_struct("guid": "03c9a4d2-6b3c-4ce5-a911-567cf66e0774")
skr_material_type_asset_t : public skr_material_type_resource_t
{

};

sreflect_struct("guid": "478cedd1-a689-45bb-aea4-079bd845f86e")
sattr("rtti": true, "serialize": ["json", "bin"])
skr_material_asset_t : public skr_material_resource_t
{

};

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "c0fc5581-f644-4752-bb30-0e7f652533b7")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SMaterialTypeImporter final : public SImporter
{
    skr::string assetPath;
    skr::string entry = "main";
    skr::string target;

    void* Import(skr::io::RAMService*, SCookContext* context) override { return nullptr; }
    void Destroy(void* resource) override { return; }
}
sregister_importer();

sreflect_struct("guid" : "816f9dd4-9a49-47e5-a29a-3bdf7241ad35")
SKR_SHADER_COMPILER_API SMaterialTypeCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override { return false; }
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_cooker("83264b35-3fde-4fff-8ee1-89abce2e445b");
} // namespace asset
} // namespace skd