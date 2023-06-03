#include "../common/io_runnner.hpp"

namespace skr {
namespace io {

bool RunnerBase::try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    const auto status = rq->getStatus();
    if (status >= SKR_IO_STAGE_LOADING) return false;

    if (bool cancel_requested = skr_atomicu32_load_acquire(&rq->future->request_cancel))
    {
        if (rq->getFinishStep() == SKR_ASYNC_IO_FINISH_STEP_NONE)
        {
            rq->setStatus(SKR_IO_STAGE_CANCELLED);
            if (rq->needPollFinish())
            {
                finish_queues[priority].enqueue(rq);
                rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_PENDING);
            }
            else
            {
                rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
            }
        }
        return true;
    }
    return false;
}

void RunnerBase::recycle() SKR_NOEXCEPT
{
    ZoneScopedN("recycle");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->recycle((SkrAsyncServicePriority)i);
        auto& arr = dispatching_batches[i];
        auto it = eastl::remove_if(arr.begin(), arr.end(), 
            [](const IOBatchId& batch) {
                for (auto request : batch->get_requests()) 
                {
                    auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
                    if (rq->getFinishStep() != SKR_ASYNC_IO_FINISH_STEP_DONE)
                    {
                        return false;
                    }
                }
                return true;
            });
        const int64_t X = (int64_t)arr.size();
        arr.erase(it, arr.end());
        const int64_t Y = (int64_t)arr.size();
        skr_atomicu64_add_relaxed(&dispatching_batch_counts[i], Y - X);
    }
}

uint64_t RunnerBase::fetch() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
    ZoneScopedN("fetch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        BatchPtr batch = nullptr;
        while (resolved_batch_queues[i].try_dequeue(batch))
        {
            reader->fetch((SkrAsyncServicePriority)i, batch);

            dispatching_batches[i].emplace_back(batch);
            skr_atomicu64_add_relaxed(&dispatching_batch_counts[i], 1);
            skr_atomicu64_add_relaxed(&queued_batch_counts[i], -1);
        }
    }
    return 0;
}

void RunnerBase::sort() SKR_NOEXCEPT
{
    ZoneScopedN("sort");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->sort((SkrAsyncServicePriority)i);
    }
}

void RunnerBase::dispatch() SKR_NOEXCEPT
{
    ZoneScopedN("dispatch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->dispatch((SkrAsyncServicePriority)i);
    }
}

void RunnerBase::resolve() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
    ZoneScopedN("resolve");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        BatchPtr batch = nullptr;
        while (batch_queues[i].try_dequeue(batch))
        {
            for (auto&& request : batch->get_requests())
            {
                auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
                if (try_cancel((SkrAsyncServicePriority)i, rq))
                {
                    // ...
                }
                else
                {
                    rq->setStatus(SKR_IO_STAGE_RESOLVING);
                    for (auto&& resolver : resolver_chain->chain)
                        resolver->resolve(request);
                }
            }
            resolved_batch_queues[i].enqueue(batch);
        }
    }
}

void RunnerBase::uncompress() SKR_NOEXCEPT
{
    // do nothing now
}

void RunnerBase::finish() SKR_NOEXCEPT
{
    ZoneScopedN("finish");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        while (auto request = reader->poll_finish((SkrAsyncServicePriority)i))
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
            rq->setStatus(SKR_IO_STAGE_COMPLETED);
            if (rq->needPollFinish())
            {
                finish_queues[i].enqueue(rq);
            }
            else
            {
                rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
            }
        }
    }
}

} // namespace io
} // namespace skr