#include "../common/io_runnner.hpp"
#include "async/thread_job.hpp"

namespace skr {
namespace io {

namespace IORunner
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

void RunnerBase::recycle() SKR_NOEXCEPT
{
    ZoneScopedN("recycle");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        for (auto& future : finish_futures)
        {
            auto status = future->wait_for(0);
            if (status == skr::FutureStatus::Ready)
            {
                SkrDelete(future);
                future = nullptr;
            }
        }
        auto it = eastl::remove_if(finish_futures.begin(), finish_futures.end(), 
            [](skr::IFuture<bool>* future) {
                return (future == nullptr);
            });
        finish_futures.erase(it, finish_futures.end());
    }

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->recycle((SkrAsyncServicePriority)i);
    }
}

void RunnerBase::dispatch_resolve() SKR_NOEXCEPT
{
    SKR_ASSERT(resolver_chain);
    ZoneScopedN("dispatch_resolve");

    const uint64_t NBytes = resolver_chain->get_prefer_batch_size();
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        uint64_t bytes = 0;
        BatchPtr batch = nullptr;
        const auto priority = (SkrAsyncServicePriority)i;
        while ((bytes <= NBytes) && batch_buffer->poll_processed_batch(priority, batch))
        {
            uint64_t batch_size = 0;
            if (bool sucess = resolver_chain->fetch(priority, batch))
            {
                SKR_ASSERT(sucess);
                for (auto&& request : batch->get_requests())
                {
                    for (auto block : request->get_blocks())
                        batch_size += block.size;
                }
                bytes += batch_size;
            }
            resolver_chain->dispatch(priority);
        }
    }
}

void RunnerBase::dispatch_read() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
    ZoneScopedN("dispatch_read");

    const uint64_t NBytes = reader->get_prefer_batch_size();
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        uint64_t bytes = 0;
        BatchPtr batch = nullptr;
        const auto priority = (SkrAsyncServicePriority)i;
        while ((bytes <= NBytes) && resolver_chain->poll_processed_batch(priority, batch))
        {
            uint64_t batch_size = 0;
            if (bool sucess = reader->fetch(priority, batch))
            {
                SKR_ASSERT(sucess);
                for (auto&& request : batch->get_requests())
                {
                    for (auto block : request->get_blocks())
                        batch_size += block.size;
                }
                bytes += batch_size;
            }
        }
        reader->dispatch(priority);
    }
}

bool RunnerBase::cancelFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    rq->setStatus(SKR_IO_STAGE_CANCELLED);
    if (rq->needPollFinish())
    {
        finish_queues[priority].enqueue(rq);
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_WAIT_CALLBACK_POLLING);
    }
    else
    {
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
    }
    skr_atomic64_add_relaxed(&processing_request_counts[priority], -1);
    return true;
}

bool RunnerBase::try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    const auto status = rq->getStatus();
    if (status >= SKR_IO_STAGE_LOADING) return false;

    if (bool cancel_requested = skr_atomicu32_load_acquire(&rq->future->request_cancel))
    {
        if (rq->getFinishStep() == SKR_ASYNC_IO_FINISH_STEP_NONE)
        {
            if (rq->async_cancel)
            {
                auto& future = finish_futures.emplace_back();
                future = IORunner::FutureLauncher(job_queue).async(
                [this, priority, rq = rq](){
                    return cancelFunction(rq, priority);
                });
            }
            else
            {
                cancelFunction(rq, priority);
            }
        }
        return true;
    }
    return false;
}

bool RunnerBase::completeFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    rq->setStatus(SKR_IO_STAGE_COMPLETED);
    if (rq->needPollFinish())
    {
        finish_queues[priority].enqueue(rq);
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_WAIT_CALLBACK_POLLING);
    }
    else
    {
        rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
    }
    skr_atomic64_add_relaxed(&processing_request_counts[priority], -1);
    return true;
}

void RunnerBase::dispatch_complete(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT
{
    if (rq->async_complete)
    {
        auto& future = finish_futures.emplace_back();
        future = IORunner::FutureLauncher(job_queue).async(
        [this, priority, rq = rq](){
            return completeFunction(rq, priority);
        });
    }
    else
    {
        completeFunction(rq, priority);
    }
}

bool RunnerBase::dispatch_decompress(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT
{
    const bool need_decompress = false;
    if (!need_decompress)
        return false;
    // decompressor->decompress();
    return true;
}

void RunnerBase::route_loaded() SKR_NOEXCEPT
{
    ZoneScopedN("route_loaded");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const SkrAsyncServicePriority priority = (SkrAsyncServicePriority)i;
        IORequestId request;
        while (reader->poll_processed_request(priority, request))
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
            // auto need_decompress = dispatch_decompress(priority, rq);
            dispatch_complete(priority, rq);
        }
        IOBatchId batch;
        while (reader->poll_processed_batch(priority, batch))
        {
            
        }
    }
}

} // namespace io
} // namespace skr