#pragma once
#include "async/service_thread.hpp"
#include "async/condlock.hpp"
#include "io_request.hpp"
#include "io_batch.hpp"
#include "io_resolver.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {

struct SleepyService : public skr::ServiceThread
{
    SleepyService(const ServiceThreadDesc& desc) SKR_NOEXCEPT
        : skr::ServiceThread(desc)
    {
        condlock.initialize(skr::format(u8"{}-CondLock", desc.name).u8_str());
    }
    virtual ~SleepyService() SKR_NOEXCEPT = default;

    void setServiceStatus(SkrAsyncServiceStatus status) SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&service_status, status);
    }

    SkrAsyncServiceStatus getServiceStatus() const SKR_NOEXCEPT
    {
        return (SkrAsyncServiceStatus)skr_atomicu32_load_acquire(&service_status);
    }

    void setSleepTime(uint32_t time) SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&sleep_time, time);
    }

    virtual bool predicate() const SKR_NOEXCEPT = 0;

    void sleep() SKR_NOEXCEPT
    {
        const auto ms = skr_atomicu64_load_relaxed(&sleep_time);

        ZoneScopedNC("ioServiceSleep(Cond)", tracy::Color::Gray55);
        condlock.lock();
        while (!predicate())
            condlock.wait(ms);
        condlock.unlock();
    }

    void request_stop() SKR_NOEXCEPT override
    {
        skr::ServiceThread::request_stop();
        tryAwake();
    }

    void wait_stop() SKR_NOEXCEPT override
    {
        tryAwake();
        skr::ServiceThread::wait_stop();
    }

    void tryAwake()
    {
        condlock.signal();
    }

private:
    SAtomicU32 sleep_time = 16u;
    CondLock condlock;
    SAtomicU32 service_status = SKR_ASYNC_SERVICE_STATUS_SLEEPING;
};

struct RunnerBase : public SleepyService
{
    RunnerBase(const ServiceThreadDesc& desc, SObjectPtr<IIOReader> reader, skr::JobQueue* job_queue) SKR_NOEXCEPT
        : SleepyService(desc), reader(reader), job_queue(job_queue)
    {
        for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
        {
            skr_atomicu64_store_relaxed(&processing_request_counts[i], 0);
            skr_atomicu64_store_relaxed(&queued_batch_counts[i], 0);
            skr_atomicu64_store_relaxed(&dispatching_batch_counts[i], 0);
        }
    }
    virtual ~RunnerBase() SKR_NOEXCEPT = default;

    void enqueueBatch(const IOBatchId& batch)
    {
        const auto pri = batch->get_priority();
        batch_queues[pri].enqueue(batch);
        for (auto&& request : batch->get_requests())
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
            rq->setStatus(SKR_IO_STAGE_ENQUEUED);
            skr_atomicu32_add_relaxed(&processing_request_counts[pri], 1);
        }
        skr_atomicu32_add_relaxed(&queued_batch_counts[pri], 1);
    }

    uint64_t getQueuedBatchCount(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomicu64_load_relaxed(&queued_batch_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                count += skr_atomicu64_load_relaxed(&queued_batch_counts[i]);
            }
            return count;
        }
    }

    uint64_t getExecutingBatchCount(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomicu64_load_relaxed(&dispatching_batch_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                count += skr_atomicu64_load_relaxed(&dispatching_batch_counts[i]);
            }
            return count;
        }
    }

    uint64_t getProcessingRequestCount(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomicu64_load_relaxed(&processing_request_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                count += skr_atomicu64_load_relaxed(&processing_request_counts[i]);
            }
            return count;
        }
    }

    void poll_finish_callbacks()
    {
        RQPtr rq = nullptr;
        while (finish_queues->try_dequeue(rq))
        {
            rq->tryPollFinish();
        }
    }

    bool predicate() const SKR_NOEXCEPT
    {
        return getQueuedBatchCount() > 0;
    }

    // cancel request marked as request_cancel
    bool try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT;
    bool cancelFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    // 0. recycletry_cancel
    void recycle() SKR_NOEXCEPT;
    // 1. fetch requests from queue
    uint64_t fetch() SKR_NOEXCEPT;
    // 3. resolve requests to pending raw request array
    void resolve() SKR_NOEXCEPT;
    // 4. dispatch I/O blocks to drives (+allocate & cpy to raw)
    void dispatch() SKR_NOEXCEPT;
    // 5. do uncompress works (+allocate & cpy to uncompressed)
    void uncompress() SKR_NOEXCEPT;
    // 6. finish
    void finish() SKR_NOEXCEPT;
    bool finishFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT;

    SObjectPtr<IIOReader> reader = nullptr;
    SObjectPtr<IOBatchResolverChain> resolver_chain = nullptr;

private:
    IOBatchQueue batch_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue resolved_batch_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

    SAtomicU64 processing_request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomicU64 queued_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomicU64 dispatching_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

    IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    skr::vector<skr::IFuture<bool>*> finish_futures;
    skr::JobQueue* job_queue = nullptr;
};

} // namespace io
} // namespace skr