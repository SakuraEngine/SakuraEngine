#pragma once
#include "SkrRT/platform/atomic.h"
#include "vram_service.hpp"

#include <EASTL/fixed_vector.h>
#include <EASTL/vector_map.h>

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {
struct RAMService;
struct VRAMService;

template<typename I = IIORequestProcessor>
struct VRAMReaderBase : public IIOReader<I>
{
    IO_RESOLVER_OBJECT_BODY
public:
    VRAMReaderBase(VRAMService* service) SKR_NOEXCEPT 
        : service(service) 
    {
        init_counters();
    }
    virtual ~VRAMReaderBase() SKR_NOEXCEPT {}

    void awakeService()
    {
        service->runner.awake();
    }

protected:
    VRAMService* service = nullptr;
};

struct SwapableCmdPool
{
public:
    SwapableCmdPool() SKR_NOEXCEPT;
    ~SwapableCmdPool() SKR_NOEXCEPT;

    struct RC
    {
        RC() SKR_NOEXCEPT;
        RC(CGPUCommandPoolId v, SAtomic32* pRC) SKR_NOEXCEPT;
        ~RC() SKR_NOEXCEPT;
        operator CGPUCommandPoolId() const SKR_NOEXCEPT { return v; }
    private:
        CGPUCommandPoolId v = nullptr;
        SAtomic32* pRC = nullptr;
    };

    void initialize(CGPUQueueId queue) SKR_NOEXCEPT;
    void finalize() SKR_NOEXCEPT;
    RC get() SKR_NOEXCEPT;
    void swap() SKR_NOEXCEPT;

private:
    CGPUCommandPoolId pools[2] = {nullptr, nullptr};
    SAtomic32 rcs[2] = { 0, 0 };
    uint32_t index = 0;
};
using SwapableCmdPoolMap = eastl::vector_map<CGPUQueueId, SwapableCmdPool>;

struct GPUUploadCmd
{
    GPUUploadCmd() SKR_NOEXCEPT;
    GPUUploadCmd(CGPUQueueId queue, IOBatchId batch) SKR_NOEXCEPT;

    void start(SwapableCmdPool& swap_pool) SKR_NOEXCEPT;
    void finish() SKR_NOEXCEPT;

    FORCEINLINE bool is_finished() const SKR_NOEXCEPT { return okay; }
    FORCEINLINE CGPUQueueId get_queue() const SKR_NOEXCEPT { return queue; }
    FORCEINLINE CGPUCommandBufferId get_cmdbuf() const SKR_NOEXCEPT { return cmdbuf; }
    FORCEINLINE CGPUFenceId get_fence() const SKR_NOEXCEPT { return fence; }
    FORCEINLINE IOBatchId get_batch() const SKR_NOEXCEPT { return batch; }

    eastl::fixed_vector<CGPUBufferId, 4> upload_buffers;
protected:
    IOBatchId batch = nullptr;        
    CGPUQueueId queue = nullptr;
    CGPUCommandBufferId cmdbuf = nullptr;
    CGPUFenceId fence = nullptr;
    bool okay = false;
    SwapableCmdPool::RC pool;
};

struct CommonVRAMReader final : public VRAMReaderBase<IIOBatchProcessor>
{
    CommonVRAMReader(VRAMService* service, IRAMService* ram_service) SKR_NOEXCEPT;
    ~CommonVRAMReader() SKR_NOEXCEPT;

    [[nodiscard]] uint8_t* allocate_staging_buffer(uint64_t size) SKR_NOEXCEPT;
    void free_staging_buffer(uint8_t* buffer) SKR_NOEXCEPT;

    bool fetch(SkrAsyncServicePriority priority, IOBatchId request) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT;
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }

protected:
    void addRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void ensureRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void addUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void ensureUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void finishBatch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT;
    bool shouldUseUpload(IIORequest* request) const SKR_NOEXCEPT;

    IRAMService* ram_service = nullptr;
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue processed_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<IOBatchId> ramloading_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<IOBatchId> to_upload_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SwapableCmdPoolMap cmdpools;
    skr::vector<GPUUploadCmd> gpu_uploads[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

} // namespace io
} // namespace skr

#include "../dstorage/dstorage_event.hpp"

namespace skr {
namespace io {

struct SKR_RUNTIME_API DStorageVRAMReader final 
    : public VRAMReaderBase<IIOBatchProcessor>
{
    DStorageVRAMReader(VRAMService* service, CGPUDeviceId device) SKR_NOEXCEPT;
    ~DStorageVRAMReader() SKR_NOEXCEPT;

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT;
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }

    void enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool shouldUseDStorage(IIORequest* request) const SKR_NOEXCEPT;

    SkrDStorageQueueId f2v_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SkrDStorageQueueId m2v_queue;
    
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue processed_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    eastl::vector<skr::SObjectPtr<DStorageEvent>> submitted[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

    SmartPoolPtr<DStorageEvent> events[SKR_ASYNC_SERVICE_PRIORITY_COUNT] = { nullptr, nullptr, nullptr };
};

} // namespace io
} // namespace skr