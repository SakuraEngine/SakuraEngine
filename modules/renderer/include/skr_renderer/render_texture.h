#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "utils/io.h"
#include "cgpu/io.h"

typedef struct skr_render_texture_request_t {
    CGPUQueueId queue_override;
    CGPUDStorageQueueId dstorage_queue_override;
    skr_async_io_request_t ram_request;
    skr_async_io_request_t vram_request;
    skr_vram_texture_request_t vram_texture_request;
} skr_render_texture_request_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_render_texture_create_from_png(skr_io_ram_service_t*, skr_io_vram_service_t*, const char* path, skr_render_texture_request_t* request);
#endif
