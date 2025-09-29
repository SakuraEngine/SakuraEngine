#pragma once
#include "SkrRenderer/fwd_types.h"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/string.hpp"
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrShaderCompiler/assets/material_type_asset.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(
    guid = "329fddb1-73a6-4b4b-8f9f-f4acca58a6e5"
    serde = @enable
)]] MaterialTypeAsset
{
    uint32_t version;

    // shader assets
    Vector<MaterialPass> passes;

    // properties are mapped to shader parameter bindings (scalars, vectors, matrices, buffers, textures, etc.)
    Vector<MaterialProperty> properties;

    // default value for options
    // options can be provided variantly by each material, if not provided, the default value will be used
    Vector<ShaderOptionInstance> switch_defaults;

    // default value for options
    // options can be provided variantly at runtime, if not provided, the default value will be used
    Vector<ShaderOptionInstance> option_defaults;

    VertexLayoutId vertex_type;
};

struct [[sattr(
    guid = "c0fc5581-f644-4752-bb30-0e7f652533b7"
    serde = @enable
)]] SKR_SHADER_COMPILER_API MaterialTypeImporter final : public Importer
{
    String jsonPath;

    void* Import(skr::io::IRAMService*, CookContext* context) override;
    void Destroy(void* resource) override;
};

struct [[sattr(
    guid = "816f9dd4-9a49-47e5-a29a-3bdf7241ad35"
)]] SKR_SHADER_COMPILER_API MaterialTypeCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
};

} // namespace skr