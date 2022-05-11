#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"
#include "platform/vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SKR_ASYNC_IO_SERVICE_MAX_TASK_COUNT 32

typedef enum SkrAsyncIOStatus
{
    SKR_ASYNC_IO_STATUS_OK = 0,
    SKR_ASYNC_IO_STATUS_ENQUEUED = 1,
    SKR_ASYNC_IO_STATUS_CANCELLED = 2,
    SKR_ASYNC_IO_STATUS_RAM_LOADING = 3,
    SKR_ASYNC_IO_STATUS_VRAM_LOADING = 4,
    SKR_ASYNC_IO_STATUS_COUNT
} SkrAsyncIOStatus;

typedef enum SkrIOServicePriority
{
    SKR_IO_SERVICE_PRIORITY_NORMAL = 0,
    SKR_IO_SERVICE_PRIORITY_URGENT = 1,
    SKR_IO_SERVICE_PRIORITY_LOW = 2,
    SKR_IO_SERVICE_PRIORITY_COUNT
} SkrIOServicePriority;

typedef struct RUNTIME_API skr_async_io_request_t {
    SAtomic32 status;
#ifdef __cplusplus
    bool is_ready() const;
    bool is_enqueued() const;
    bool is_cancelled() const;
    bool is_ram_loading() const;
    bool is_vram_loading() const;
#endif
} skr_async_io_request_t;

typedef struct RUNTIME_API skr_ram_io_service_desc_t {
    const char8_t* name;
} skr_ram_io_service_desc_t;

typedef struct RUNTIME_API skr_ram_io_t {
    const char8_t* path;
    uint8_t* bytes;
    uint64_t offset;
    uint64_t size;
} skr_ram_io_t;

#ifdef __cplusplus
}
#endif