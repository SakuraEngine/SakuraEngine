#pragma once
#include "async/service_thread.hpp"
#include "async/condlock.hpp"
#include "io_request.hpp"
#include "io_batch.hpp"

namespace skr {
namespace io {

struct RunnerBase : public skr::ServiceThread
{
    RunnerBase(const ServiceThreadDesc& desc) SKR_NOEXCEPT
        : skr::ServiceThread(desc)
    {
        condlock.initialize(skr::format(u8"{}-CondLock", desc.name).u8_str());
    }
    virtual ~RunnerBase() SKR_NOEXCEPT = default;

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

    const bool condsleep = false;
    void sleep() SKR_NOEXCEPT
    {
        const auto ms = skr_atomicu64_load_relaxed(&sleep_time);
        if (!condsleep)
        {
            ZoneScopedNC("ioServiceSleep(Sleep)", tracy::Color::Gray55);
            skr_thread_sleep(ms);
        }
        else
        {
            ZoneScopedNC("ioServiceSleep(Cond)", tracy::Color::Gray55);
            condlock.lock();
            condlock.wait(ms);
            condlock.unlock();
        }
    }

    void tryAwake()
    {
        if (condsleep)
        {
            condlock.signal();
        } 
    }

    void enqueueBatch(const IOBatchId& batch)
    {
        const auto pri = batch->get_priority();
        batch_queues[pri].enqueue(batch);
        for (auto&& request : batch->get_requests())
        {
            auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
            rq->setStatus(SKR_IO_STAGE_ENQUEUED);
            skr_atomicu32_add_relaxed(&queued_batch_counts[pri], 1);
        }
    }

    uint64_t getQueuedBatchCount(SkrAsyncServicePriority priority) const SKR_NOEXCEPT
    {
        return skr_atomicu64_load_relaxed(&queued_batch_counts[priority]);
    }

protected:
    IOBatchQueue batch_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IOBatchQueue resolved_batch_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomicU64 queued_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

private:
    SAtomicU32 sleep_time = 16u;
    CondLock condlock;
    SAtomicU32 service_status = SKR_ASYNC_SERVICE_STATUS_SLEEPING;
};

} // namespace io
} // namespace skr