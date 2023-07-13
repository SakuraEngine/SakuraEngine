#include "../common/io_runnner.hpp"
#include "SkrRT/async/thread_job.hpp"
#include "SkrRT/async/wait_timeout.hpp"

namespace skr {
namespace io {

void RunnerBase::recycle() SKR_NOEXCEPT
{
    ZoneScopedN("recycle");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const auto priority = (SkrAsyncServicePriority)i;

        auto& futures = finish_futures;
        for (auto& future : futures)
        {
            auto status = future->wait_for(0);
            if (status == skr::FutureStatus::Ready)
            {
                SkrDelete(future);
                future = nullptr;
            }
        }
        auto cleaner = [](skr::IFuture<bool>* future) { return (future == nullptr); };
        auto it = eastl::remove_if(futures.begin(), futures.end(), cleaner);
        futures.erase(it, futures.end());

        for (auto processor : batch_processors)
            processor->recycle(priority);
    }
}

void RunnerBase::process_batches() SKR_NOEXCEPT
{
    ZoneScopedN("process_batches");

    for (uint32_t k = 0; k < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++k)
    {
        const auto priority = (SkrAsyncServicePriority)k;
        // poll batches across batches processor
        for (size_t j = 1; j < batch_processors.size(); j++)
        {
            auto&& processor = batch_processors[j];
            auto&& prev_processor = batch_processors[j - 1];
            
            const uint64_t NBytes = processor->get_prefer_batch_size();
            uint64_t bytes = 0;
            BatchPtr batch = nullptr;
            while ((bytes <= NBytes) && prev_processor->poll_processed_batch(priority, batch))
            {
                uint64_t batch_size = 0;
                if (bool sucess = processor->fetch(priority, batch))
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
            processor->dispatch(priority);
        }

        if (!request_processors.size())
        {
            complete_batches(priority);
            continue;
        }

        // poll requests from last batches processor
        {
            BatchPtr batch = nullptr;
            auto& back_processor = batch_processors.back();
            while (back_processor->poll_processed_batch(priority, batch))
            {
                auto bq = skr::static_pointer_cast<IOBatchBase>(batch);
                for (auto&& request : bq->get_requests())
                {
                    request_processors.front()->fetch(priority, request);
                }
            }
            request_processors.front()->dispatch(priority);
        }

        // poll requests across requests processor
        for (size_t j = 1; j < request_processors.size(); j++)
        {
            auto&& processor = request_processors[j];
            auto&& prev_processor = request_processors[j - 1];
            
            IORequestId request = nullptr;
            while (prev_processor->poll_processed_request(priority, request))
            {
                processor->fetch(priority, request);
            }
            processor->dispatch(priority);
        }

        complete_requests(priority);
    }
}

void RunnerBase::complete_batches(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    BatchPtr batch;
    auto& back_processor = batch_processors.back();
    while (back_processor->poll_processed_batch(priority, batch))
    {
        for (auto&& request : batch->get_requests())
        {
            auto rq = skr::static_pointer_cast<IORequestBase>(request);
            dispatch_complete(priority, rq);
        }
    }
};

void RunnerBase::complete_requests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IORequestId request;
    auto& back_processor = request_processors.back();
    while (back_processor->poll_processed_request(priority, request))
    {
        auto rq = skr::static_pointer_cast<IORequestBase>(request);
        dispatch_complete(priority, rq);
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
    return true;
}

bool RunnerBase::try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    const auto status = rq->getStatus();
    if (status == SKR_IO_STAGE_CANCELLED) return true;
    if (status == SKR_IO_STAGE_LOADING) return false;

    if (bool cancel_requested = rq->getCancelRequested())
    {
        if (rq->getFinishStep() == SKR_ASYNC_IO_FINISH_STEP_NONE)
        {
            if (rq->async_cancel)
            {
                auto cancel = [this, priority, rq] { return cancelFunction(rq, priority); };
                finish_futures.emplace_back(skr::FutureLauncher<bool>(job_queue).async(cancel));
            }
            else
            {
                cancelFunction(rq, priority);
            }
        }
        // remove from batch
        if (auto batch = static_cast<IOBatchBase*>(rq->getOwnerBatch()))
            batch->removeCancelledRequest(rq);
        else
            SKR_UNREACHABLE_CODE();
        return true;
    }
    return false;
}

bool RunnerBase::completeFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SKR_ASSERT(rq->getStatus() == SKR_IO_STAGE_LOADED);
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
    return true;
}

void RunnerBase::dispatch_complete(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT
{
    if (rq->async_complete)
    {
        auto complete = [this, priority, rq] { return completeFunction(rq, priority); };
        finish_futures.emplace_back(skr::FutureLauncher<bool>(job_queue).async(complete));
    }
    else
    {
        completeFunction(rq, priority);
    }
}

skr::AsyncResult RunnerBase::serve() SKR_NOEXCEPT
{
    if (!predicate())
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_SLEEPING);
        sleep();
        return ASYNC_RESULT_OK;
    }
    
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
        ZoneScopedNC("Dispatch", tracy::Color::Orchid1);
        process_batches();
    }
    {
        ZoneScopedNC("Recycle", tracy::Color::Tan1);
        recycle();
    }
    return ASYNC_RESULT_OK;
}

void RunnerBase::drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    if (priority == SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        return drain();

    ZoneScopedN("runner_drain");
    auto predicate = [this, priority]() {
        uint64_t cnt = 0;
        for (auto processor : batch_processors)
            cnt += (processor->processing_count(priority) + processor->processed_count(priority));
        for (auto processor : request_processors)
            cnt += (processor->processing_count(priority) + processor->processed_count(priority));
        return !cnt;
    };
    bool fatal = !wait_timeout(predicate, 5);
    if (fatal)
    {
        {
            skr::string processing_message = u8"batch processing: ";
            skr::string processed_message = u8"batch processed: ";
            for (auto processor : batch_processors)
            {
                processing_message += skr::format(u8"{} ", processor->processing_count(priority));
                processed_message += skr::format(u8"{} ", processor->processed_count(priority));
            }
            SKR_LOG_FATAL(processing_message.c_str());
            SKR_LOG_FATAL(processed_message.c_str());
        }
        {
            skr::string processing_message = u8"request processing: ";
            skr::string processed_message = u8"request processed: ";
            for (auto processor : request_processors)
            {
                processing_message += skr::format(u8"{} ", processor->processing_count(priority));
                processed_message += skr::format(u8"{} ", processor->processed_count(priority));
            }
            SKR_LOG_FATAL(processing_message.c_str());
            SKR_LOG_FATAL(processed_message.c_str());
        }
    }
}

void RunnerBase::drain() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        drain((SkrAsyncServicePriority)i);
    }
}

void RunnerBase::destroy() SKR_NOEXCEPT
{
    drain();
    if (get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
        stop();
    }
    wait_stop();
    exit();
}

} // namespace io
} // namespace skr