#include "../../pch.hpp" // IWYU pragma: keep
#include "SkrRT/platform/vfs.h"
#include "io_runnner.hpp"
#include "io_resolver.hpp"
#include "processors.hpp"

namespace skr {
namespace io {

IORequestResolverChain::IORequestResolverChain() SKR_NOEXCEPT 
{
    init_counters();
}

void IORequestResolverChain::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId batch;
    while (fetched_batches[priority].try_dequeue(batch))
    {
        for (auto request : batch->get_requests())
        {
            if (auto rq = skr::static_pointer_cast<IORequestBase>(request))
            {
                rq->setStatus(SKR_IO_STAGE_RESOLVING);
                for (auto resolver : chain)
                {
                    if (!runner->try_cancel(priority, rq))
                        resolver->resolve(priority, request);
                }
            }
        }
        processed_batches[priority].enqueue(batch);
        dec_processing(priority);
        inc_processed(priority);
    }
}

void VFSFileResolver::resolve(SkrAsyncServicePriority priority,IORequestId request) SKR_NOEXCEPT
{
    auto rq = skr::static_pointer_cast<IORequestBase>(request);
    SKR_ASSERT(rq->vfs);
    if (!rq->file)
    {
        rq->file = skr_vfs_fopen(rq->vfs, rq->path.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
    }
}

} // namespace io
} // namespace skr