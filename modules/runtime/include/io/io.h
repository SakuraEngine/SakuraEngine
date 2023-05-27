#pragma once
#include "misc/types.h"
#include "platform/atomic.h"
#include "platform/thread.h"

#define SKR_IO_SERVICE_MAX_TASK_COUNT 32
#define SKR_ASYNC_SERVICE_SLEEP_TIME_MAX UINT32_MAX

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)
SKR_DECLARE_TYPE_ID_FWD(skr::io, RAMService, skr_io_ram_service)
struct skr_vfs_t;

typedef struct skr_io_request_t skr_io_request_t;
typedef struct skr_vfile_t skr_io_file_t;
typedef skr_io_file_t* skr_io_file_handle;

typedef enum ESkrIOErrorCode
{
    SKR_IO_OK,
    SKR_IO_ERROR_UNKNOWN,
    SKR_IO_ERROR_BUFFER_ALLOC_FAILED,
    SKR_IO_ERROR_COMPRESSED_BUFFER_ALLOC_FAILED,
    SKR_IO_ERROR_FOPEN_FAILED,
    SKR_IO_ERROR_FREAD_FAILED,
    SKR_IO_ERROR_FCLOSE_FAILED,
    SKR_IO_ERROR_INVALID_PATH,
    SKR_IO_ERROR_INVALID_ARGUMENTS,
    SKR_IO_ERROR_DECOMPRESS_FAILED,
    SKR_IO_ERROR_CODE_MAX_ENUM = UINT32_MAX
} ESkrIOErrorCode;

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

// TODO: Remove with IOStage
typedef enum SkrAsyncIOStatus
{
    SKR_ASYNC_IO_STATUS_NONE = 0,
    SKR_ASYNC_IO_STATUS_ENQUEUED = 1,
    SKR_ASYNC_IO_STATUS_CANCELLED = 2,
    SKR_ASYNC_IO_STATUS_CREATING_RESOURCE = 3,
    SKR_ASYNC_IO_STATUS_RAM_LOADING = 5,
    SKR_ASYNC_IO_STATUS_VRAM_LOADING = 6,
    SKR_ASYNC_IO_STATUS_READ_OK = 7,
    SKR_ASYNC_IO_STATUS_COUNT,
    SKR_ASYNC_IO_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncIOStatus;

typedef enum SkrAsyncServicePriority
{
    SKR_ASYNC_SERVICE_PRIORITY_URGENT = 0,
    SKR_ASYNC_SERVICE_PRIORITY_NORMAL = 1,
    SKR_ASYNC_SERVICE_PRIORITY_LOW = 2,
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

typedef enum ESkrIOStage
{
    SKR_IO_STAGE_NONE,
    SKR_IO_STAGE_ENQUEUED,
    SKR_IO_STAGE_SORTING,
    SKR_IO_STAGE_RESOLVING,
    SKR_IO_STAGE_CHUNKING,
    SKR_IO_STAGE_LOADING,
    SKR_IO_STAGE_DECOMPRESSIONG,
    SKR_IO_STAGE_COMPLETED,
    SKR_IO_STAGE_MAX_ENUM = UINT32_MAX
} ESkrIOStage;

typedef enum ESkrIOFinishPoint
{
    SKR_IO_FINISH_POINT_COMPLETE,
    SKR_IO_FINISH_POINT_CANCEL,
    SKR_IO_FINISH_POINT_ERROR,
    SKR_IO_FINISH_POINT_COUNT = 3,
    SKR_IO_FINISH_POINT_MAX_ENUM = UINT32_MAX
} ESkrIOFinishPoint;

typedef enum ESkrIOEndpoint
{
    SKR_IO_ENDPOINT_DISK,
    SKR_IO_ENDPOINT_RAM,
    SKR_IO_ENDPOINT_VRAM,
    SKR_IO_ENDPOINT_MAX_ENUM = UINT32_MAX
} ESkrIOEndpoint;

typedef struct skr_guid_t skr_io_decompress_method_t;
typedef struct skr_guid_t skr_io_request_resolve_pass_t;

typedef struct skr_io_future_t {
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
} skr_io_future_t;
typedef struct skr_async_ram_destination_t skr_io_ram_buffer_t;

typedef struct skr_async_ram_destination_t {
    uint8_t* bytes SKR_IF_CPP(= nullptr);
    uint64_t size SKR_IF_CPP(= 0);
} skr_async_ram_destination_t;

typedef struct skr_io_block_t
{
    uint64_t offset SKR_IF_CPP(= 0);
    uint64_t size SKR_IF_CPP(= 0);
} skr_io_block_t;

typedef struct skr_io_compressed_block_t
{
    uint64_t offset SKR_IF_CPP(= 0);
    uint64_t compressed_size SKR_IF_CPP(= 0);
    uint64_t uncompressed_size SKR_IF_CPP(= 0);
    skr_io_decompress_method_t decompress_method;
} skr_io_compressed_block_t;

typedef void (*skr_io_callback_t)(skr_io_future_t* future, skr_io_request_t* request, void* data);
typedef struct skr_io_request_t
{
    const char8_t* path SKR_IF_CPP(= nullptr);
    const skr_io_file_handle file SKR_IF_CPP(= nullptr); 

    skr_io_block_t block;
    // skr_io_compressed_block_t compressed_block;

    SkrAsyncServicePriority priority SKR_IF_CPP(= SKR_ASYNC_SERVICE_PRIORITY_NORMAL);
    float sub_priority SKR_IF_CPP(= 0.f); /*0.f ~ 1.f*/

    skr_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];
} skr_io_request_t;

typedef struct skr_io_batch_t
{
    const skr_io_request_t* requests;
    uint32_t request_count;
} skr_io_batch_t;

typedef struct skr_ram_io_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    bool lockless SKR_IF_CPP(= true);
    SkrAsyncServiceSortMethod sort_method SKR_IF_CPP(= SKR_ASYNC_SERVICE_SORT_METHOD_NEVER);
    SkrAsyncServiceSleepMode sleep_mode SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR);
    skr_job_queue_id job_queue SKR_IF_CPP(= nullptr);
} skr_ram_io_service_desc_t;

#ifdef __cplusplus
#include "containers/vector.hpp"
#include "containers/concurrent_queue.h"
#include <EASTL/fixed_vector.h>

namespace skr {
namespace io {

// io flow
// 1. enqueue requests to service
// 2. sort raw requests
// 3. resolve requests to pending raw request array
//  3.1 package parsing happens here.
// 4. chunk pending raw requests to block slices
// 5. dispatch I/O blocks to drives (+allocate & cpy to raw)
// 6. do uncompress works (+allocate & cpy to uncompressed)
// 7. 2 kinds of callbacks are provided
//  7.1 inplace callbacks are executed in the I/O thread/workers
//  7.2 finish callbacks are polled & executed by usr threads
struct RAMService2;

// 1~2 
#pragma region Enqueue & Sort

using IOBlock = skr_io_block_t;
using IOCompressedBlock = skr_io_compressed_block_t;
using IORequest = skr_io_request_t;
using IOBatch = skr_io_batch_t;

#pragma endregion

#pragma region Resolve

#pragma endregion

#pragma region Chunking

struct RUNTIME_API IORequstChunk
{
    IORequstChunk() SKR_NOEXCEPT;
    ~IORequstChunk() SKR_NOEXCEPT;

    eastl::fixed_vector<skr_io_block_t, 1> blocks;
    eastl::fixed_vector<skr_io_compressed_block_t, 1> compressed_blocks;
};

#pragma endregion

#pragma region Dispatch & Uncompress

#pragma endregion

struct RUNTIME_API RAMService
{
    [[nodiscard]] static skr_io_ram_service_t* create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    static void destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT;

    // we do not lock an ioService to a single vfs, but for better bandwidth use and easier profiling
    // it's recommended to make a unique relevance between ioService & vfs（or vfses share a single I/O hardware)
    virtual void request(skr_vfs_t*, const skr_io_request_t* request, skr_io_future_t* future, skr_async_ram_destination_t* dst) SKR_NOEXCEPT = 0;

    // emplace a cancel **command** to ioService thread
    // it's recommended to use this under lockless mode
    virtual void cancel(skr_io_future_t* future) SKR_NOEXCEPT = 0;

    // stop service and hang up underground thread
    virtual void stop(bool wait_drain = false) SKR_NOEXCEPT = 0;

    // start & run service
    virtual void run() SKR_NOEXCEPT = 0;

    // block & finish up all requests
    virtual void drain(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) SKR_NOEXCEPT = 0;

    // set sleep time when io queue is detected to be idle
    virtual void set_sleep_time(uint32_t ms) SKR_NOEXCEPT = 0;

    // finish callbacks are polled & executed by usr threads
    virtual void poll_finish_callbacks() SKR_NOEXCEPT = 0;

    // get service status (sleeping or running)
    virtual SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT = 0;

    virtual ~RAMService() SKR_NOEXCEPT = default;
    RAMService() SKR_NOEXCEPT = default;
};
} // namespace io
} // namespace skr

#endif