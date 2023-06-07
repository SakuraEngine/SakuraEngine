#include "ram_readers.hpp"
#include "async/thread_job.hpp"

namespace skr {
namespace io {

using VFSReaderFutureLauncher = skr::FutureLauncher<bool>;

bool VFSRAMReader::fetch(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
{
    auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
    fetched_requests[priority].enqueue(rq);
    skr_atomic64_add_relaxed(&pending_counts[priority], 1);
    return true;
}

uint64_t VFSRAMReader::get_prefer_batch_size() const SKR_NOEXCEPT
{
    return 0; // fread is blocking, so we don't need to batch
}

void VFSRAMReader::dispatchFunction(SkrAsyncServicePriority priority, const IORequestId& request) SKR_NOEXCEPT
{
    {
        ZoneScopedN("dispatch_read");
        auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
        auto&& buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
        if (service->runner.try_cancel(priority, rq))
        {
            // cancel...
            if (rq->file) 
            {
                skr_vfs_fclose(rq->file);
                rq->file = nullptr;
            }
            if (buf)
            {
                buf->free_buffer();
            }
        }
        else if (rq->getStatus() == SKR_IO_STAGE_RESOLVING)
        {
            ZoneScopedN("read_request");

            rq->setStatus(SKR_IO_STAGE_LOADING);
            // SKR_LOG_DEBUG("dispatch read request: %s", rq->path.c_str());
            uint64_t dst_offset = 0u;
            for (const auto& block : rq->blocks)
            {
                const auto address = buf->bytes + dst_offset;
                skr_vfs_fread(rq->file, address, block.offset, block.size);
                dst_offset += block.size;
            }
            rq->setStatus(SKR_IO_STAGE_LOADED);
        }
    }
    {
        ZoneScopedN("dispatch_close");
        auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
        if (rq->file)
        {
            // SKR_LOG_DEBUG("dispatch close request: %s", rq->path.c_str());
            skr_vfs_fclose(rq->file);
            rq->file = nullptr;
            loaded_requests[priority].enqueue(rq);
            skr_atomic64_add_relaxed(&processed_counts[priority], 1);
        }
    }
    skr_atomic64_add_relaxed(&pending_counts[priority], -1);

    awakeService();
}

void VFSRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    RQPtr rq;
    if (fetched_requests[priority].try_dequeue(rq))
    {
        auto launcher = VFSReaderFutureLauncher(job_queue);
        loaded_futures[priority].emplace_back(
            launcher.async([this, rq, priority](){
                ZoneScopedN("VFSReadTask");
                dispatchFunction(priority, rq);
                return true;
            })
        );
    }
}

bool VFSRAMReader::poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT
{
    if (loaded_requests[priority].try_dequeue(request))
    {
        skr_atomic64_add_relaxed(&processed_counts[priority], -1);
        return request.get();
    }
    return false;
}

void VFSRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    ZoneScopedN("VFSRAMReader::recycle");

    auto& arr = loaded_futures[priority];
    for (auto& future : arr)
    {
        auto status = future->wait_for(0);
        if (status == skr::FutureStatus::Ready)
        {
            SkrDelete(future);
            future = nullptr;
        }
    }
    auto it = eastl::remove_if(arr.begin(), arr.end(), 
        [](skr::IFuture<bool>* future) {
            return (future == nullptr);
        });
    arr.erase(it, arr.end());
}

} // namespace io
} // namespace skr