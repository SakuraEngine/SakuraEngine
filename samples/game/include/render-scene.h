#pragma once
#include "gamert_configure.h"
#include "ecs/dual.h"
// align up & can directly update with GPU-scene
#include "scene.h"
#include "cgpu/api.h"

// root signatures & pipeline-objects are hidden bardward.

// {909389f9-3850-4be4-af60-4f4b3b128a2b}
const skr_guid_t pipeline_shader_set_guid = 
{0x909389f9, 0x3850, 0x4be4, {0xaf, 0x60, 0x4f, 0x4b, 0x3b, 0x12, 0x8a, 0x2b}};
typedef struct pipeline_shader_set_t {
    CGPUShaderLibraryId vs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId hs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId ds SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId gs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId ps SKR_IF_CPP(= nullptr);
} pipeline_shader_set_t;
typedef dual_entity_t pipeline_shader_set_id_t;

// {0b2a2bb5-1a6d-4f63-aee2-ef88174c39ed}
const skr_guid_t processor_shader_set_guid = 
{0x0b2a2bb5, 0x1a6d, 0x4f63, {0xae, 0xe2, 0xef, 0x88, 0x17, 0x4c, 0x39, 0xed}};
typedef struct processor_shader_set_t {
    CGPUShaderLibraryId cs SKR_IF_CPP(= nullptr);
} processor_shader_set_t;
typedef dual_entity_t processor_shader_set_id_t;

// {640a76c0-ec8e-46ad-89f2-be886950b315}
const skr_guid_t gfx_material_guid = 
{0x640a76c0, 0xec8e, 0x46ad, {0x89, 0xf2, 0xbe, 0x88, 0x69, 0x50, 0xb3, 0x15}};
typedef struct gfx_material_t {
    pipeline_shader_set_id_t m_gfx;
    ECGPUFormat formats[CGPU_MAX_MRT_COUNT];
    uint32_t render_target_count;
    ECGPUFormat depth_stencil_format;
    ECGPUPrimitiveTopology prim_topology;
    // MSAA
    ECGPUSampleCount sample_count;
    uint32_t sample_quality;
    uint32_t color_resolve_disable_mask;
    // static samplers
    const char8_t* static_sampler_names;
    CGPUSamplerId static_samplers;
    uint32_t static_sampler_count;
    // TODO: handle this automatically with spirv-reflect under cook-time
    const char8_t* push_constant_names;
    uint32_t push_constant_count;
    // TODO: indirect cmd
    // TODO: const specs
} gfx_material_t;
typedef dual_entity_t gfx_material_id_t;

// {fde818b7-777d-4d29-8230-cc06550cedee}
const skr_guid_t cmpt_material_guid =
{0xfde818b7, 0x777d, 0x4d29, {0x82, 0x30, 0xcc, 0x06, 0x55, 0x0c, 0xed, 0xee}};
typedef struct cmpt_material_t {
    processor_shader_set_id_t m_cs;
    CGPUComputePipelineDescriptor cp;
} cmpt_material_t;
typedef cmpt_material_t processor_material_t;
typedef dual_entity_t cmpt_material_id_t;
typedef cmpt_material_id_t processor_material_id_t;

typedef enum skr_gfx_mat_param_freq {
    SKR_GFX_MAT_PARM_FREQ_PUSH_CONST,
    SKR_GFX_MAT_PARM_FREQ_PER_INST,
    SKR_GFX_MAT_PARM_FREQ_PER_BATCH,
    SKR_GFX_MAT_PARM_FREQ_PER_FRAME,
    SKR_GFX_MAT_PARM_FREQ_COUNT,
    SKR_GFX_MAT_PARM_PER_MAX_ENUM = 0x7FFFFFFF
} skr_gfx_mat_param_freq;

typedef struct skr_gfx_mat_param_t {
    CGPUShaderStages out_visiblity;
    skr_guid_t parameter_t;
    const char8_t* name;
    skr_gfx_mat_param_freq frequency;
} skr_gfx_mat_param_t;

GAMERT_API gfx_material_id_t ecsr_register_gfx_material(const gfx_material_t*) SKR_NOEXCEPT;
GAMERT_API bool ecsr_unregister_gfx_material(gfx_material_id_t) SKR_NOEXCEPT;
GAMERT_API processor_material_t ecsr_register_processor_material(const processor_material_t*) SKR_NOEXCEPT;
GAMERT_API bool ecsr_unregister_processor_material(processor_material_t) SKR_NOEXCEPT;
GAMERT_API bool ecsr_register_gfx_mat_paramter(gfx_material_id_t mat, const skr_gfx_mat_param_t*) SKR_NOEXCEPT; 