#include "ram_readers.hpp"
#include "async/thread_job.hpp"

namespace skr {
namespace io {

namespace VFSReader
{
using Future = skr::IFuture<bool>;
using JobQueueFuture = skr::ThreadedJobQueueFuture<bool>;
using SerialFuture = skr::SerialFuture<bool>;
struct FutureLauncher
{
    FutureLauncher(skr::JobQueue* q) : job_queue(q) {}
    template<typename F, typename... Args>
    Future* async(F&& f, Args&&... args)
    {
        if (job_queue)
            return SkrNew<JobQueueFuture>(job_queue, std::forward<F>(f), std::forward<Args>(args)...);
        else
            return SkrNew<SerialFuture>(std::forward<F>(f), std::forward<Args>(args)...);
    }
    skr::JobQueue* job_queue = nullptr;
};
}

bool VFSRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    fetched_batches[priority].enqueue(batch);
    return true;
}

uint64_t VFSRAMReader::get_prefer_batch_size() const SKR_NOEXCEPT
{
    return 0; // fread is blocking, so we don't need to batch
}

void VFSRAMReader::dispatchFunction(IOBatchId batch) SKR_NOEXCEPT
{
    const auto priority = batch->get_priority();
    auto&& arr = batch->get_requests();
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
                loaded_requests[priority].enqueue(rq);
            }
        }
    }
    loaded_batches[priority].enqueue(batch);
    tryAwakeService();
}

void VFSRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        IOBatchId batch;
        if (fetched_batches[i].try_dequeue(batch))
        {
            auto launcher = VFSReader::FutureLauncher(job_queue);
            loaded_futures[i].emplace_back(
                launcher.async([this, batch](){
                    ZoneScopedN("VFSReadTask");
                    dispatchFunction(batch);
                    return true;
                })
            );
        }
    }
}

IORequestId VFSRAMReader::poll_processed_request(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IORequestId request;
    if (loaded_requests[priority].try_dequeue(request))
    {
        return request;
    }
    return nullptr;
}

IOBatchId VFSRAMReader::poll_processed_batch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId batch;
    if (loaded_batches[priority].try_dequeue(batch))
    {
        return batch;
    }
    return nullptr;
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