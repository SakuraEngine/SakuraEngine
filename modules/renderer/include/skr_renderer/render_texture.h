#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/io.h"
#include "utils/types.h"
#include "cgpu/io.h"
#ifndef __meta__
#include "SkrRenderer/skr_renderer/render_texture.generated.h"
#endif

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

// (GPU) texture resource
sreflect_struct("guid" : "f8821efb-f027-4367-a244-9cc3efb3a3bf")
sattr("serialize" : "bin")
skr_render_texture_t
{
    uint32_t format; // TODO: TEnum<ECGPUFormat>
    uint32_t mips_count;
    uint64_t data_size;
};
typedef struct skr_render_texture_t skr_render_texture_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_texture_create_from_png(skr_render_texture_io_t* io, const char* path, 
    skr_render_texture_request_t* request, skr_async_vtexture_destination_t* destination);
#endif
