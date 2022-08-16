#pragma once
#include "model_resource.h"
#include "skr_renderer/primitive_draw.h"
#include "cgpu/api.h"
#include "cgpu/io.h"

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

typedef struct skr_live2d_vertex_pos_t {
    float x;
    float y;
} skr_live2d_vertex_pos_t;

typedef struct skr_live2d_vertex_uv_t {
    float u;
    float v;
} skr_live2d_vertex_uv_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_render_model_create_from_raw(skr_io_ram_service_t*, skr_io_vram_service_t*, CGPUDeviceId device,
    skr_live2d_model_resource_id resource, skr_live2d_render_model_request_t* request);
#endif

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_render_model_free(skr_live2d_render_model_id render_model);