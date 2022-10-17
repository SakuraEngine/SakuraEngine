#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/io.h"
#include "cgpu/io.h"

typedef struct skr_render_texture_io_t {
    skr_io_ram_service_t* ram_service;
    skr_io_vram_service_t* vram_service;
    CGPUDeviceId device;
    CGPUQueueId copy_queue;
    CGPUDStorageQueueId file_dstorage_queue;
} skr_render_texture_io_t;

typedef struct skr_render_texture_request_t {
    skr_async_io_request_t ram_request;
    skr_async_io_request_t vtexture_request;
} skr_render_texture_request_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_texture_create_from_png(skr_render_texture_io_t* io, const char* path, 
    skr_render_texture_request_t* request, skr_async_vtexture_destination_t* destination);
#endif
