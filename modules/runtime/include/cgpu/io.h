#pragma once
#include "cgpu/api.h"
#include "io/io.h"

typedef void (*skr_async_callback_t)(skr_io_future_t* future, void* data);

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
    SkrAsyncServicePriority priority;
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
    SkrAsyncServicePriority priority;
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
    SkrAsyncServiceSortMethod sort_method;
    SkrAsyncServiceSleepMode sleep_mode;
} skr_vram_io_service_desc_t;


#ifdef __cplusplus
struct CGPU_API skr_io_vram_service_t
{
public:
    [[nodiscard]] static skr_io_vram_service_t* create(const skr_vram_io_service_desc_t* desc) SKR_NOEXCEPT;
    static void destroy(skr_io_vram_service_t* service) SKR_NOEXCEPT;

    // we do not lock an ioService to a single vfs, but for better bandwidth use and easier profiling
    // it's recommended to make a unique relevance between ioService & vfs（or vfses share a single I/O hardware)

    virtual void request(const skr_vram_buffer_io_t* buffer_info, skr_io_future_t* async_request, skr_async_vbuffer_destination_t* destination) SKR_NOEXCEPT = 0;
    virtual void request(const skr_vram_texture_io_t* texture_info, skr_io_future_t* async_request, skr_async_vtexture_destination_t* destination) SKR_NOEXCEPT = 0;

    // try to cancel an enqueued request at **this** thread
    // not available (returns always false) under lockless mode
    // returns false if the request is under LOADING status
    virtual bool try_cancel(skr_io_future_t* request) SKR_NOEXCEPT = 0;

    // emplace a cancel **command** to ioService thread
    // it's recommended to use this under lockless mode
    virtual void defer_cancel(skr_io_future_t* request) SKR_NOEXCEPT = 0;

    // stop service and hang up underground thread
    virtual void stop(bool wait_drain = false) SKR_NOEXCEPT = 0;

    // start & run service
    virtual void run() SKR_NOEXCEPT = 0;

    // block & finish up all requests
    virtual void drain() SKR_NOEXCEPT = 0;

    // set sleep time when io queue is detected to be idle
    virtual void set_sleep_time(uint32_t time) SKR_NOEXCEPT = 0;

    // get service status (sleeping or running)
    virtual SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT = 0;

    virtual ~skr_io_vram_service_t() SKR_NOEXCEPT = default;
    skr_io_vram_service_t() SKR_NOEXCEPT = default;
};
#else
typedef struct skr_io_vram_service_t skr_io_vram_service_t;
#endif