#pragma once
#include "cgpu/api.h"
#include "utils/io.h"

// no format & stride parameters provided now because we think it's not necessary to support async io with UAV buffers
// we can add them if necessary in the future
typedef struct skr_vram_buffer_io_t {
    CGPUDeviceId device;
    CGPUQueueId transfer_queue;
    CGPUSemaphoreId opt_semaphore;
    // CGPU Resource Desc
    struct 
    {
        const char8_t* buffer_name;
        CGPUResourceTypes resource_types;
        ECGPUMemoryUsage memory_usage;
        CGPUBufferCreationFlags flags;
        uint64_t buffer_size;
        uint64_t offset;
        /// Preferred actual location
        /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
        bool prefer_on_device;
        /// Preferred actual location
        /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
        bool prefer_on_host;
    } vbuffer;
    // Direct Storage
    struct 
    {
        const char8_t* path;
        CGPUDStorageQueueId queue;
        CGPUDStorageCompression compression;
        ECGPUDStorageSource source_type;
        uint64_t uncompressed_size;
    } dstorage;
    // Src Memory
    struct
    {
        uint8_t* bytes;
        uint64_t size;
    } src_memory;
    SkrIOServicePriority priority;
    float sub_priority; /*0.f ~ 1.f*/
    skr_async_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_vram_buffer_io_t;

typedef struct skr_async_vbuffer_destination_t {
    CGPUBufferId buffer;
} skr_async_vbuffer_destination_t;

typedef struct skr_vram_texture_io_t {
    CGPUDeviceId device;
    CGPUQueueId transfer_queue;
    CGPUSemaphoreId opt_semaphore;
    // CGPU Resource Desc
    struct
    {
        const char8_t* texture_name;
        CGPUResourceTypes resource_types;
        CGPUTextureCreationFlags flags;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        ECGPUFormat format;
    } vtexture;
    // Direct Storage
    struct
    {
        const char8_t* path;
        CGPUDStorageQueueId queue;
        CGPUDStorageCompression compression;
        ECGPUDStorageSource source_type;
        uint64_t uncompressed_size;
    } dstorage;
    // Data bytes
    struct
    {
        const uint8_t* bytes;
        uint64_t size;
    } src_memory;
    SkrIOServicePriority priority;
    float sub_priority; /*0.f ~ 1.f*/
    skr_async_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_vram_texture_io_t;

typedef struct skr_async_vtexture_destination_t {
    CGPUTextureId texture;
} skr_async_vtexture_destination_t;

typedef struct skr_vram_io_service_desc_t {
    const char8_t* name;
    uint32_t sleep_time;
    bool lockless;
    SkrServiceTaskSortMethod sort_method;
    SkrAsyncServiceSleepMode sleep_mode;
} skr_vram_io_service_desc_t;


#ifdef __cplusplus
namespace skr { namespace io { class VRAMService; } }
using skr_io_vram_service_t = skr::io::VRAMService;
#else
typedef struct skr_io_vram_service_t skr_io_vram_service_t;
#endif