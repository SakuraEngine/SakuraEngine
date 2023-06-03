#include "ram/ram_readers.hpp"

namespace skr {
namespace io {

void VFSRAMReader::fetch(SkrAsyncServicePriority priority, IOBatch batch) SKR_NOEXCEPT
{
    auto& arr = ongoing_requests[priority];
    for (auto& request : batch->get_requests())
    {
        auto rq = skr::static_pointer_cast<RAMIORequest>(request);
        arr.emplace_back(rq);
    }
}

void VFSRAMReader::sort(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto& arr = ongoing_requests[priority];

    auto it = eastl::remove_if(arr.begin(), arr.end(), 
        [](const IORequest& request) {
            if (!request) 
                return true;
            return false;
        });
    arr.erase(it, arr.end());

    std::sort(arr.begin(), arr.end(), 
    [](const RQPtr& a, const RQPtr& b) {
        return a->sub_priority > b->sub_priority;
    });
}

void VFSRAMReader::resolve(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SKR_ASSERT(service);
    auto& arr = ongoing_requests[priority];
    for (auto& rq : arr)
    {
        if (service->runner.try_cancel(priority, rq))
        {
            // cancel...
        }
        else if (rq->getStatus() == SKR_IO_STAGE_ENQUEUED)
        {
            // SKR_LOG_DEBUG("dispatch open request: %s", rq->path.c_str());
            rq->setStatus(SKR_IO_STAGE_RESOLVING);
            
            // TODO: resolver ordering

            // TODO: resolver jobs
            skr_rw_mutex_acuire_r(&service->runner.resolvers_mutex);
            for (auto&& [id, resolver] : service->runner.resolvers)
            {
                resolver(rq);
            }
            skr_rw_mutex_release(&service->runner.resolvers_mutex);
        }
        else
        {
            // SKR_LOG_DEBUG("dispatch open request: %s, skip", rq->path.c_str());
        }
    }
}

void VFSRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    {
        ZoneScopedN("dispatch_read");
        auto& arr = ongoing_requests[priority];
        for (auto& rq : arr)
        {
            auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
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
            else
            {
                // SKR_LOG_DEBUG("dispatch read request: %s, skip", rq->path.c_str());
            }
        }
    }
    {
        ZoneScopedN("dispatch_close");

        auto& arr = ongoing_requests[priority];
        for (auto& rq : arr)
        {
            if (rq->file)
            {
                // SKR_LOG_DEBUG("dispatch close request: %s", rq->path.c_str());
                skr_vfs_fclose(rq->file);
                rq->file = nullptr;
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch close request: %s, skip", rq->path.c_str());
            }
        }
    }
}

IORequest VFSRAMReader::poll_finish(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto& arr = ongoing_requests[priority];
    for (auto& rq : arr)
    {
        if (!rq) 
            continue;

        const auto d = skr_atomicu32_load_relaxed(&rq->done);
        if (d == 0)
        {
            auto polled = rq;
            rq.reset();
            return polled;
        }
    }
    return nullptr;
}

} // namespace io
} // namespace skr