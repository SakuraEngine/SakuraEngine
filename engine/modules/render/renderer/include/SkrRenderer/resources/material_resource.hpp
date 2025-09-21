#pragma once
#include "SkrBase/config.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRuntime/resource/resource_factory.h"
#include "SkrGraphics/cgpux.h"
#include "SkrRenderer/graphics/gpu_table.hpp"
#include "SkrRenderer/resources/material_resource.generated.h" // IWYU pragma: export

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)

namespace skr
{
using MaterialPropertyNameView = skr::SerializeConstString;

struct [[sattr(guid = "e2c14489-3223-489a-8e30-95d2014e99f2" serde = @enable)]]
MaterialValueBool
{
    MaterialPropertyNameView slot_name;
    bool value;
};

struct [[sattr(guid = "bb5b5c8e-367c-4ec8-b4ee-60c14c212160" serde = @enable)]]
MaterialValueFloat
{
    MaterialPropertyNameView slot_name;
    float value;
};

struct [[sattr(guid = "ce285a7f-0713-4e55-b960-be4b8022a620" serde = @enable)]]
MaterialValueDouble
{
    MaterialPropertyNameView slot_name;
    double value;
};

struct [[sattr(guid = "7b9c85a6-292f-4bd0-85bf-6fd3dec8410a" serde = @enable)]]
MaterialValueFloat2
{
    MaterialPropertyNameView slot_name;
    float2 value;
};

struct [[sattr(guid = "d788b57b-65f6-490d-9fc6-4f7bc32c18ed" serde = @enable)]]
MaterialValueFloat3
{
    MaterialPropertyNameView slot_name;
    float3 value;
};

struct [[sattr(guid = "7b26477e-caa7-4aa6-8fb0-76f2976e23e2" serde = @enable)]]
MaterialValueFloat4
{
    MaterialPropertyNameView slot_name;
    float4 value;
};

struct [[sattr(guid = "31c522ce-7124-45c6-8d2d-5430aaf17e8a" serde = @enable)]]
MaterialValueTexture
{
    MaterialPropertyNameView slot_name;
    skr::GUID value;

    [[sattr(serde = @disable)]]
    mutable uint32_t bindless_id;
};

struct [[sattr(guid = "760d78ba-c42c-49fa-9164-6968e7693461" serde = @enable)]]
MaterialValueSampler
{
    MaterialPropertyNameView slot_name;
    skr::GUID value;
};

struct [[sattr(guid = "7cbbb808-20d9-4bff-b72d-3c23d5b00f2b" serde = @enable)]]
MaterialShaderVariant
{
    // refers to a ShaderCollectionResource
    skr::GUID shader_collection;
    // variant hash of static switches -> MultiShaderResource
    StableShaderHash switch_hash;
    // static switch value selection indices, const during runtime
    skr::SerializeConstVector<uint32_t> switch_indices;
    // variant hash of default options -> PlatformShaderIdentifier
    StableShaderHash option_hash;
    // options value selection indices, immutable during runtime
    skr::SerializeConstVector<uint32_t> option_indices;
};

struct [[sattr(guid = "e81946ee-fb88-4cde-abd5-b4ae56dbaa89" serde = @enable)]]
MaterialOverrides
{
    skr::SerializeConstVector<MaterialShaderVariant> switch_variants;
    skr::SerializeConstVector<MaterialValueBool> bools;
    skr::SerializeConstVector<MaterialValueFloat> floats;
    skr::SerializeConstVector<MaterialValueFloat2> float2s;
    skr::SerializeConstVector<MaterialValueFloat3> float3s;
    skr::SerializeConstVector<MaterialValueFloat4> float4s;
    skr::SerializeConstVector<MaterialValueDouble> doubles;
    skr::SerializeConstVector<MaterialValueTexture> textures;
    skr::SerializeConstVector<MaterialValueSampler> samplers;
};

struct [[sattr(guid = "2efad635-b331-4fc6-8c52-2f8ca954823e" serde = @enable)]]
MaterialResource
{
    uint32_t material_type_version;
    AsyncResource<MaterialTypeResource> material_type;

    MaterialOverrides overrides;

    typedef struct installed_shader
    {
        PlatformShaderIdentifier identifier;
        skr::StringView entry;
        ECGPUShaderStage stage;
    } installed_shader;

    typedef struct installed_pass
    {
        skr::String name;
        skr::Vector<installed_shader> shaders;
        ESkrInstallStatus status;
        CGPURootSignatureId root_signature;
        skr_pso_map_key_id key;
        CGPURenderPipelineId pso;
        CGPUXBindTableId bind_table;
    } installed_pass;

    [[sattr(serde = @disable)]]
    skr::Vector<installed_pass> installed_passes;
    [[sattr(serde = @disable)]]
    uint64_t mat_id;
};

struct SKR_RENDERER_API MaterialFactory : public ResourceFactory
{
    virtual ~MaterialFactory() = default;

    struct Root
    {
        const RenderDevice* render_device = nullptr;
        ShaderMap* shader_map = nullptr;
        skr_vfs_t* bytecode_vfs = nullptr;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_job_queue_id job_queue = nullptr;
        skr::RC<gpu::TableManager> table_manager = nullptr;
    };

    virtual CGPUDescriptorBufferId descriptor_buffer() = 0;
    virtual skr::RC<gpu::TableInstance> material_table() = 0;
    virtual skr::render_graph::BufferHandle UpdateGPUTable(skr::render_graph::RenderGraph* graph) = 0;

    [[nodiscard]] static MaterialFactory* Create(const Root& root);
    static void Destroy(MaterialFactory* factory);
};
} // namespace skr