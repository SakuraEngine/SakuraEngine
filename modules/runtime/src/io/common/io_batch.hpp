#pragma once
#include "io/io.h"
#include "pool.hpp"
#include "platform/thread.h"
#include "misc/defer.hpp"
#include "containers/vector.hpp"
#include <EASTL/fixed_vector.h>

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
        skr_rw_mutex_acquire_r(&rw_lock);
        SKR_DEFER( { skr_rw_mutex_release(&rw_lock); });
        return requests;
    }

    void set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT { priority = pri; }
    SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT { return priority; }

protected:
    friend struct RunnerBase;
    void addRequest(IORequestId rq) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&rw_lock);
        SKR_DEFER( { skr_rw_mutex_release(&rw_lock); });
        requests.push_back(rq);
    }

    void removeCancelledRequest(IORequestId rq) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&rw_lock);
        SKR_DEFER( { skr_rw_mutex_release(&rw_lock); });
        auto fnd = eastl::remove_if(
            requests.begin(), requests.end(), [rq](IORequestId r) { return r == rq; });
        cancelled_requests.emplace_back(*fnd);
        requests.erase(fnd, requests.end());
    }

private:
    SkrAsyncServicePriority priority;
    SRWMutex rw_lock;
    eastl::fixed_vector<IORequestId, 4> requests;
    eastl::fixed_vector<IORequestId, 1> cancelled_requests;

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
    IOBatchBase(ISmartPoolPtr<IIOBatch> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
        : sequence(sequence), pool(pool), service(service)
    {
        skr_init_rw_mutex(&rw_lock);
    }

    ~IOBatchBase() SKR_NOEXCEPT
    {
        skr_destroy_rw_mutex(&rw_lock);
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