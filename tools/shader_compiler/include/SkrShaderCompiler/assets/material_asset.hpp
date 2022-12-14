#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "material_type_asset.hpp"

#ifndef __meta__
#include "SkrShaderCompiler/assets/material_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{

sreflect_struct("guid" : "b38147b2-a5af-40c6-b2bd-185d16ca83ac")
sattr("rtti": true, "serialize" : ["json", "bin"])
skr_material_asset_t
{
    uint32_t material_type_version;

    // refers to a material type
    skr_material_type_handle_t material_type;

    // properties are mapped to shader parameter bindings (scalars, vectors, matrices, buffers, textures, etc.)
    eastl::vector<skr_material_value_t> override_values;

    // final values for options
    // options can be provided variantly by each material, if not provided, the default value will be used
    eastl::vector<skr_shader_option_instance_t> switch_values;

    // default value for options
    // options can be provided variantly at runtime, if not provided, the default value will be used
    eastl::vector<skr_shader_option_instance_t> option_defaults;
};

sreflect_struct("guid" : "b5fc88c3-0770-4332-9eda-9e283e29c7dd")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SMaterialImporter final : public SImporter
{
    skr::string jsonPath;
    
    // stable hash for material paramters, can be used by PSO cache or other places.
    uint64_t identity[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void Destroy(void* resource) override;
}
sregister_importer();

// Cookers

sreflect_struct("guid" : "0e3b550f-cdd7-4796-a6d5-0c457e0640bd")
SKR_SHADER_COMPILER_API SMaterialCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_default_cooker("2efad635-b331-4fc6-8c52-2f8ca954823e");

} // namespace asset
} // namespace skd