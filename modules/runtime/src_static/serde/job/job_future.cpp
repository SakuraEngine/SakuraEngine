#include "async/thread_job.hpp"
#include "job_thread.hpp"
#include "containers/vector.hpp"
#include "utils/defer.hpp"
#include "utils/log.h"

namespace skr
{

ThreadedJobQueueFutureJob::ThreadedJobQueueFutureJob(JobQueue* Q) SKR_NOEXCEPT 
    : JobItem(u8"TestJob"), Q(Q) 
{

}

bool ThreadedJobQueueFutureJob::valid() const SKR_NOEXCEPT { return true; }

void ThreadedJobQueueFutureJob::wait() SKR_NOEXCEPT
{
    while (skr_atomic32_load_relaxed(&finished) == false)
    {
        skr_thread_sleep(1);
    }
}

skr::FutureStatus ThreadedJobQueueFutureJob::wait_for(uint32_t ms) SKR_NOEXCEPT
{
    skr_thread_sleep(ms);
    const auto f = skr_atomic32_load_relaxed(&finished);
    if (f) return skr::FutureStatus::Ready;
    return skr::FutureStatus::Timeout;
}

void ThreadedJobQueueFutureJob::finish(skr::JobResult result) SKR_NOEXCEPT
{
    if (result == skr::JOB_RESULT_OK)
    {
        skr_atomic32_store_relaxed(&finished, true);
    }
}

} // namespace skr