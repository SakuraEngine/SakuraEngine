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

struct IOBatchBuffer : public IIOBatchBuffer
{
    IO_RC_OBJECT_BODY
public:
    IOBatchBuffer() SKR_NOEXCEPT 
    {
        for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
        {
            skr_atomic64_store_relaxed(&processed_batch_counts[i], 0);
        }
    }

    uint64_t get_prefer_batch_size() const SKR_NOEXCEPT { return UINT64_MAX; }

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        queues[priority].enqueue(batch);
        skr_atomic64_add_relaxed(&processed_batch_counts[priority], 1);
        return true;
    }

    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT {}
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT {}

    virtual bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT
    {
        return false;
    }

    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
    {
        if (queues[priority].try_dequeue(batch))
        {
            skr_atomic64_add_relaxed(&processed_batch_counts[priority], -1);
            return batch.get();
        }
        return false;
    }

    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT
    {
        return false;
    }

    uint64_t pending_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        return 0;
    }

    uint64_t processed_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomic64_load_acquire(&processed_batch_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
            {
                count += skr_atomic64_load_acquire(&processed_batch_counts[i]);
            }
            return count;
        }
    }
protected:
    SAtomic64 processed_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
public:
    IOBatchQueue queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};
} // namespace io
} // namespace skr