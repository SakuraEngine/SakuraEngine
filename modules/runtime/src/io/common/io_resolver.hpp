#pragma once
#include "io/io.h"
#include "containers/vector.hpp"
#include "io_batch.hpp"
#include "io_request.hpp"

namespace skr {
namespace io {
struct RunnerBase;

struct IOBatchResolverBase : public IIOBatchResolver
{
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

struct IOBatchResolverChain final : public IIOBatchResolverChain
{
    uint64_t get_prefer_batch_size() const SKR_NOEXCEPT { return UINT64_MAX; }

    bool fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
    {
        fetched_batches[priority].enqueue(batch);
        return true;
    }

    virtual void dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    virtual void recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
    {
        return;
    }

    virtual IORequestId poll_processed_request(SkrAsyncServicePriority priority) SKR_NOEXCEPT
    {
        SKR_ASSERT(false && "Not implemented");
        return nullptr;
    }

    virtual IOBatchId poll_processed_batch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
    {
        IOBatchId batch;
        if (resolved_batches[priority].try_dequeue(batch))
        {
            return batch;
        }
        return nullptr;
    }

    RunnerBase* runner = nullptr;
private:
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue resolved_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

public:
    IOBatchResolverChain(IOBatchResolverId resolver) SKR_NOEXCEPT
    {
        if (resolver)
        {
            chain.emplace_back(resolver);
        }
    }
    SObjectPtr<IIOBatchResolverChain> then(IOBatchResolverId resolver) SKR_NOEXCEPT
    {
        chain.emplace_back(resolver);
        return this;
    }
private:
    skr::vector<IOBatchResolverId> chain;

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

} // namespace io
} // namespace skr