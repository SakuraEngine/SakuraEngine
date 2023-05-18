#pragma once
#include "SkrRenderer/module.configure.h"
#include "misc/types.h"

struct skr_io_ram_service_t;
struct skr_io_vram_service_t;
struct skr_threaded_service_t;
#ifdef __cplusplus
namespace skr { struct RendererDevice; }
typedef struct skr::RendererDevice SRenderDevice;
namespace skr::resource { template <class T> struct TResourceHandle; }
class SkrRendererModule;
#else
typedef struct SRenderDevice SRenderDevice;
#endif

typedef SRenderDevice* SRenderDeviceId;
typedef skr_guid_t skr_vertex_layout_id;

typedef struct skr_stable_shader_hash_t skr_stable_shader_hash_t;
typedef struct skr_platform_shader_hash_t skr_platform_shader_hash_t;
typedef struct skr_platform_shader_identifier_t skr_platform_shader_identifier_t;

SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderOptionInstance, skr_shader_option_instance);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderOptionTemplate, skr_shader_option_template);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderOptionsResource, skr_shader_options_resource);

SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MultiShaderResource, skr_multi_shader_resource);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderOptionSequence, skr_shader_option_sequence);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderCollectionResource, skr_shader_collection_resource);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderCollectionJSON, skr_shader_collection_json);

SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialProperty, skr_material_property);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValue, skr_material_value);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialPass, skr_material_pass);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialTypeResource, skr_material_type_resource);

SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialOverrides, skr_material_overrides);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialShaderVariant, skr_material_shader_variant);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueBool, skr_material_value_bool);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueFloat, skr_material_value_float);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueDouble, skr_material_value_double);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueFloat2, skr_material_value_float2);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueFloat3, skr_material_value_float3);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueFloat4, skr_material_value_float4);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueTexture, skr_material_value_texture);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialValueSampler, skr_material_value_sampler);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MaterialResource, skr_material_resource);

typedef struct skr_mesh_buffer_t skr_mesh_buffer_t;
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MeshPrimitive, skr_mesh_primitive);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MeshResource, skr_mesh_resource);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, MeshSection, skr_mesh_section);

SKR_DECLARE_TYPE_ID_FWD(skr::renderer, RenderMesh, skr_render_mesh);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, PrimitiveCommand, skr_render_primitive_command);

typedef struct skr_shader_map_t skr_shader_map_t;
typedef struct skr_shader_map_t* skr_shader_map_id;
typedef struct skr_shader_map_root_t skr_shader_map_root_t;

typedef struct skr_pso_map_key_t skr_pso_map_key_t;
typedef struct skr_pso_map_key_t* skr_pso_map_key_id;
typedef struct skr_pso_map_t* skr_pso_map_id;
typedef struct skr_pso_map_root_t skr_pso_map_root_t;

#ifdef __cplusplus
    using skr_shader_resource_handle_t = skr::resource::TResourceHandle<skr_multi_shader_resource_t>;
    using skr_material_type_handle_t = skr::resource::TResourceHandle<skr_material_type_resource_t>;
    using skr_shader_collection_handle_t = skr::resource::TResourceHandle<skr_shader_collection_resource_t>;
    namespace skr { namespace renderer { enum class EShaderOptionType : uint32_t; } }
#else
    typedef struct skr_resource_handle_t skr_shader_resource_handle_t;
    typedef struct skr_resource_handle_t skr_material_type_handle_t;
    typedef struct skr_resource_handle_t skr_shader_collection_handle_t;
#endif