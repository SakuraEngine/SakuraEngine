#include "ram_readers.hpp"

namespace skr {
namespace io {

void VFSRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    auto& arr = dispatching_requests[priority];
    for (auto& request : batch->get_requests())
    {
        auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
        arr.emplace_back(rq);
        skr_atomicu64_add_relaxed(&dispatching_requests_counts[priority], 1);
    }
}

uint64_t VFSRAMReader::get_prefer_batch_size() const SKR_NOEXCEPT
{
    return 1024 * 1024 * 4;
}

void VFSRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto& arr = dispatching_requests[priority];
    {
        ZoneScopedN("sort");
        std::sort(arr.begin(), arr.end(), 
        [](const RQPtr& a, const RQPtr& b) {
            return a->sub_priority > b->sub_priority;
        });
    }
    {
        ZoneScopedN("dispatch_read");
        for (auto&& request : arr)
        {
            auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
            auto&& buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
            if (service->runner.try_cancel(priority, rq))
            {
                // cancel...
                if (rq->file) 
                    skr_vfs_fclose(rq->file);
                if (buf)
                    buf->free_buffer();
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
    }
    {
        ZoneScopedN("dispatch_close");

        for (auto&& request : arr)
        {
            auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
            if (rq->file)
            {
                // SKR_LOG_DEBUG("dispatch close request: %s", rq->path.c_str());
                skr_vfs_fclose(rq->file);
                rq->file = nullptr;
                finish_requests[priority].enqueue(rq);
            }
        }
    }
}

IORequestId VFSRAMReader::poll_finish(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IORequestId request;
    if (finish_requests[priority].try_dequeue(request))
    {
        auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_PENDING);
        return rq;
    }
    return nullptr;
}

void VFSRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto& arr = dispatching_requests[priority];
    auto it = eastl::remove_if(arr.begin(), arr.end(), 
        [](const IORequestId& request) {
            auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
            return (rq->getFinishStep() != SKR_ASYNC_IO_FINISH_STEP_NONE);
        });
    arr.erase(it, arr.end());

    const int64_t X = (int64_t)arr.size();
    arr.erase(it, arr.end());
    const int64_t Y = (int64_t)arr.size();
    skr_atomicu64_add_relaxed(&dispatching_requests_counts[priority], Y - X);
}

} // namespace io
} // namespace skr