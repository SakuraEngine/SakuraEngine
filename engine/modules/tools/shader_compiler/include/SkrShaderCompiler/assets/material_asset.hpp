#pragma once
#include "SkrCore/memory/sp.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrShaderCompiler/assets/material_type_asset.hpp" // IWYU pragma: export
#include "SkrShaderCompiler/assets/material_asset.generated.h" // IWYU pragma: export

namespace skr
{

struct [[sattr(
    guid = "b38147b2-a5af-40c6-b2bd-185d16ca83ac"
    serde = @enable
)]] SKR_SHADER_COMPILER_API MaterialAsset
{
    ~MaterialAsset();

    uint32_t material_type_version;
    // refers to a material type
    AsyncResource<MaterialTypeResource> material_type;
    // properties are mapped to shader parameter bindings (scalars, vectors, matrices, buffers, textures, etc.)
    Vector<MaterialValue> override_values;
    // final values for options
    // options can be provided variantly by each material, if not provided, the default value will be used
    Vector<ShaderOptionInstance> switch_values;
    // default value for options
    // options can be provided variantly at runtime, if not provided, the default value will be used
    Vector<ShaderOptionInstance> option_defaults;
};

struct [[sattr(
    guid = "b5fc88c3-0770-4332-9eda-9e283e29c7dd"
    serde = @enable
)]] SKR_SHADER_COMPILER_API MaterialImporter final : public Importer
{
    MaterialImporter();
    ~MaterialImporter();

    String jsonPath;

    // stable hash for material paramters, can be used by PSO cache or other places.
    uint64_t identity[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    [[sattr(serde = @disable)]]
    skr::SP<MaterialAsset> asset = nullptr;

    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

// Cookers

struct [[sattr(
    guid = "0e3b550f-cdd7-4796-a6d5-0c457e0640bd"
)]] SKR_SHADER_COMPILER_API MaterialCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
};

} // namespace skr