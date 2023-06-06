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

    virtual bool poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT
    {
        SKR_ASSERT(false && "Not implemented");
        return false;
    }

    virtual bool poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
    {
        if (resolved_batches[priority].try_dequeue(batch))
        {
            return batch.get();
        }
        return false;
    }

    RunnerBase* runner = nullptr;
private:
    IOBatchQueue fetched_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue resolved_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

public:
    IORequestResolverChain(IORequestResolverId resolver) SKR_NOEXCEPT
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