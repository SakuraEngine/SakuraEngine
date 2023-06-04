#pragma once
#include "io/io.h"
#include "ram_request.hpp"
#include "ram_service.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {
struct RAMService;

struct RAMReaderBase : public IIOReader
{
public:
    RAMReaderBase(RAMService* service) SKR_NOEXCEPT : service(service) {}
    virtual ~RAMReaderBase() SKR_NOEXCEPT {}
protected:
    RAMService* service = nullptr;

public:
    uint32_t add_refcount() 
    { 
        return 1 + skr_atomicu32_add_relaxed(&rc, 1); 
    }
    uint32_t release() 
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }
private:
    SAtomicU32 rc = 0;
};

struct VFSRAMReader final : public RAMReaderBase
{
    VFSRAMReader(RAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT 
        : RAMReaderBase(service), job_queue(job_queue) {}
    ~VFSRAMReader() SKR_NOEXCEPT {}

    uint64_t get_prefer_batch_size() const SKR_NOEXCEPT;
    void fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT;
    void dispatchFunction(IOBatchId batch) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    IORequestId poll_finish(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    skr::JobQueue* job_queue = nullptr;
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IORequestQueue finish_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<skr::IFuture<bool>*> futures[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
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
    IORequestId poll_finish(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
};
*/
} // namespace io
} // namespace skr
#endif