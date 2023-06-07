#pragma once
#include "io_batch.hpp"

namespace skr {
namespace io {
struct RunnerBase;

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

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        queues[priority].enqueue(batch);
        skr_atomic64_add_relaxed(&processed_batch_counts[priority], 1);
        return true;
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

    uint64_t processed_count(SkrAsyncServicePriority priority) const SKR_NOEXCEPT
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

    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT {}
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT {}
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }
    uint64_t processing_count(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return 0; }

protected:
    SAtomic64 processed_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

struct IORequestResolverChain final : public IIORequestResolverChain
{
    IO_RC_OBJECT_BODY
public:
    IORequestResolverChain() SKR_NOEXCEPT;

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        fetched_batches[priority].enqueue(batch);
        skr_atomic64_add_relaxed(&resolving_counts[priority], 1);
        return true;
    }
    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT { return; }
    virtual bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }

    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
    {
        if (processed_batches[priority].try_dequeue(batch))
        {
            skr_atomic64_add_relaxed(&processed_batch_counts[priority], -1);
            return batch.get();
        }
        return false;
    }

    virtual uint64_t processing_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomic64_load_acquire(&resolving_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
            {
                count += skr_atomic64_load_acquire(&resolving_counts[i]);
            }
            return count;
        }
    }

    virtual uint64_t processed_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
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

    RunnerBase* runner = nullptr;
private:
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue processed_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomic64 resolving_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomic64 processed_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

public:
    IORequestResolverChain(IORequestResolverId resolver) SKR_NOEXCEPT
        : IORequestResolverChain()
    {
        if (resolver)
        {
            chain.emplace_back(resolver);
        }
    }
    SObjectPtr<IIORequestResolverChain> then(IORequestResolverId resolver) SKR_NOEXCEPT
    {
        chain.emplace_back(resolver);
        return this;
    }
private:
    skr::vector<IORequestResolverId> chain;
};

} // namespace io
} // namespace skr