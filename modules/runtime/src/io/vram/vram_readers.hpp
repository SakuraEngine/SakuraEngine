#pragma once
#include "SkrRT/io/vram_io.hpp"
#include "vram_service.hpp"
#include <stdint.h>

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

struct CommonVRAMReader final : public VRAMReaderBase<IIORequestProcessor>
{
    CommonVRAMReader(VRAMService* service, RAMService* ram_service) SKR_NOEXCEPT 
        : VRAMReaderBase(service), ram_service(ram_service) 
    {

    }
    ~CommonVRAMReader() SKR_NOEXCEPT {}

    [[nodiscard]] uint8_t* allocate_staging_buffer(uint64_t size) SKR_NOEXCEPT;
    void free_staging_buffer(uint8_t* buffer) SKR_NOEXCEPT;

    uint64_t get_prefer_batch_size() const SKR_NOEXCEPT;
    bool fetch(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT;
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return ram_service; }
    void dispatchFunction(SkrAsyncServicePriority priority, const IORequestId& request) SKR_NOEXCEPT;

    RAMService* ram_service = nullptr;
    IORequestQueue fetched_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IORequestQueue loaded_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<skr::IFuture<bool>*> loaded_futures[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

} // namespace io
} // namespace skr

#include "../dstorage/dstorage_event.hpp"

namespace skr {
namespace io {

struct RUNTIME_API DStorageVRAMReader final 
    : public VRAMReaderBase<IIOBatchProcessor>
{
    DStorageVRAMReader(VRAMService* service) SKR_NOEXCEPT;
    ~DStorageVRAMReader() SKR_NOEXCEPT;

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT;
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }

    void enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    SkrDStorageQueueId f2v_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SkrDStorageQueueId m2v_queue;
    
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue loaded_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    eastl::vector<skr::SObjectPtr<DStorageEvent>> submitted[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

    SmartPoolPtr<DStorageEvent> events[SKR_ASYNC_SERVICE_PRIORITY_COUNT] = { nullptr, nullptr, nullptr };
};

} // namespace io
} // namespace skr