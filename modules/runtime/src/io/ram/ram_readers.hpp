#pragma once
#include "io/io.h"
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

    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT
    {
        return true;
    }

    void dispatchFunction(SkrAsyncServicePriority priority, const IORequestId& request) SKR_NOEXCEPT;

    skr::JobQueue* job_queue = nullptr;
    IORequestQueue fetched_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IORequestQueue loaded_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<skr::IFuture<bool>*> loaded_futures[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

} // namespace io
} // namespace skr

#ifdef _WIN32
// #include "platform/dstorage.h"

namespace skr {
namespace io {
/*
struct DStorageRAMReader final : public RAMReaderBase
{
    DStorageRAMReader(RAMService* service) : RAMReaderBase(service) {}

    void fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT;
    void sort(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    IORequestId poll_loaded_request(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    IOBatchId poll_loaded_batch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
};
*/
} // namespace io
} // namespace skr
#endif