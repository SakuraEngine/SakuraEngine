#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"
#include "platform/vfs.h"

#ifdef __cplusplus
extern "C" {
#endif
#define SKR_ASYNC_IO_STATUS_OK 0
#define SKR_ASYNC_IO_STATUS_NONE 1
#define SKR_ASYNC_IO_STATUS_RAM_LOADING 2
#define SKR_ASYNC_IO_STATUS_VRAM_LOADING 3
typedef struct RUNTIME_API skr_async_io_request_t {
    SAtomic32 status;
    struct skr_async_io_request_counter_t* counter;
    struct skr_async_io_task_t* task;
#ifdef __cplusplus
    bool is_ready() const;
#endif
} skr_async_io_request_t;

RUNTIME_API void skr_io_ram_initialize();
RUNTIME_API void skr_io_ram_request(skr_vfs_t* vfs,
const char8_t* path, uint8_t* bytes,
uint64_t offset, uint64_t size, skr_async_io_request_t* request);
#ifdef __cplusplus
}
#endif