#pragma once
#include "pool.hpp"
#include "containers/vector.hpp"
#include <EASTL/fixed_vector.h>
#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

struct IOBatchBase : public IIOBatch
{
    IO_RC_OBJECT_BODY
public:
    void reserve(uint64_t n) SKR_NOEXCEPT
    {
        requests.reserve(n);
    }

    skr::span<IORequestId> get_requests() SKR_NOEXCEPT
    {
        return requests;
    }

    void set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT { priority = pri; }
    SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT { return priority; }

protected:
    SkrAsyncServicePriority priority;
    eastl::fixed_vector<IORequestId, 1> requests;

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<IOBatchBase*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<IOBatchBase, IIOBatch>;

protected:
    IOBatchBase(ISmartPoolPtr<IIOBatch> pool, IIOService* service, const uint64_t sequence) 
        : sequence(sequence), pool(pool), service(service)
    {

    }
    
    const uint64_t sequence;
    ISmartPoolPtr<IIOBatch> pool = nullptr;
    IIOService* service = nullptr;
};

using BatchPtr = skr::SObjectPtr<IIOBatch>;
using IOBatchQueue = IOConcurrentQueue<BatchPtr>;  
using IOBatchArray = skr::vector<BatchPtr>;

} // namespace io
} // namespace skr