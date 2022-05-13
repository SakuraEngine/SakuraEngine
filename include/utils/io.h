#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"
#include "platform/vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SKR_ASYNC_IO_SERVICE_MAX_TASK_COUNT 32
#define SKR_ASYNC_IO_SERVICE_SLEEP_TIME_NEVER UINT32_MAX

typedef enum SkrAsyncIOServiceStatus
{
    SKR_ASYNC_IO_SERVICE_STATUS_SLEEPING = 0,
    SKR_ASYNC_IO_SERVICE_STATUS_RUNNING = 1,
    SKR_ASYNC_IO_SERVICE_STATUS_COUNT,
    SKR_ASYNC_IO_SERVICE_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncIOServiceStatus;

typedef enum SkrAsyncIOStatus
{
    SKR_ASYNC_IO_STATUS_OK = 0,
    SKR_ASYNC_IO_STATUS_ENQUEUED = 1,
    SKR_ASYNC_IO_STATUS_CANCELLED = 2,
    SKR_ASYNC_IO_STATUS_RAM_LOADING = 3,
    SKR_ASYNC_IO_STATUS_VRAM_LOADING = 4,
    SKR_ASYNC_IO_STATUS_COUNT,
    SKR_ASYNC_IO_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncIOStatus;

typedef enum SkrIOServicePriority
{
    SKR_IO_SERVICE_PRIORITY_LOW = -1,
    SKR_IO_SERVICE_PRIORITY_NORMAL = 0,
    SKR_IO_SERVICE_PRIORITY_URGENT = 1,
    SKR_IO_SERVICE_PRIORITY_COUNT = 3,
    SKR_IO_SERVICE_PRIORITY_MAX_ENUM = INT32_MAX
} SkrIOServicePriority;

typedef struct skr_async_io_request_t {
    SAtomic32 status;
    char8_t* bytes;
    uint64_t size;
#ifdef __cplusplus
    RUNTIME_API bool is_ready() const RUNTIME_NOEXCEPT;
    RUNTIME_API bool is_enqueued() const RUNTIME_NOEXCEPT;
    RUNTIME_API bool is_cancelled() const RUNTIME_NOEXCEPT;
    RUNTIME_API bool is_ram_loading() const RUNTIME_NOEXCEPT;
    RUNTIME_API bool is_vram_loading() const RUNTIME_NOEXCEPT;
    RUNTIME_API SkrAsyncIOStatus get_status() const RUNTIME_NOEXCEPT;
#endif
} skr_async_io_request_t;

typedef struct skr_ram_io_service_desc_t {
    const char8_t* name;
    uint32_t sleep_time;
} skr_ram_io_service_desc_t;

typedef void (*skr_async_io_callback_t)(void* data);
typedef struct skr_ram_io_t {
    const char8_t* path;
    uint8_t* bytes;
    uint64_t offset;
    uint64_t size;
    SkrIOServicePriority priority;
    float sub_priority; /*0.f ~ 1.f*/
    skr_async_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_ram_io_t;

#ifdef __cplusplus
}
#endif