#pragma once
#include "model_resource.h"
#include "skr_renderer/primitive_draw.h"
#include "cgpu/api.h"
#include "cgpu/io.h"

typedef struct skr_live2d_render_model_t skr_live2d_render_model_t;
typedef struct skr_live2d_render_model_t* skr_live2d_render_model_id;

typedef struct skr_live2d_render_model_request_t {
    const char* model_name;
    CGPUQueueId queue_override;
    CGPUDStorageQueueId dstorage_queue_override;
    ECGPUDStorageSource dstorage_source;
    skr_live2d_render_model_id render_model;
    SAtomic32 buffers_io_status;
#ifdef __cplusplus
    bool is_buffer_ready() const SKR_NOEXCEPT;
    SkrAsyncIOStatus get_buffer_status() const SKR_NOEXCEPT;
#endif
} skr_live2d_render_model_request_t;


#ifndef SKR_SERIALIZE_GURAD
SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_render_model_create_from_raw(skr_io_ram_service_t*, skr_io_vram_service_t*, skr_live2d_model_resource_id resource, skr_live2d_render_model_t* request);
#endif

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void 
skr_live2d_render_model_free(skr_live2d_render_model_id render_model);