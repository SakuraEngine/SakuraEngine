#include "../../pch.hpp"
#include "SkrRT/platform/debug.h"
#include "vram_readers.hpp"
#include "SkrRT/async/thread_job.hpp"

// VFS READER IMPLEMENTATION

namespace skr {
namespace io {

/*
CommonVRAMReader::CommonVRAMReader(VRAMService* service, IRAMService* ram_service) SKR_NOEXCEPT 
    : VRAMReaderBase(service), ram_service(ram_service) 
{

}

CommonVRAMReader::~CommonVRAMReader() SKR_NOEXCEPT
{
    
}

bool CommonVRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    auto B = static_cast<IOBatchBase*>(batch.get());
    if (B->can_use_dstorage)
    {
        fetched_batches[priority].enqueue(batch);
        inc_processing(priority);
    }
    else
    {
        processed_batches[priority].enqueue(batch);
        inc_processed(priority);
    }
    return true;
}

void CommonVRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    addRAMRequests(priority);
    ensureRAMRequests(priority);
    addUploadRequests(priority);
    ensureUploadRequests(priority);
}

void CommonVRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{

}

bool CommonVRAMReader::poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
{
    if (processed_batches[priority].try_dequeue(batch))
    {
        dec_processed(priority);
        return batch.get();
    }
    return false;
}

void CommonVRAMReader::addRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId batch;
    while (fetched_batches[priority].try_dequeue(batch))
    {
        for (auto&& request : batch->get_requests())
        {
            if (service->runner.try_cancel(priority, request))
            {
                // ...
            }
            else if (auto pStatus = io_component<IOStatusComponent>(request.get()))
            {
                if (pStatus->getStatus() == SKR_IO_STAGE_RESOLVING)
                {
                    ZoneScopedN("VRAMReader::RAMRequest");

                }
                else
                    SKR_UNREACHABLE_CODE()
            }
        }
    }
}
*/

} // namespace io
} // namespace skr