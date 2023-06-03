#pragma once
#include "io/io.h"
#include "ram_request.hpp"
#include "ram_service.hpp"

namespace skr {
namespace io {

struct RAMReaderBase : public IIOReader
{
public:
    RAMReaderBase(RAMServiceImpl* service) SKR_NOEXCEPT : service(service) {}
    virtual ~RAMReaderBase() SKR_NOEXCEPT {}
protected:
    RAMServiceImpl* service = nullptr;

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
    VFSRAMReader(RAMServiceImpl* service) SKR_NOEXCEPT : RAMReaderBase(service) {}
    ~VFSRAMReader() SKR_NOEXCEPT {}

    void fetch(SkrAsyncServicePriority priority, IOBatch batch) SKR_NOEXCEPT;
    void sort(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void resolve(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    IORequest poll_finish(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    IORequestArray ongoing_requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomicU64 ongoing_requests_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

/*
struct DStorageRAMReader final : public RAMReaderBase
{
    DStorageRAMReader(RAMServiceImpl* service) : RAMReaderBase(service) {}

    void fetch(SkrAsyncServicePriority priority, IOBatch batch) SKR_NOEXCEPT;
    void sort(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void resolve(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    IORequest poll_finish(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
};
*/

} // namespace io
} // namespace skr