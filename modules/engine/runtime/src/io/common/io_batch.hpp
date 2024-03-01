#pragma once
#include "SkrRT/io/io.h"
#include "pool.hpp"
#include "SkrOS/thread.h"
#include "SkrBase/misc/defer.hpp"
#include "SkrContainers/vector.hpp"

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
        SKR_DEFER( { skr_rw_mutex_release_r(&rw_lock); });
        return {requests.data(), requests.size()};
    }

    void set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT { priority = pri; }
    SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT { return priority; }

    const bool can_use_dstorage = true; // TODO: make it configurable

protected:
    friend struct RunnerBase;
    void addRequest(IORequestId rq) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&rw_lock);
        SKR_DEFER( { skr_rw_mutex_release_w(&rw_lock); });
        requests.add(rq);
    }

    void removeCancelledRequest(IORequestId rq) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&rw_lock);
        SKR_DEFER( { skr_rw_mutex_release_w(&rw_lock); });
        requests.remove_all_if([rq](IORequestId r) { return r == rq; });
    }

private:
    SkrAsyncServicePriority priority;
    SRWMutex rw_lock;
    skr::Vector<IORequestId> requests;

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
using IOBatchArray = skr::Vector<BatchPtr>;

} // namespace io
} // namespace skr