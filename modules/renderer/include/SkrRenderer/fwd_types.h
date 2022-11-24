#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/types.h"

#ifdef __cplusplus
namespace skr { struct RendererDevice; }
struct skr_threaded_service_t;
namespace skr { namespace io { class VRAMService; } }
typedef struct skr::RendererDevice SRenderDevice;
namespace skr::resource { template <class T> struct TResourceHandle; }
class SkrRendererModule;
#else
typedef struct SRenderDevice SRenderDevice;
#endif

typedef SRenderDevice* SRenderDeviceId;
typedef skr_guid_t skr_vertex_layout_id;

typedef struct skr_platform_shader_collection_resource_t skr_platform_shader_collection_resource_t;
typedef struct skr_platform_shader_resource_t skr_platform_shader_resource_t;
typedef struct skr_material_type_resource_t skr_material_type_resource_t;
typedef struct skr_material_resource_t skr_material_resource_t;
typedef struct skr_material_value_t skr_material_value_t;

#ifdef __cplusplus
    using skr_shader_resource_handle_t = skr::resource::TResourceHandle<skr_platform_shader_resource_t>;
    using skr_material_type_handle_t = skr::resource::TResourceHandle<skr_material_type_resource_t>;
#else
    typedef struct skr_resource_handle_t skr_shader_resource_handle_t;
    typedef struct skr_resource_handle_t skr_material_type_handle_t;
#endif