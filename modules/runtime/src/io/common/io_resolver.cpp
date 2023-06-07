#include "io_runnner.hpp"
#include "io_resolver.hpp"
#include "batch_processors.hpp"

namespace skr {
namespace io {

IORequestResolverChain::IORequestResolverChain() SKR_NOEXCEPT 
{
    for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
    {
        skr_atomic64_store_relaxed(&resolving_counts[i], 0);
        skr_atomic64_store_relaxed(&processed_batch_counts[i], 0);
    }
}

void IORequestResolverChain::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId batch;
    while (fetched_batches[priority].try_dequeue(batch))
    {
        for (auto request : batch->get_requests())
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
            if (runner->try_cancel(priority, rq))
            {
                continue;
            }
            else
            {
                rq->setStatus(SKR_IO_STAGE_RESOLVING);
                for (auto resolver : chain)
                {
                    resolver->resolve(request);
                }
            }
        }
        processed_batches[priority].enqueue(batch);
        skr_atomic64_add_relaxed(&resolving_counts[priority], -1);
        skr_atomic64_add_relaxed(&processed_batch_counts[priority], 1);
    }
}

struct OpenFileResolver : public IORequestResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        request->open_file(); 
    }
};

IORequestResolverId IIOService::create_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<OpenFileResolver>::Create();
}

} // namespace io
} // namespace skr