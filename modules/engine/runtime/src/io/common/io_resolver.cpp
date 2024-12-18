#include "SkrCore/platform/vfs.h"
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
            if (auto pComp = io_component<IOStatusComponent>(request.get()))
            {
                pComp->setStatus(SKR_IO_STAGE_RESOLVING);
                for (auto resolver : chain)
                {
                    if (!runner->try_cancel(priority, request))
                        resolver->resolve(priority, batch, request);
                }
            }
        }
        processed_batches[priority].enqueue(batch);
        dec_processing(priority);
        inc_processed(priority);
    }
}

void VFSFileResolver::resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT
{
    auto pPath = io_component<PathSrcComponent>(request.get());
    auto pFile = io_component<FileComponent>(request.get());
    if (pPath && pFile)
    {
        if (!pFile->dfile && !pFile->file)
        {
            SKR_ASSERT(pPath->get_vfs());
            pFile->file = skr_vfs_fopen(pPath->get_vfs(), pPath->get_path(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
        }
    }
}

} // namespace io
} // namespace skr