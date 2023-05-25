#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"

#define SKR_IO_SERVICE_MAX_TASK_COUNT 32
#define SKR_ASYNC_SERVICE_SLEEP_TIME_MAX UINT32_MAX

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)

typedef enum SkrAsyncServiceStatus
{
    SKR_ASYNC_SERVICE_STATUS_SLEEPING = 0,
    SKR_ASYNC_SERVICE_STATUS_RUNNING = 1,
    SKR_ASYNC_SERVICE_STATUS_QUITING = 2,
    SKR_ASYNC_SERVICE_STATUS_COUNT,
    SKR_ASYNC_SERVICE_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncServiceStatus;

typedef enum SkrAsyncServiceSleepMode
{
    SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR = 0,
    SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP = 1,
    SKR_ASYNC_SERVICE_SLEEP_MODE_COUNT,
    SKR_ASYNC_SERVICE_SLEEP_MAX_ENUM = UINT32_MAX
} SkrAsyncServiceSleepMode;

typedef enum SkrAsyncServiceTaskStatus
{
    SKR_ASYNC_SERVICE_TASK_STATUS_NONE = 0,
    SKR_ASYNC_SERVICE_TASK_STATUS_ENQUEUED = 1,
    SKR_ASYNC_SERVICE_TASK_STATUS_CANCELLED = 2,
    SKR_ASYNC_SERVICE_TASK_STATUS_OK = 3,
    SKR_ASYNC_SERVICE_TASK_STATUS_COUNT,
    SKR_ASYNC_SERVICE_TASK_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncServiceTaskStatus;

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

typedef enum SkrAsyncServicePriority
{
    SKR_ASYNC_SERVICE_PRIORITY_LOW = -1,
    SKR_ASYNC_SERVICE_PRIORITY_NORMAL = 0,
    SKR_ASYNC_SERVICE_PRIORITY_URGENT = 1,
    SKR_ASYNC_SERVICE_PRIORITY_COUNT = 3,
    SKR_ASYNC_SERVICE_PRIORITY_MAX_ENUM = INT32_MAX
} SkrAsyncServicePriority;

typedef enum SkrAsyncServiceSortMethod
{
    SKR_ASYNC_SERVICE_SORT_METHOD_NEVER = 0,
    SKR_ASYNC_SERVICE_SORT_METHOD_STABLE = 1,
    SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL = 2,
    SKR_ASYNC_SERVICE_SORT_METHOD_COUNT,
    SKR_ASYNC_SERVICE_SORT_METHOD_MAX_ENUM = INT32_MAX
} SkrAsyncServiceSortMethod;

typedef struct skr_async_request_t {
    SAtomicU32 status;
    SAtomicU32 request_cancel;
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
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    bool lockless SKR_IF_CPP(= true);
    SkrAsyncServiceSortMethod sort_method SKR_IF_CPP(= SKR_ASYNC_SERVICE_SORT_METHOD_NEVER);
    SkrAsyncServiceSleepMode sleep_mode SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR);
    skr_job_queue_id job_queue SKR_IF_CPP(= nullptr);
} skr_ram_io_service_desc_t;

typedef void (*skr_async_callback_t)(skr_async_request_t* request, void* data);
typedef struct skr_ram_io_t {
    const char8_t* path SKR_IF_CPP(= nullptr);
    uint64_t offset SKR_IF_CPP(= 0);
    SkrAsyncServicePriority priority SKR_IF_CPP(= SKR_ASYNC_SERVICE_PRIORITY_NORMAL);
    float sub_priority SKR_IF_CPP(= 0.f); /*0.f ~ 1.f*/
    skr_async_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_ram_io_t;

#ifdef __cplusplus
struct skr_vfs_t;
struct RUNTIME_API skr_io_ram_service_t
{
public:
    [[nodiscard]] static skr_io_ram_service_t* create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    static void destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT;

    // we do not lock an ioService to a single vfs, but for better bandwidth use and easier profiling
    // it's recommended to make a unique relevance between ioService & vfs（or vfses share a single I/O hardware)
    virtual void request(skr_vfs_t*, const skr_ram_io_t* info, skr_async_request_t* async_request, skr_async_ram_destination_t* dst) SKR_NOEXCEPT = 0;

    // try to cancel an enqueued request at **this** thread
    // not available (returns always false) under lockless mode
    // returns false if the request is under LOADING status
    virtual bool try_cancel(skr_async_request_t* request) SKR_NOEXCEPT = 0;

    // emplace a cancel **command** to ioService thread
    // it's recommended to use this under lockless mode
    virtual void defer_cancel(skr_async_request_t* request) SKR_NOEXCEPT = 0;

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

    virtual ~skr_io_ram_service_t() SKR_NOEXCEPT = default;
    skr_io_ram_service_t() SKR_NOEXCEPT = default;
};
#else
typedef struct skr_io_ram_service_t skr_io_ram_service_t;
#endif