#pragma once
#include "async/async_service.h"
#include "async/condlock.hpp"
#include "io_request.hpp"
#include "io_batch.hpp"
#include "io_resolver.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {

struct RunnerBase : public AsyncService
{
    RunnerBase(const ServiceThreadDesc& desc, skr::JobQueue* job_queue) SKR_NOEXCEPT
        : AsyncService(desc), job_queue(job_queue)
    {

    }
    virtual ~RunnerBase() SKR_NOEXCEPT = default;

    void poll_finish_callbacks()
    {
        RQPtr rq = nullptr;
        while (finish_queues->try_dequeue(rq))
        {
            rq->tryPollFinish();
        }
    }

    // cancel request marked as request_cancel
    bool try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT;
    virtual bool cancelFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void dispatch_complete(SkrAsyncServicePriority priority, skr::SObjectPtr<IORequestBase> rq) SKR_NOEXCEPT;
    virtual bool completeFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    void process_batches() SKR_NOEXCEPT;
    void recycle() SKR_NOEXCEPT;
    virtual skr::AsyncResult serve() SKR_NOEXCEPT;
    virtual void drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual void drain() SKR_NOEXCEPT;
    virtual void destroy() SKR_NOEXCEPT;

    uint64_t predicate() const
    {
        uint64_t cnt = 0;
        for (auto processor : batch_processors)
        {
            if (!processor->is_async())
                cnt += processor->processing_count();
            cnt += processor->processed_count();
        }
        for (auto processor : request_processors)
        {
            if (!processor->is_async())
                cnt += processor->processing_count();
            cnt += processor->processed_count();
        }
        return cnt;
    }

protected:
    void complete_batches(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void complete_requests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    skr::vector<IOBatchProcessorId> batch_processors; 
    skr::vector<IORequestProcessorId> request_processors; 
private:
    IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    
    skr::vector<skr::IFuture<bool>*> finish_futures;
    skr::JobQueue* job_queue = nullptr;
};

} // namespace io
} // namespace skr