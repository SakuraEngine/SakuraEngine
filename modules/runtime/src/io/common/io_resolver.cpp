#include "io_runnner.hpp"
#include "io_resolver.hpp"

namespace skr {
namespace io {

void IOBatchResolverChain::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        IOBatchId batch;
        if (fetched_batches[i].try_dequeue(batch))
        {
            for (auto request : batch->get_requests())
            {
                auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
                if (runner->try_cancel((SkrAsyncServicePriority)i, rq))
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
            resolved_batches[priority].enqueue(batch);
        }
    }
}

struct OpenFileResolver : public IOBatchResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        request->open_file(); 
    }
};

IOBatchResolverId IIOService::create_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<OpenFileResolver>::Create();
}

} // namespace io
} // namespace skr