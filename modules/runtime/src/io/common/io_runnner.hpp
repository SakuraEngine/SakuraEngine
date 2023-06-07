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

    SkrAsyncServiceStatus getServiceStatus() const SKR_NOEXCEPT
    {
        return (SkrAsyncServiceStatus)skr_atomicu32_load_acquire(&service_status);
    }

    void set_sleep_time(uint32_t time) SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&sleep_time, time);
    }

    void sleep() SKR_NOEXCEPT
    {
        const auto ms = skr_atomicu64_load_relaxed(&sleep_time);
        ZoneScopedNC("ioServiceSleep(Cond)", tracy::Color::Gray55);

        condlock.lock();
        if (!event)
        {
            condlock.wait(ms);
        }
        event = false;
        condlock.unlock();
    }

    void request_stop() SKR_NOEXCEPT override
    {
        skr::ServiceThread::request_stop();
        awake();
    }

    void wait_stop(uint32_t fatal_timeout = 8) SKR_NOEXCEPT override
    {
        awake();
        skr::ServiceThread::wait_stop(fatal_timeout);
    }

    void awake()
    {
        condlock.lock();
        event = true;
        condlock.signal();
        condlock.unlock();
    }

protected:
    void setServiceStatus(SkrAsyncServiceStatus status) SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&service_status, status);
    }

private:
    SAtomicU32 sleep_time = 16u;
    bool event = false;
    CondLock condlock;
    SAtomicU32 service_status = SKR_ASYNC_SERVICE_STATUS_SLEEPING;
};

struct RunnerBase : public SleepyService
{
    RunnerBase(const ServiceThreadDesc& desc, skr::JobQueue* job_queue) SKR_NOEXCEPT
        : SleepyService(desc), job_queue(job_queue)
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