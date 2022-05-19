#pragma once
#include "gamert_configure.h"
#include "ecs/dual.h"
// align up & can directly update with GPU-scene
#include "scene.h"
#include "cgpu/api.h"
#include "utils/types.h"

struct skr_render_graph_t;
// root signatures & pipeline-objects are hidden bardward.

// {909389f9-3850-4be4-af60-4f4b3b128a2b}
const skr_guid_t gfx_shader_set_guid = 
{0x909389f9, 0x3850, 0x4be4, {0xaf, 0x60, 0x4f, 0x4b, 0x3b, 0x12, 0x8a, 0x2b}};
typedef struct gfx_shader_set_t {
    CGPUShaderLibraryId vs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId hs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId ds SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId gs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId ps SKR_IF_CPP(= nullptr);
} gfx_shader_set_t;
typedef dual_entity_t gfx_shader_set_id_t;

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
    CGPUDeviceId device;
    gfx_shader_set_id_t m_gfx;
    // static samplers
    const char8_t* const* static_sampler_names;
    const CGPUSamplerId* static_samplers;
    uint32_t static_sampler_count;
    // below: maybe suitable to be placed at otherwhere?
    // CGPUVertexLayout vertex_layout;
    ECGPUFormat formats[CGPU_MAX_MRT_COUNT];
    uint32_t render_target_count;
    ECGPUFormat depth_stencil_format;
    ECGPUPrimitiveTopology prim_topology;
    // MSAA
    ECGPUSampleCount sample_count;
    uint32_t sample_quality;
    uint32_t color_resolve_disable_mask;
    // TODO: handle this automatically with spirv-reflect under cook-time
    const char8_t* const* push_constant_names;
    uint32_t push_constant_count;
    // TODO: indirect cmd
    // TODO: const specs
} gfx_material_t;
typedef dual_entity_t gfx_material_id_t;
// scene primitives use gfx_material entity as meta ent for automatic batch

// {97e3f0c6-66ad-4bbe-927b-1d5f2baace51}
const skr_guid_t gfx_material_inst_guid = 
{0x97e3f0c6, 0x66ad, 0x4bbe, {0x92, 0x7b, 0x1d, 0x5f, 0x2b, 0xaa, 0xce, 0x51}};
typedef struct gfx_material_inst_t {
    gfx_material_id_t material;
} gfx_material_inst_t;
typedef dual_entity_t gfx_material_inst_id_t;

// {2bd63472-32dc-4be3-b984-0e08d763b61b}
const skr_guid_t transform_guid =
{0x2bd63472, 0x32dc, 0x4be3, {0xb9, 0x84, 0x0e, 0x08, 0xd7, 0x63, 0xb6, 0x1b}};
typedef struct transform_t {
    skr_float3_t location;
    skr_float3_t scale;
    skr_float4_t rotation;
} transform_t;

struct sreflect sattr(
    "guid" : "2a661e86-4dc0-4fb6-808f-ce9f4ffd0448",
    "component" : 
    {
        "buffer" : 8
    }
) ecsr_vertex_buffer_t {
    CGPUBufferId buffer;
    uint32_t stride;
    uint32_t offset;
#ifdef __cplusplus
    ecsr_vertex_buffer_t(CGPUBufferId buffer, uint32_t stride, uint32_t offset)
        : buffer(buffer), stride(stride), offset(offset){}
    ecsr_vertex_buffer_t() = default;
#endif
};
typedef struct ecsr_vertex_buffer_t ecsr_vertex_buffer_t;

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

// scene primitive necessary components:
// | material | transform | vb(s) | ib | registered_params... |
typedef struct skr_scene_primitive_desc_t {
    gfx_material_id_t material;
    
} skr_scene_primitive_desc_t;

#ifdef __cplusplus
extern "C" {
#endif

// gfx-shaders
GAMERT_API gfx_shader_set_id_t ecsr_register_gfx_shader_set(const gfx_shader_set_t*) SKR_NOEXCEPT;
GAMERT_API gfx_shader_set_t* ecsr_query_gfx_shader_set(gfx_shader_set_id_t) SKR_NOEXCEPT;
GAMERT_API bool ecsr_unregister_gfx_shader_set(gfx_shader_set_id_t) SKR_NOEXCEPT;

// gfx-mats
GAMERT_API gfx_material_id_t ecsr_register_gfx_material(const gfx_material_t*) SKR_NOEXCEPT;
GAMERT_API gfx_material_t* ecsr_query_gfx_material(gfx_material_id_t) SKR_NOEXCEPT;
// normally no need to call this
GAMERT_API dual_type_index_t ecsr_query_material_parameter_type(gfx_material_id_t, const char8_t*);
GAMERT_API bool ecsr_unregister_gfx_material(gfx_material_id_t) SKR_NOEXCEPT;

// gfx-parameters

// scene
GAMERT_API bool ecsr_renderable_primitive_type(const skr_scene_primitive_desc_t* desc, 
dual_type_index_t* ctypes, uint32_t* ctype_count, dual_entity_t* emetas, uint32_t* meta_count) SKR_NOEXCEPT;
GAMERT_API void ecsr_draw_scene(struct skr_render_graph_t* graph) SKR_NOEXCEPT;

// cs-mats
// GAMERT_API processor_material_t ecsr_register_processor_material(const processor_material_t*) SKR_NOEXCEPT;
// GAMERT_API bool ecsr_unregister_processor_material(processor_material_t) SKR_NOEXCEPT;

extern GAMERT_API dual_type_index_t gfx_shader_set_type;
extern GAMERT_API dual_type_index_t processor_shader_set_type;
extern GAMERT_API dual_type_index_t gfx_material_type;
extern GAMERT_API dual_type_index_t gfx_root_sig_type;
extern GAMERT_API dual_type_index_t processor_material_type;

extern GAMERT_API dual_type_index_t gfx_material_inst_type; 
// index/vertex buffers
extern GAMERT_API dual_type_index_t index_buffer_type; 
extern GAMERT_API dual_type_index_t vertex_buffer_type; 
extern GAMERT_API dual_type_index_t transform_type; 

#ifdef __cplusplus
}
#endif