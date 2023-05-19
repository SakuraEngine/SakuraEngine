#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "resource/resource_factory.h"
#include "cgpu/cgpux.h"

#ifndef __meta__
#include "SkrRenderer/resources/material_resource.generated.h"
#endif

namespace skr sreflect
{
namespace renderer sreflect
{
using MaterialPropertyNameView = skr::string_view;

sreflect_struct("guid": "e2c14489-3223-489a-8e30-95d2014e99f2")
sattr("blob" : true)
MaterialValueBool
{
    MaterialPropertyNameView slot_name;
    bool value;
};
GENERATED_BLOB_BUILDER(MaterialValueBool)

sreflect_struct("guid": "bb5b5c8e-367c-4ec8-b4ee-60c14c212160")
sattr("blob" : true)
MaterialValueFloat
{
    MaterialPropertyNameView slot_name;
    float value;
};
GENERATED_BLOB_BUILDER(MaterialValueFloat)

sreflect_struct("guid": "ce285a7f-0713-4e55-b960-be4b8022a620")
sattr("blob" : true)
MaterialValueDouble
{
    MaterialPropertyNameView slot_name;
    double value;
};
GENERATED_BLOB_BUILDER(MaterialValueDouble)

sreflect_struct("guid": "7b9c85a6-292f-4bd0-85bf-6fd3dec8410a")
sattr("blob" : true)
MaterialValueFloat2
{
    MaterialPropertyNameView slot_name;
    skr_float2_t value;
};
GENERATED_BLOB_BUILDER(MaterialValueFloat2)

sreflect_struct("guid": "d788b57b-65f6-490d-9fc6-4f7bc32c18ed")
sattr("blob" : true)
MaterialValueFloat3
{
    MaterialPropertyNameView slot_name;
    skr_float3_t value;
};
GENERATED_BLOB_BUILDER(MaterialValueFloat3)

sreflect_struct("guid": "7b26477e-caa7-4aa6-8fb0-76f2976e23e2")
sattr("blob" : true)
MaterialValueFloat4
{
    MaterialPropertyNameView slot_name;
    skr_float4_t value;
};
GENERATED_BLOB_BUILDER(MaterialValueFloat4)

sreflect_struct("guid": "31c522ce-7124-45c6-8d2d-5430aaf17e8a")
sattr("blob" : true)
MaterialValueTexture
{
    MaterialPropertyNameView slot_name;
    skr_guid_t value;
};
GENERATED_BLOB_BUILDER(MaterialValueTexture)

sreflect_struct("guid": "760d78ba-c42c-49fa-9164-6968e7693461")
sattr("blob" : true)
MaterialValueSampler
{
    MaterialPropertyNameView slot_name;
    skr_guid_t value;
};
GENERATED_BLOB_BUILDER(MaterialValueSampler)

sreflect_struct("guid": "7cbbb808-20d9-4bff-b72d-3c23d5b00f2b")
sattr("blob" : true)
MaterialShaderVariant
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
GENERATED_BLOB_BUILDER(MaterialShaderVariant)

sreflect_struct("guid": "e81946ee-fb88-4cde-abd5-b4ae56dbaa89") 
sattr("blob" : true)
MaterialOverrides
{
    skr::span<MaterialShaderVariant> switch_variants;
    skr::span<MaterialValueBool> bools;
    skr::span<MaterialValueFloat> floats;
    skr::span<MaterialValueFloat2> float2s;
    skr::span<MaterialValueFloat3> float3s;
    skr::span<MaterialValueFloat4> float4s;
    skr::span<MaterialValueDouble> doubles;
    skr::span<MaterialValueTexture> textures;
    skr::span<MaterialValueSampler> samplers;
};
GENERATED_BLOB_BUILDER(MaterialOverrides)

sreflect_struct("guid" : "2efad635-b331-4fc6-8c52-2f8ca954823e")
sattr("rtti": true, "serialize" : "bin")
MaterialResource
{
    uint32_t material_type_version;
    skr_material_type_handle_t material_type;

    sattr("no-rtti" : true)
    skr_blob_arena_t arena;
    
    sattr("no-rtti" : true, "arena" : "arena")
    skr_material_overrides_t overrides;

    typedef struct installed_shader {
        skr_platform_shader_identifier_t identifier;
        skr::string_view entry;
        ECGPUShaderStage stage;
    } installed_shader;

    typedef struct installed_pass {
        skr::string name;
        skr::vector<installed_shader> shaders;
        ESkrInstallStatus status;
        CGPURootSignatureId root_signature;
        struct skr_pso_map_key_t* key;
        CGPURenderPipelineId pso;
        CGPUXBindTableId bind_table;
    } installed_pass;

    spush_attr("no-rtti" : true, "transient": true)
    skr::vector<installed_pass> installed_passes;
};

struct SKR_RENDERER_API SMaterialFactory : public resource::SResourceFactory {
    virtual ~SMaterialFactory() = default;

    struct Root {
        CGPUDeviceId device = nullptr;
        skr_shader_map_id shader_map = nullptr;
        skr_vfs_t* bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_threaded_service_t* aux_service = nullptr;
    };
    [[nodiscard]] static SMaterialFactory* Create(const Root& root);
    static void Destroy(SMaterialFactory* factory); 
};
} // namespace resource
} // namespace skr