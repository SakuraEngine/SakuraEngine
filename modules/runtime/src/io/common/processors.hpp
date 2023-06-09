#pragma once
#include "io_batch.hpp"

namespace skr {
namespace io {
struct RunnerBase;

struct IOBatchBuffer : public IIOBatchProcessor
{
    IO_RC_OBJECT_BODY
public:
    IOBatchBuffer() SKR_NOEXCEPT 
    {
        for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
        {
            skr_atomic64_store_relaxed(&counts[i], 0);
        }
    }

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        queues[priority].enqueue(batch);
        skr_atomic64_add_relaxed(&counts[priority], 1);
        return true;
    }

    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
    {
        if (queues[priority].try_dequeue(batch))
        {
            skr_atomic64_add_relaxed(&counts[priority], -1);
            return batch.get();
        }
        return false;
    }

    uint64_t processed_count(SkrAsyncServicePriority priority) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomic64_load_acquire(&counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
            {
                count += skr_atomic64_load_acquire(&counts[i]);
            }
            return count;
        }
    }

    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT {}
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT {}
    bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }
    uint64_t processing_count(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return 0; }

protected:
    SAtomic64 counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};
using IOBatchBufferId = SObjectPtr<IOBatchBuffer>;

#define IO_RESOLVER_OBJECT_BODY \
    IO_RC_OBJECT_BODY\
    uint64_t processing_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT\
    {\
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)\
            return skr_atomic64_load_acquire(&processing_counts[priority]);\
        else\
        {\
            uint64_t count = 0;\
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)\
            {\
                count += skr_atomic64_load_acquire(&processing_counts[i]);\
            }\
            return count;\
        }\
    }\
    uint64_t processed_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT\
    {\
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)\
            return skr_atomic64_load_acquire(&processed_counts[priority]);\
        else\
        {\
            uint64_t count = 0;\
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)\
                count += skr_atomic64_load_acquire(&processed_counts[i]);\
            return count;\
        }\
    }\
    inline void init_counters() \
    { \
        for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)\
        {\
            skr_atomic64_store_relaxed(&processing_counts[i], 0);\
            skr_atomic64_store_relaxed(&processed_counts[i], 0);\
        }\
    }\
    inline void inc_processing(SkrAsyncServicePriority priority, int64_t n = 1) { skr_atomic64_add_relaxed(&processing_counts[priority], (n)); }\
    inline void dec_processing(SkrAsyncServicePriority priority, int64_t n = 1) { skr_atomic64_add_relaxed(&processing_counts[priority], -(n)); }\
    inline void inc_processed(SkrAsyncServicePriority priority, int64_t n = 1) { skr_atomic64_add_relaxed(&processed_counts[priority], (n)); }\
    inline void dec_processed(SkrAsyncServicePriority priority, int64_t n = 1) { skr_atomic64_add_relaxed(&processed_counts[priority], -(n)); }\
private:\
    SAtomic64 processing_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT] = { 0, 0, 0 };\
    SAtomic64 processed_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT] = { 0, 0, 0 };

struct IORequestResolverChain final : public IIORequestResolverChain
{
    IO_RESOLVER_OBJECT_BODY
public:
    IORequestResolverChain() SKR_NOEXCEPT;

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        fetched_batches[priority].enqueue(batch);
        inc_processing(priority);
        return true;
    }
    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT { return; }
    virtual bool is_async(SkrAsyncServicePriority priority) const SKR_NOEXCEPT { return false; }

    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
    {
        if (processed_batches[priority].try_dequeue(batch))
        {
            dec_processed(priority);
            return batch.get();
        }
        return false;
    }

    RunnerBase* runner = nullptr;
private:
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue processed_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

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