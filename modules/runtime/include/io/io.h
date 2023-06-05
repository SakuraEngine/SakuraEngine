#pragma once
#include "misc/types.h"
#include "platform/atomic.h"

#define SKR_IO_SERVICE_MAX_TASK_COUNT 32
#define SKR_ASYNC_SERVICE_SLEEP_TIME_MAX UINT32_MAX

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)
SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service)
SKR_DECLARE_TYPE_ID_FWD(skr::io, IIORequest, skr_io_request)
struct skr_vfs_t;

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

typedef enum SkrAsyncServicePriority
{
    SKR_ASYNC_SERVICE_PRIORITY_URGENT = 0,
    SKR_ASYNC_SERVICE_PRIORITY_NORMAL = 1,
    SKR_ASYNC_SERVICE_PRIORITY_LOW = 2,
    SKR_ASYNC_SERVICE_PRIORITY_COUNT = 3,
    SKR_ASYNC_SERVICE_PRIORITY_MAX_ENUM = INT32_MAX
} SkrAsyncServicePriority;

typedef enum ESkrIOStage
{
    SKR_IO_STAGE_NONE,
    SKR_IO_STAGE_ENQUEUED,
    SKR_IO_STAGE_RESOLVING,
    SKR_IO_STAGE_LOADING,
    SKR_IO_STAGE_LOADED,
    SKR_IO_STAGE_DECOMPRESSIONG,
    SKR_IO_STAGE_COMPLETED,
    SKR_IO_STAGE_CANCELLED,

    SKR_IO_STAGE_COUNT,
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
    SAtomicU32 status SKR_IF_CPP(= 0);
    SAtomicU32 request_cancel SKR_IF_CPP(= 0);
#ifdef __cplusplus
    RUNTIME_API bool is_ready() const SKR_NOEXCEPT;
    RUNTIME_API bool is_enqueued() const SKR_NOEXCEPT;
    RUNTIME_API bool is_cancelled() const SKR_NOEXCEPT;
    RUNTIME_API bool is_loading() const SKR_NOEXCEPT;
    RUNTIME_API bool is_vram_loading() const SKR_NOEXCEPT;
    RUNTIME_API ESkrIOStage get_status() const SKR_NOEXCEPT;
#endif
} skr_io_future_t;

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

typedef struct skr_ram_io_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    skr_job_queue_id io_job_queue SKR_IF_CPP(= nullptr);
    bool trace_log SKR_IF_CPP(= false);
    bool awake_at_request SKR_IF_CPP(= true);
} skr_ram_io_service_desc_t;

#ifdef __cplusplus
#include "containers/sptr.hpp"

namespace skr {
namespace io {

// io flow
// 1. Enqueue requests(batches) to service
// 2. Sort raw requests
// 3. Resolve requests to pending raw request array
//  3.1 package parsing happens here.
//  3.2 you can resolve paths, open files, allocate buffers, etc.
// 4. Dispatch I/O blocks to drives (+allocate & cpy to raw)
// 5. Do uncompress works (+allocate & cpy to uncompressed)
// 6. Two kinds of callbacks are provided
//  6.1 inplace callbacks are executed in the I/O thread/workers
//  6.2 finish callbacks are polled & executed by usr threads
struct IRAMService;

using IOBlock = skr_io_block_t;
using IOCompressedBlock = skr_io_compressed_block_t;
struct RUNTIME_API IIORequest : public skr::SInterface
{
    virtual void set_vfs(skr_vfs_t* vfs) SKR_NOEXCEPT = 0;
    virtual void set_path(const char8_t* path) SKR_NOEXCEPT = 0;
    virtual const char8_t* get_path() const SKR_NOEXCEPT = 0;
    
    virtual void open_file() SKR_NOEXCEPT = 0; 

    virtual uint64_t get_fsize() const SKR_NOEXCEPT = 0;

    virtual void set_sub_priority(float sub_pri) SKR_NOEXCEPT = 0; 
    virtual float get_sub_priority() const SKR_NOEXCEPT = 0;

    virtual void add_callback(ESkrIOStage stage, skr_io_callback_t callback, void* data) SKR_NOEXCEPT = 0;
    virtual void add_finish_callback(ESkrIOFinishPoint point, skr_io_callback_t callback, void* data) SKR_NOEXCEPT = 0;

    virtual skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT = 0;
    virtual void add_block(const skr_io_block_t& block) SKR_NOEXCEPT = 0;
    virtual void reset_blocks() SKR_NOEXCEPT = 0;

    virtual skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT = 0;
    virtual void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT = 0;
    virtual void reset_compressed_blocks() SKR_NOEXCEPT = 0;
};
using IORequestId = SObjectPtr<IIORequest>;
using IOResultId = SObjectPtr<skr::SInterface>;

struct RUNTIME_API IIOBatch : public skr::SInterface
{
    virtual void reserve(uint64_t size) SKR_NOEXCEPT = 0;
    virtual IOResultId add_request(IORequestId request, skr_io_future_t* future = nullptr) SKR_NOEXCEPT = 0;
    virtual skr::span<IORequestId> get_requests() SKR_NOEXCEPT = 0;

    virtual void set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT = 0;
    virtual SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT = 0;
};
using IOBatchId = SObjectPtr<IIOBatch>;

struct RUNTIME_API IIOBatchResolver : public skr::SInterface
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT;

    virtual ~IIOBatchResolver() SKR_NOEXCEPT;
};
using IOBatchResolverId = SObjectPtr<IIOBatchResolver>;

struct RUNTIME_API IIOBatchResolverChain : public skr::SInterface
{
    static SObjectPtr<IIOBatchResolverChain> Create(IOBatchResolverId resolver = nullptr) SKR_NOEXCEPT;
    virtual SObjectPtr<IIOBatchResolverChain> then(IOBatchResolverId resolver) SKR_NOEXCEPT = 0;

    virtual ~IIOBatchResolverChain() SKR_NOEXCEPT;
};
using IOBatchResolverChainId = SObjectPtr<IIOBatchResolverChain>;

struct RUNTIME_API IIOReader : public skr::SInterface
{
    virtual uint64_t get_prefer_batch_size() const SKR_NOEXCEPT = 0;
    virtual void fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT = 0;
    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT = 0;
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT = 0;
    virtual IORequestId poll_finish_request(SkrAsyncServicePriority priority) SKR_NOEXCEPT = 0;
    virtual IOBatchId poll_finish_batch(SkrAsyncServicePriority priority) SKR_NOEXCEPT = 0;

    virtual ~IIOReader() SKR_NOEXCEPT;
    IIOReader() SKR_NOEXCEPT = default;
};
using IOReaderId = SObjectPtr<IIOReader>;

struct RUNTIME_API IIOService
{
    // add a resolver to service
    virtual void set_resolvers(IOBatchResolverChainId chain) SKR_NOEXCEPT = 0;

    // open a request for filling
    [[nodiscard]] virtual IORequestId open_request() SKR_NOEXCEPT = 0;

    // start a request batch
    [[nodiscard]] virtual IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT = 0;

    // cancel request
    virtual void cancel(skr_io_future_t* future) SKR_NOEXCEPT = 0;

    // stop service and hang up underground thread
    virtual void stop(bool wait_drain = false) SKR_NOEXCEPT = 0;

    // start & run service
    virtual void run() SKR_NOEXCEPT = 0;

    // block & finish up all requests
    virtual void drain(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) SKR_NOEXCEPT = 0;

    // set sleep time when io queue is detected to be idle
    virtual void set_sleep_time(uint32_t ms) SKR_NOEXCEPT = 0;

    // get service status (sleeping or running)
    virtual SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT = 0;

    virtual ~IIOService() SKR_NOEXCEPT = default;
    IIOService() SKR_NOEXCEPT = default;
    
    IOBatchResolverId create_file_resolver() SKR_NOEXCEPT;
};

struct RUNTIME_API IRAMIOBuffer : public skr::IBlob
{
    virtual ~IRAMIOBuffer() SKR_NOEXCEPT;
};
using RAMIOBufferId = SObjectPtr<IRAMIOBuffer>;

struct RUNTIME_API IRAMService : public IIOService
{
    [[nodiscard]] static skr_io_ram_service_t* create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    static void destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT;

    virtual RAMIOBufferId request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT = 0;
    
    virtual void request(IOBatchId request) SKR_NOEXCEPT = 0;

    IOBatchResolverId create_iobuffer_resolver() SKR_NOEXCEPT;
    IOBatchResolverId create_chunking_resolver(uint64_t chunk_size = 256 * 1024) SKR_NOEXCEPT;
    void add_default_resolvers() SKR_NOEXCEPT;

    virtual ~IRAMService() SKR_NOEXCEPT = default;
    IRAMService() SKR_NOEXCEPT = default;
};
} // namespace io
} // namespace skr

#endif