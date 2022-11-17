#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"

#define SKR_IO_SERVICE_MAX_TASK_COUNT 32
#define SKR_IO_SERVICE_SLEEP_TIME_MAX UINT32_MAX

typedef enum SkrAsyncIOServiceStatus
{
    SKR_IO_SERVICE_STATUS_SLEEPING = 0,
    SKR_IO_SERVICE_STATUS_RUNNING = 1,
    SKR_IO_SERVICE_STATUS_COUNT,
    SKR_IO_SERVICE_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncIOServiceStatus;

typedef enum SkrAsyncServiceSleepMode
{
    SKR_IO_SERVICE_SLEEP_MODE_COND_VAR = 0,
    SKR_IO_SERVICE_SLEEP_MODE_SLEEP = 1,
    SKR_IO_SERVICE_SLEEP_MODE_COUNT,
    SKR_IO_SERVICE_SLEEP_MAX_ENUM = UINT32_MAX
} SkrAsyncServiceSleepMode;

typedef enum SkrAsyncIOStatus
{
    SKR_ASYNC_IO_STATUS_NONE = 0,
    SKR_ASYNC_IO_STATUS_ENQUEUED = 1,
    SKR_ASYNC_IO_STATUS_CANCELLED = 2,
    SKR_ASYNC_IO_STATUS_CREATING_RESOURCE = 3,
    SKR_ASYNC_IO_STATUS_RAM_LOADING = 5,
    SKR_ASYNC_IO_STATUS_VRAM_LOADING = 6,
    SKR_ASYNC_IO_STATUS_OK = 7,
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

typedef enum SkrServiceTaskSortMethod
{
    SKR_IO_SERVICE_SORT_METHOD_NEVER = 0,
    SKR_IO_SERVICE_SORT_METHOD_STABLE = 1,
    SKR_IO_SERVICE_SORT_METHOD_PARTIAL = 2,
    SKR_IO_SERVICE_SORT_METHOD_COUNT,
    SKR_IO_SERVICE_SORT_METHOD_MAX_ENUM = INT32_MAX
} SkrServiceTaskSortMethod;

typedef struct skr_async_request_t {
    SAtomic32 status;
    SAtomic32 request_cancel;
#ifdef __cplusplus
    RUNTIME_API bool is_ready() const SKR_NOEXCEPT;
    RUNTIME_API bool is_enqueued() const SKR_NOEXCEPT;
    RUNTIME_API bool is_cancelled() const SKR_NOEXCEPT;
    RUNTIME_API bool is_ram_loading() const SKR_NOEXCEPT;
    RUNTIME_API bool is_vram_loading() const SKR_NOEXCEPT;
    RUNTIME_API SkrAsyncIOStatus get_status() const SKR_NOEXCEPT;
#endif
} skr_async_request_t;

typedef struct skr_async_ram_destination_t {
    uint8_t* bytes SKR_IF_CPP(= nullptr);
    uint64_t size SKR_IF_CPP(= 0);
} skr_async_ram_destination_t;

typedef struct skr_ram_io_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_IO_SERVICE_SLEEP_TIME_MAX);
    bool lockless SKR_IF_CPP(= true);
    SkrServiceTaskSortMethod sort_method SKR_IF_CPP(= SKR_IO_SERVICE_SORT_METHOD_NEVER);
    SkrAsyncServiceSleepMode sleep_mode SKR_IF_CPP(= SKR_IO_SERVICE_SLEEP_MODE_COND_VAR);
} skr_ram_io_service_desc_t;

typedef void (*skr_async_callback_t)(skr_async_request_t* request, void* data);
typedef struct skr_ram_io_t {
    const char8_t* path SKR_IF_CPP(= nullptr);
    uint64_t offset SKR_IF_CPP(= 0);
    SkrIOServicePriority priority SKR_IF_CPP(= SKR_IO_SERVICE_PRIORITY_NORMAL);
    float sub_priority SKR_IF_CPP(= 0.f); /*0.f ~ 1.f*/
    skr_async_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_ram_io_t;

#ifdef __cplusplus
namespace skr { namespace io { class RAMService; } }
using skr_io_ram_service_t = skr::io::RAMService;
#else
typedef struct skr_io_ram_service_t skr_io_ram_service_t;
#endif