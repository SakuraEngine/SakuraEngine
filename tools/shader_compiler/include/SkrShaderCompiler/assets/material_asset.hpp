#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrRenderer/resources/material_resource.hpp"

#ifndef __meta__
#include "SkrShaderCompiler/assets/material_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
// Importers

sreflect_struct("guid" : "c0fc5581-f644-4752-bb30-0e7f652533b7")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SMaterialTypeImporter final : public SImporter
{
    uint32_t version;
    eastl::vector<skr_resource_handle_t> shader_assets;
    eastl::vector<skr_material_value_t> default_values;

    void* Import(skr::io::RAMService*, SCookContext* context) override { return nullptr; }
    void Destroy(void* resource) override { return; }
}
sregister_importer();


sreflect_struct("guid" : "b5fc88c3-0770-4332-9eda-9e283e29c7dd")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SMaterialImporter final : public SImporter
{
    uint32_t version;
    skr_resource_handle_t material_type;
    
    // stable hash for material paramters, can be used by PSO cache or other places.
    uint64_t identity[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    void* Import(skr::io::RAMService*, SCookContext* context) override { return nullptr; }
    void Destroy(void* resource) override { return; }
}
sregister_importer();

// Cookers

sreflect_struct("guid" : "816f9dd4-9a49-47e5-a29a-3bdf7241ad35")
SKR_SHADER_COMPILER_API SMaterialTypeCooker final : public SCooker
{
    bool Cook(SCookContext* ctx) override { return false; }
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_cooker("83264b35-3fde-4fff-8ee1-89abce2e445b");

sreflect_struct("guid" : "0e3b550f-cdd7-4796-a6d5-0c457e0640bd")
SKR_SHADER_COMPILER_API SMaterialCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override { return false; }
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_cooker("2efad635-b331-4fc6-8c52-2f8ca954823e");

} // namespace asset
} // namespace skd