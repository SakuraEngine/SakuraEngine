#pragma once
#include "SkrBase/config.h"
#include "SkrToolCore/asset/importer.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrGuid/guid.hpp"

#ifndef __meta__
    #include "SkrShaderCompiler/assets/material_type_asset.generated.h" // IWYU pragma: export
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "329fddb1-73a6-4b4b-8f9f-f4acca58a6e5")
sattr("serialize" : ["json", "bin"])
skr_material_type_asset_t {
    uint32_t version;

    // shader assets
    skr::Vector<skr_material_pass_t> passes;

    // properties are mapped to shader parameter bindings (scalars, vectors, matrices, buffers, textures, etc.)
    skr::Vector<skr_material_property_t> properties;

    // default value for options
    // options can be provided variantly by each material, if not provided, the default value will be used
    skr::Vector<skr_shader_option_instance_t> switch_defaults;

    // default value for options
    // options can be provided variantly at runtime, if not provided, the default value will be used
    skr::Vector<skr_shader_option_instance_t> option_defaults;

    skr_vertex_layout_id vertex_type;
};

sreflect_struct("guid" : "c0fc5581-f644-4752-bb30-0e7f652533b7")
sattr("serialize" : "json")
SKR_SHADER_COMPILER_API SMaterialTypeImporter final : public SImporter {
    skr::String jsonPath;

    void* Import(skr_io_ram_service_t*, SCookContext* context) override;
    void  Destroy(void* resource) override;
};

sreflect_struct("guid" : "816f9dd4-9a49-47e5-a29a-3bdf7241ad35")
SKR_SHADER_COMPILER_API SMaterialTypeCooker final : public SCooker {
    bool     Cook(SCookContext* ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
};

} // namespace asset sreflect
} // namespace skd sreflect