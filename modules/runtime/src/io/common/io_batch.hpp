#pragma once
#include "pool.hpp"
#include "containers/vector.hpp"
#include <EASTL/fixed_vector.h>
#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

struct IOBatchBase : public IIOBatch
{
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
    IOBatchBase(ISmartPool<IIOBatch>* pool, const uint64_t sequence) 
        : sequence(sequence), pool(pool) 
    {

    }
    
    const uint64_t sequence;
    ISmartPool<IIOBatch>* pool = nullptr;
};

using BatchPtr = skr::SObjectPtr<IIOBatch>;
using IOBatchQueue = moodycamel::ConcurrentQueue<BatchPtr>;  
using IOBatchArray = skr::vector<BatchPtr>;
} // namespace io
} // namespace skr