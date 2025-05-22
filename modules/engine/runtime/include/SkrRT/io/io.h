#pragma once
#include "SkrBase/atomic/atomic.h"
#include "SkrCore/async/async_service.h"
#ifdef __cplusplus
    #include "SkrCore/memory/rc.hpp"
    #include "SkrContainers/span.hpp"
#endif

#define SKR_IO_SERVICE_MAX_TASK_COUNT 32
#define SKR_ASYNC_SERVICE_SLEEP_TIME_MAX UINT32_MAX

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)
SKR_DECLARE_TYPE_ID_FWD(skr::io, IIORequest, skr_io_request)
struct skr_vfs_t;

typedef struct skr_vfile_t skr_io_file_t;
typedef skr_io_file_t*     skr_io_file_handle;

typedef enum ESkrIOStage
{
    SKR_IO_STAGE_NONE,
    SKR_IO_STAGE_ENQUEUED,
    SKR_IO_STAGE_RESOLVING,
    SKR_IO_STAGE_LOADING,
    SKR_IO_STAGE_LOADED,
    SKR_IO_STAGE_DECOMPRESSIONG,
    SKR_IO_STAGE_DECOMPRESSED,
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
    SKR_IO_FINISH_POINT_COUNT    = 3,
    SKR_IO_FINISH_POINT_MAX_ENUM = UINT32_MAX
} ESkrIOFinishPoint;

typedef skr_guid_t skr_io_decompress_method_t;
typedef skr_guid_t skr_io_request_resolve_pass_t;

typedef struct skr_io_future_t {
    SAtomicU32 status         SKR_IF_CPP(= 0);
    SAtomicU32 request_cancel SKR_IF_CPP(= 0);
#ifdef __cplusplus
    SKR_RUNTIME_API bool        is_ready() const SKR_NOEXCEPT;
    SKR_RUNTIME_API bool        is_enqueued() const SKR_NOEXCEPT;
    SKR_RUNTIME_API bool        is_cancelled() const SKR_NOEXCEPT;
    SKR_RUNTIME_API bool        is_loading() const SKR_NOEXCEPT;
    SKR_RUNTIME_API ESkrIOStage get_status() const SKR_NOEXCEPT;
#endif
} skr_io_future_t;

typedef struct skr_io_block_t {
    uint64_t offset SKR_IF_CPP(= 0);
    uint64_t size   SKR_IF_CPP(= 0);
} skr_io_block_t;

typedef struct skr_io_compressed_block_t {
    uint64_t offset            SKR_IF_CPP(= 0);
    uint64_t compressed_size   SKR_IF_CPP(= 0);
    uint64_t uncompressed_size SKR_IF_CPP(= 0);
    skr_io_decompress_method_t decompress_method;
} skr_io_compressed_block_t;

typedef void (*skr_io_callback_t)(skr_io_future_t* future, skr_io_request_t* request, void* data);

#ifdef __cplusplus

namespace skr
{
namespace io
{

using IOBlock           = skr_io_block_t;
using IOCompressedBlock = skr_io_compressed_block_t;
using IOFuture          = skr_io_future_t;
using IOCallback        = skr_io_callback_t;
using IOResultId        = RC<skr::IRCAble>;
struct IORequestComponent;
struct IIOService;

struct SKR_RUNTIME_API IIORequest : public skr::IRCAble {
    virtual ~IIORequest() SKR_NOEXCEPT;

    virtual IORequestComponent*       get_component(skr_guid_t tid) SKR_NOEXCEPT       = 0;
    virtual const IORequestComponent* get_component(skr_guid_t tid) const SKR_NOEXCEPT = 0;
    virtual IIOService*               get_service() const SKR_NOEXCEPT                 = 0;

    #pragma region PathSrcComponent
    virtual void           set_vfs(skr_vfs_t* vfs) SKR_NOEXCEPT       = 0;
    virtual void           set_path(const char8_t* path) SKR_NOEXCEPT = 0;
    virtual const char8_t* get_path() const SKR_NOEXCEPT              = 0;
    #pragma endregion

    #pragma region IOStatusComponent
    virtual void            use_async_complete() SKR_NOEXCEPT = 0;
    virtual void            use_async_cancel() SKR_NOEXCEPT   = 0;
    virtual const IOFuture* get_future() const SKR_NOEXCEPT   = 0;

    virtual void add_callback(ESkrIOStage stage, IOCallback callback, void* data) SKR_NOEXCEPT              = 0;
    virtual void add_finish_callback(ESkrIOFinishPoint point, IOCallback callback, void* data) SKR_NOEXCEPT = 0;
    #pragma endregion
};
using IORequestId = RC<IIORequest>;

struct SKR_RUNTIME_API IIOBatch : public skr::IRCAble {
    SKR_RC_DELETER_INTERFACE()

    virtual void                   reserve(uint64_t size) SKR_NOEXCEPT                             = 0;
    virtual IOResultId             add_request(IORequestId request, IOFuture* future) SKR_NOEXCEPT = 0;
    virtual skr::span<IORequestId> get_requests() SKR_NOEXCEPT                                     = 0;

    virtual void                    set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT = 0;
    virtual SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT                      = 0;
};
using IOBatchId = RC<IIOBatch>;

struct SKR_RUNTIME_API IIORequestResolver : public skr::IRCAble {
    virtual void resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT;

    virtual ~IIORequestResolver() SKR_NOEXCEPT;
};
using IORequestResolverId = RC<IIORequestResolver>;

struct SKR_RUNTIME_API IIOProcessor : public skr::IRCAble {
    virtual void     dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT                                                  = 0;
    virtual void     recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT                                                   = 0;
    virtual bool     is_async(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT         = 0;
    virtual uint64_t processing_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT = 0;
    virtual uint64_t processed_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT  = 0;

    virtual ~IIOProcessor() SKR_NOEXCEPT;
    IIOProcessor() SKR_NOEXCEPT = default;
};

struct SKR_RUNTIME_API IIOBatchProcessor : public IIOProcessor {
    virtual bool     fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT                 = 0;
    virtual bool     poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT = 0;
    virtual uint64_t get_prefer_batch_size() const SKR_NOEXCEPT;
    // virtual uint64_t get_prefer_batch_count() const SKR_NOEXCEPT;

    virtual ~IIOBatchProcessor() SKR_NOEXCEPT;
    IIOBatchProcessor() SKR_NOEXCEPT = default;
};
using IOBatchProcessorId = RC<IIOBatchProcessor>;

struct SKR_RUNTIME_API IIORequestProcessor : public IIOProcessor {
    virtual bool fetch(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT                   = 0;
    virtual bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT = 0;

    virtual ~IIORequestProcessor() SKR_NOEXCEPT;
    IIORequestProcessor() SKR_NOEXCEPT = default;
};
using IORequestProcessorId = RC<IIORequestProcessor>;

struct SKR_RUNTIME_API IIORequestResolverChain : public IIOBatchProcessor {
    static RC<IIORequestResolverChain>  Create(IORequestResolverId resolver = nullptr) SKR_NOEXCEPT;
    virtual RC<IIORequestResolverChain> then(IORequestResolverId resolver) SKR_NOEXCEPT = 0;

    virtual ~IIORequestResolverChain() SKR_NOEXCEPT;
};
using IORequestResolverChainId = RC<IIORequestResolverChain>;

template <typename I = IIORequestProcessor>
struct SKR_RUNTIME_API IIOReader : public I {
    virtual ~IIOReader() SKR_NOEXCEPT;
    IIOReader() SKR_NOEXCEPT = default;
};
template <typename I = IIORequestProcessor>
using IOReaderId = RC<IIOReader<I>>;

template <typename I = IIORequestProcessor>
struct SKR_RUNTIME_API IIODecompressor : public I {
    virtual ~IIODecompressor() SKR_NOEXCEPT;
    IIODecompressor() SKR_NOEXCEPT = default;
};
template <typename I = IIORequestProcessor>
using IODecompressorId = RC<IIODecompressor<I>>;

struct SKR_RUNTIME_API IIOService {
    // add a resolver to service
    // virtual void set_resolvers(IORequestResolverChainId chain) SKR_NOEXCEPT = 0;

    // cancel request
    virtual void cancel(IOFuture* future) SKR_NOEXCEPT = 0;

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
    IIOService() SKR_NOEXCEPT          = default;
};

} // namespace io
} // namespace skr

#endif