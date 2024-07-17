#pragma once
#include "SkrBase/config.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRT/resource/resource_factory.h"
#include "SkrGraphics/cgpux.h"

#ifndef __meta__
    #include "SkrRenderer/resources/material_resource.generated.h" // IWYU pragma: export
#endif

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)

namespace skr
{
namespace renderer
{
using MaterialPropertyNameView = skr::SerializeConstString;

sreflect_struct("guid": "e2c14489-3223-489a-8e30-95d2014e99f2")
sattr("serde" : "bin")
MaterialValueBool {
    MaterialPropertyNameView slot_name;
    bool                     value;
};

sreflect_struct("guid": "bb5b5c8e-367c-4ec8-b4ee-60c14c212160")
sattr("serde" : "bin")
MaterialValueFloat {
    MaterialPropertyNameView slot_name;
    float                    value;
};

sreflect_struct("guid": "ce285a7f-0713-4e55-b960-be4b8022a620")
sattr("serde" : "bin")
MaterialValueDouble {
    MaterialPropertyNameView slot_name;
    double                   value;
};

sreflect_struct("guid": "7b9c85a6-292f-4bd0-85bf-6fd3dec8410a")
sattr("serde" : "bin")
MaterialValueFloat2 {
    MaterialPropertyNameView slot_name;
    skr_float2_t             value;
};

sreflect_struct("guid": "d788b57b-65f6-490d-9fc6-4f7bc32c18ed")
sattr("serde" : "bin")
MaterialValueFloat3 {
    MaterialPropertyNameView slot_name;
    skr_float3_t             value;
};

sreflect_struct("guid": "7b26477e-caa7-4aa6-8fb0-76f2976e23e2")
sattr("serde" : "bin")
MaterialValueFloat4 {
    MaterialPropertyNameView slot_name;
    skr_float4_t             value;
};

sreflect_struct("guid": "31c522ce-7124-45c6-8d2d-5430aaf17e8a")
sattr("serde" : "bin")
MaterialValueTexture {
    MaterialPropertyNameView slot_name;
    skr_guid_t               value;
};

sreflect_struct("guid": "760d78ba-c42c-49fa-9164-6968e7693461")
sattr("serde" : "bin")
MaterialValueSampler {
    MaterialPropertyNameView slot_name;
    skr_guid_t               value;
};

sreflect_struct("guid": "7cbbb808-20d9-4bff-b72d-3c23d5b00f2b")
sattr("serde" : "bin")
MaterialShaderVariant {
    // refers to a skr_shader_collection_resource_t
    skr_guid_t shader_collection;

    // variant hash of static switches -> skr_multi_shader_resource_t
    skr_stable_shader_hash_t switch_hash;

    // static switch value selection indices, const during runtime
    skr::SerializeConstVector<uint32_t> switch_indices;

    // variant hash of default options -> skr_platform_shader_identifier_t
    skr_stable_shader_hash_t option_hash;

    // options value selection indices, immutable during runtime
    skr::SerializeConstVector<uint32_t> option_indices;
};

sreflect_struct("guid": "e81946ee-fb88-4cde-abd5-b4ae56dbaa89") 
sattr("serde" : "bin")
MaterialOverrides {
    skr::SerializeConstVector<MaterialShaderVariant> switch_variants;
    skr::SerializeConstVector<MaterialValueBool>     bools;
    skr::SerializeConstVector<MaterialValueFloat>    floats;
    skr::SerializeConstVector<MaterialValueFloat2>   float2s;
    skr::SerializeConstVector<MaterialValueFloat3>   float3s;
    skr::SerializeConstVector<MaterialValueFloat4>   float4s;
    skr::SerializeConstVector<MaterialValueDouble>   doubles;
    skr::SerializeConstVector<MaterialValueTexture>  textures;
    skr::SerializeConstVector<MaterialValueSampler>  samplers;
};

sreflect_struct("guid" : "2efad635-b331-4fc6-8c52-2f8ca954823e")
sattr("serde" : "bin")
MaterialResource {
    uint32_t                   material_type_version;
    skr_material_type_handle_t material_type;

    MaterialOverrides overrides;

    typedef struct installed_shader {
        skr_platform_shader_identifier_t identifier;
        skr::StringView                  entry;
        ECGPUShaderStage                 stage;
    } installed_shader;

    typedef struct installed_pass {
        skr::String                   name;
        skr::Vector<installed_shader> shaders;
        ESkrInstallStatus             status;
        CGPURootSignatureId           root_signature;
        skr_pso_map_key_id            key;
        CGPURenderPipelineId          pso;
        CGPUXBindTableId              bind_table;
    } installed_pass;

    sattr("serde": "disable")
    skr::Vector<installed_pass> installed_passes;
};

struct SKR_RENDERER_API SMaterialFactory : public resource::SResourceFactory {
    virtual ~SMaterialFactory() = default;

    struct Root {
        CGPUDeviceId          device       = nullptr;
        skr_shader_map_id     shader_map   = nullptr;
        skr_vfs_t*            bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service  = nullptr;
        skr_job_queue_id      job_queue    = nullptr;
    };
    [[nodiscard]] static SMaterialFactory* Create(const Root& root);
    static void                            Destroy(SMaterialFactory* factory);
};
} // namespace renderer
} // namespace skr