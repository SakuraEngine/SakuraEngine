#pragma once
#include "cgpu/api.h"
#include "utils/io.h"

typedef struct skr_vram_buffer_io_t {
    CGPUDeviceId device;
    CGPUQueueId transfer_queue;
    CGPUDStorageQueueId dstorage_queue;
    CGPUSemaphoreId opt_semaphore;
    // CGPU Resource Desc
    const char8_t* buffer_name;
    CGPUResourceTypes resource_types;
    ECGPUMemoryUsage memory_usage;
    ECGPUFormat format;
    uint64_t buffer_size;
    CGPUBufferCreationFlags flags;
    /// Preferred actual location
    /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
    bool prefer_on_device;
    /// Preferred actual location
    /// Only available when memory_usage is CPU_TO_GPU or GPU_TO_CPU
    bool prefer_on_host;
    // Direct Storage
    ECGPUDStorageSource dstorage_source_type;
    const char8_t* path;
    // Data bytes
    uint8_t* bytes;
    uint64_t offset;
    uint64_t size;
    SkrIOServicePriority priority;
    float sub_priority; /*0.f ~ 1.f*/
    skr_async_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_vram_buffer_io_t;

typedef struct skr_vram_buffer_request_t {
    CGPUBufferId out_buffer;
} skr_vram_buffer_request_t;

typedef struct skr_vram_texture_request_t {
    CGPUTextureId out_texture;
} skr_vram_texture_request_t;

typedef struct skr_vram_io_service_desc_t {
    const char8_t* name;
    uint32_t sleep_time;
    bool lockless;
    SkrIOServiceSortMethod sort_method;
    SkrAsyncIOServiceSleepMode sleep_mode;
} skr_vram_io_service_desc_t;


#ifdef __cplusplus
namespace skr { namespace io { class VRAMService; } }
using skr_io_vram_service_t = skr::io::VRAMService;
#else
typedef struct skr_io_vram_service_t skr_io_vram_service_t;
#endif