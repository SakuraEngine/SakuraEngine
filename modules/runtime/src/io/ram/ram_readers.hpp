#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "ram_service.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {
struct RAMService;

template<typename I = IIORequestProcessor>
struct RAMReaderBase : public IIOReader<I>
{
    IO_RESOLVER_OBJECT_BODY
public:
    RAMReaderBase(RAMService* service) SKR_NOEXCEPT 
        : service(service) 
    {
        init_counters();
    }
    virtual ~RAMReaderBase() SKR_NOEXCEPT {}

    void awakeService()
    {
        service->runner.awake();
    }

protected:
    RAMService* service = nullptr;
};

struct VFSRAMReader final : public RAMReaderBase<IIORequestProcessor>
{
    VFSRAMReader(RAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT 
        : RAMReaderBase(service), job_queue(job_queue) 
    {

    }
    ~VFSRAMReader() SKR_NOEXCEPT {}

    uint64_t get_prefer_batch_size() const SKR_NOEXCEPT;
    bool fetch(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT;
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return job_queue; }
    void dispatchFunction(SkrAsyncServicePriority priority, const IORequestId& request) SKR_NOEXCEPT;

    skr::JobQueue* job_queue = nullptr;
    IORequestQueue fetched_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IORequestQueue loaded_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<skr::IFuture<bool>*> loaded_futures[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

} // namespace io
} // namespace skr

#include "SkrRT/platform/dstorage.h"
#include <EASTL/fixed_vector.h>

namespace skr {
namespace io {

struct RUNTIME_API DStorageEvent : public skr::SInterface
{
    IO_RC_OBJECT_BODY
public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<DStorageEvent*>(ptr);
            p->pool->deallocate(p); 
        };
    }

    bool okay() { return skr_dstorage_event_test(event); }

    friend struct SmartPool<DStorageEvent, DStorageEvent>;
protected:
    DStorageEvent(ISmartPoolPtr<DStorageEvent> pool, SkrDStorageQueueId queue) 
        : queue(queue), pool(pool)
    {
        if (!event)
            event = skr_dstorage_queue_create_event(queue);
    }
    ~DStorageEvent() SKR_NOEXCEPT 
    {
        if (event)
        {
            skr_dstorage_queue_free_event(queue, event);
        }
        queue = nullptr;
    }
    friend struct DStorageRAMReader;
    eastl::fixed_vector<IOBatchId, 32> batches;
    SkrDStorageQueueId queue = nullptr;
    ISmartPoolPtr<DStorageEvent> pool = nullptr;
    SkrDStorageEventId event = nullptr;
};

struct RUNTIME_API DStorageRAMReader final 
    : public RAMReaderBase<IIOBatchProcessor>
{
    DStorageRAMReader(RAMService* service) SKR_NOEXCEPT;
    ~DStorageRAMReader() SKR_NOEXCEPT;

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT;
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }

    void enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    SkrDStorageQueueId f2m_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SkrDStorageQueueId m2m_queue;
    
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue loaded_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    eastl::vector<skr::SObjectPtr<DStorageEvent>> submitted[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

    SmartPoolPtr<DStorageEvent> events[SKR_ASYNC_SERVICE_PRIORITY_COUNT] = { nullptr, nullptr, nullptr };
};

} // namespace io
} // namespace skr