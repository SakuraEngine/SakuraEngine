#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "resource/resource_factory.h"

#ifndef __meta__
#include "SkrRenderer/resources/material_resource.generated.h"
#endif

using skr_material_property_name_view_t = skr::string_view;

sreflect_struct("guid": "e2c14489-3223-489a-8e30-95d2014e99f2")
sattr("blob" : true)
skr_material_value_bool_t
{
    skr_material_property_name_view_t slot_name;
    bool value;
};
GENERATED_BLOB_BUILDER(skr_material_value_bool_t)

sreflect_struct("guid": "bb5b5c8e-367c-4ec8-b4ee-60c14c212160")
sattr("blob" : true)
skr_material_value_float_t
{
    skr_material_property_name_view_t slot_name;
    float value;
};
GENERATED_BLOB_BUILDER(skr_material_value_float_t)

sreflect_struct("guid": "ce285a7f-0713-4e55-b960-be4b8022a620")
sattr("blob" : true)
skr_material_value_double_t
{
    skr_material_property_name_view_t slot_name;
    double value;
};
GENERATED_BLOB_BUILDER(skr_material_value_double_t)

sreflect_struct("guid": "7b9c85a6-292f-4bd0-85bf-6fd3dec8410a")
sattr("blob" : true)
skr_material_value_float2_t
{
    skr_material_property_name_view_t slot_name;
    skr_float2_t value;
};
GENERATED_BLOB_BUILDER(skr_material_value_float2_t)

sreflect_struct("guid": "d788b57b-65f6-490d-9fc6-4f7bc32c18ed")
sattr("blob" : true)
skr_material_value_float3_t
{
    skr_material_property_name_view_t slot_name;
    skr_float3_t value;
};
GENERATED_BLOB_BUILDER(skr_material_value_float3_t)

sreflect_struct("guid": "7b26477e-caa7-4aa6-8fb0-76f2976e23e2")
sattr("blob" : true)
skr_material_value_float4_t
{
    skr_material_property_name_view_t slot_name;
    skr_float4_t value;
};
GENERATED_BLOB_BUILDER(skr_material_value_float4_t)

sreflect_struct("guid": "31c522ce-7124-45c6-8d2d-5430aaf17e8a")
sattr("blob" : true)
skr_material_value_texture_t
{
    skr_material_property_name_view_t slot_name;
    skr_guid_t value;
};
GENERATED_BLOB_BUILDER(skr_material_value_texture_t)

sreflect_struct("guid": "7cbbb808-20d9-4bff-b72d-3c23d5b00f2b")
sattr("blob" : true)
skr_material_shader_variant
{
    // refers to a skr_shader_collection_resource_t 
    skr_guid_t shader_collection;

    // variant hash of static switches -> skr_multi_shader_resource_t
    skr_stable_shader_hash_t switch_hash;

    // static switch value selection indices, const during runtime
    skr::span<uint32_t> switch_indices; 

    // variant hash of default options -> skr_platform_shader_identifier_t
    skr_stable_shader_hash_t option_hash;

    // options value selection indices, immutable during runtime
    skr::span<uint32_t> option_indices; 
};
GENERATED_BLOB_BUILDER(skr_material_shader_variant)

sreflect_struct("guid": "e81946ee-fb88-4cde-abd5-b4ae56dbaa89") 
sattr("blob" : true)
skr_material_overrides_t
{
    skr::span<skr_material_shader_variant> switch_variants;
    skr::span<skr_material_value_bool_t> bools;
    skr::span<skr_material_value_float_t> floats;
    skr::span<skr_material_value_float2_t> float2s;
    skr::span<skr_material_value_float3_t> float3s;
    skr::span<skr_material_value_float4_t> float4s;
    skr::span<skr_material_value_double_t> doubles;
    skr::span<skr_material_value_texture_t> textures;
};
GENERATED_BLOB_BUILDER(skr_material_overrides_t)

sreflect_struct("guid" : "2efad635-b331-4fc6-8c52-2f8ca954823e")
sattr("rtti": true, "serialize" : "bin")
skr_material_resource_t
{
    uint32_t material_type_version;
    skr_material_type_handle_t material_type;

    sattr("no-rtti" : true)
    skr_blob_arena_t arena;
    
    sattr("no-rtti" : true, "arena" : "arena")
    skr_material_overrides_t overrides;

    spush_attr("no-rtti" : true, "transient": true)
    skr::vector<skr_platform_shader_identifier_t> installed_shaders;
    skr::vector<skr::string_view> shader_entries;
    skr::vector<ECGPUShaderStage> shader_stages;
    CGPURootSignatureId root_signature;
    CGPURenderPipelineId pipeline;
};

namespace skr sreflect
{
namespace resource sreflect
{
struct SKR_RENDERER_API SMaterialFactory : public SResourceFactory {
    virtual ~SMaterialFactory() = default;

    struct Root {
        SRenderDeviceId render_device = nullptr;
        skr_vfs_t* bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_threaded_service_t* aux_service = nullptr;
    };
    [[nodiscard]] static SMaterialFactory* Create(const Root& root);
    static void Destroy(SMaterialFactory* factory); 
};
} // namespace resource
} // namespace skr