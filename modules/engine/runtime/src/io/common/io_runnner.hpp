#pragma once
#include "SkrContainers/stl_vector.hpp"
#include "SkrCore/async/async_service.h"
#include "io_request.hpp"
#include <utility>

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {

struct RunnerBase : public AsyncService
{
    RunnerBase(const ServiceThreadDesc& desc, skr::JobQueue* job_queue) SKR_NOEXCEPT;
    virtual ~RunnerBase() SKR_NOEXCEPT;

    bool try_cancel(SkrAsyncServicePriority priority, IORequestId rq) SKR_NOEXCEPT;
    uint64_t predicate() const SKR_NOEXCEPT;
    uint64_t processing_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT;
    void poll_finish_callbacks() SKR_NOEXCEPT;

    virtual void drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual void drain() SKR_NOEXCEPT;
    virtual void destroy() SKR_NOEXCEPT;
    virtual skr::AsyncResult serve() SKR_NOEXCEPT;

protected:
    void dispatch_complete_(SkrAsyncServicePriority priority, IORequestId rq) SKR_NOEXCEPT;
    virtual bool complete_(IIORequest* rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    virtual bool cancel_(IIORequest* rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    skr::stl_vector<IOBatchProcessorId> batch_processors; 
    skr::stl_vector<IORequestProcessorId> request_processors; 
    SAtomic64 processing_request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

private:
    void phaseRecycle() SKR_NOEXCEPT;
    void phaseProcessBatches() SKR_NOEXCEPT;
    void phaseCompleteBatches(SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void phaseCompleteRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::stl_vector<std::pair<skr::IFuture<bool>*, IORequestId>> finish_futures;
    skr::JobQueue* job_queue = nullptr;
};

} // namespace io
} // namespace skr