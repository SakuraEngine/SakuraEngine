#pragma once
#include "utils/types.h"
#include "cgpu/api.h"
#include "cgpu/io.h"
#include "skr_renderer/primitive_draw.h"
#include "model_resource.h"

typedef struct skr_live2d_clipping_manager_t skr_live2d_clipping_manager_t;
typedef struct skr_live2d_clipping_manager_t* skr_live2d_clipping_manager_id;
typedef struct skr_live2d_render_model_t skr_live2d_render_model_t;
typedef struct skr_live2d_render_model_t* skr_live2d_render_model_id;

typedef struct skr_live2d_render_model_request_t {
    const char* model_name;
    skr_vfs_t* vfs_override;
    CGPUQueueId queue_override;
    CGPUDStorageQueueId file_dstorage_queue_override;
    CGPUDStorageQueueId memory_dstorage_queue_override;
    ECGPUDStorageSource dstorage_source;
    skr_live2d_render_model_id render_model;
    SAtomic32 io_status;
#ifdef __cplusplus
    SKR_LIVE2D_API bool is_ready() const SKR_NOEXCEPT;
    SKR_LIVE2D_API SkrAsyncIOStatus get_status() const SKR_NOEXCEPT;
#endif
} skr_live2d_render_model_request_t;

struct skr_live2d_render_model_comp_t {
    skr_guid_t resource_guid;
    skr_live2d_ram_io_request_t ram_request;
    skr_live2d_render_model_request_t vram_request;
};
typedef struct skr_live2d_render_model_comp_t skr_live2d_render_model_comp_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_render_model_create_from_raw(skr_io_ram_service_t*, skr_io_vram_service_t*, CGPUDeviceId device,
    skr_live2d_model_resource_id resource, skr_live2d_render_model_request_t* request);
#endif

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API CGPUTextureId
skr_live2d_render_model_get_texture(skr_live2d_render_model_id render_model, uint32_t drawable_index);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API CGPUTextureViewId
skr_live2d_render_model_get_texture_view(skr_live2d_render_model_id render_model, uint32_t drawable_index);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_render_model_free(skr_live2d_render_model_id render_model);

#ifdef __cplusplus
#include <EASTL/vector.h>
struct skr_live2d_render_model_t {
    virtual ~skr_live2d_render_model_t() = default;
    skr_live2d_model_resource_id model_resource_id;
    bool use_dynamic_buffer = true;

    // clipping 
    skr_live2d_clipping_manager_id clipping_manager;
    // pos-uv-pos-uv...
    eastl::vector<CGPUTextureId> textures;
    eastl::vector<CGPUTextureViewId> texture_views;
    eastl::vector<skr_vertex_buffer_view_t> vertex_buffer_views;
    eastl::vector<skr_index_buffer_view_t> index_buffer_views;
    eastl::vector<skr_render_primitive_command_t> primitive_commands;
};

#include "ecs/dual.h"
template<>
struct SKR_LIVE2D_API dual_id_of<skr_live2d_render_model_comp_t>
{
    static dual_type_index_t get();
};
#endif
