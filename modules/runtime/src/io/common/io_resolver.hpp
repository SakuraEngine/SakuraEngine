#pragma once
#include "io/io.h"
#include "containers/vector.hpp"
#include "io_batch.hpp"
#include "io_request.hpp"

namespace skr {
namespace io {
struct RunnerBase;

struct IORequestResolverBase : public IIORequestResolver
{
    IO_RC_OBJECT_BODY
public:

};

struct IORequestResolverChain final : public IIORequestResolverChain
{
    IO_RC_OBJECT_BODY
public:
    IORequestResolverChain() SKR_NOEXCEPT;

    uint64_t get_prefer_batch_size() const SKR_NOEXCEPT { return UINT64_MAX; }

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        fetched_batches[priority].enqueue(batch);
        skr_atomic64_add_relaxed(&pending_batch_counts[priority], 1);
        return true;
    }

    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
    {
        return;
    }

    virtual bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT
    {
        return false;
    }

    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
    {
        if (processed_batches[priority].try_dequeue(batch))
        {
            skr_atomic64_add_relaxed(&processed_batch_counts[priority], -1);
            return batch.get();
        }
        return false;
    }

    virtual bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT
    {
        return false;
    }

    virtual uint64_t pending_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomic64_load_acquire(&pending_batch_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
            {
                count += skr_atomic64_load_acquire(&pending_batch_counts[i]);
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
    SAtomic64 pending_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
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

/*
struct IOBatchProcessorChain : public IIOBatchProcessorChain
{
    IO_RC_OBJECT_BODY
public:
    virtual uint64_t get_prefer_batch_size() const SKR_NOEXCEPT { return UINT64_MAX; }
    virtual bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        if (!chain.size()) return false;
        return chain[0]->fetch(priority, batch);
    }

    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT = 0;
    
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT = 0;
    virtual bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT = 0;
    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT = 0;

public:
    IOBatchProcessorChain(IOBatchProcessorId processor) SKR_NOEXCEPT
    {
        if (processor)
        {
            chain.emplace_back(processor);
        }
    }
    SObjectPtr<IIOBatchProcessorChain> then(IOBatchProcessorId processor) SKR_NOEXCEPT
    {
        chain.emplace_back(processor);
        return this;
    }
private:
    skr::vector<IOBatchProcessorId> chain;
};
*/

} // namespace io
} // namespace skr