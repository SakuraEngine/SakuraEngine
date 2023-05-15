#include "job/thread_job.hpp"
#include "job_thread.hpp"

namespace skr
{
JobThreadFunction::~JobThreadFunction() SKR_NOEXCEPT
{

}

JobQueueThread::JobQueueThread() SKR_NOEXCEPT
{

}

JobQueueThread::~JobQueueThread() SKR_NOEXCEPT
{
    if (skr_atomic32_load_acquire(&started)) 
    {
        skr_destroy_thread(tHandle);
        skr_atomic32_store_release(&started, false);
    }
}

JobQueueThread::JobQueueThread(const char8_t *name, JobQueuePriority priority, uint32_t stack_size, const JobQueueThreadDesc *desc) SKR_NOEXCEPT
    : started(false), alive(false), func(nullptr)
{
    initialize(name, priority, stack_size, desc);
}

void JobQueueThread::jobThreadFunc(void* args)
{
    JobQueueThread* pSelf = (JobQueueThread*)args;
    pSelf->tID = skr_current_thread_id();
    
    skr_atomic32_store_release(&pSelf->started, true);
    skr_atomic32_store_release(&pSelf->alive, true);

    pSelf->func->run();
    
    skr_atomic32_store_release(&pSelf->alive, false);
}


JobResult JobQueueThread::start(JobThreadFunction* pFunc) SKR_NOEXCEPT
{
    if (skr_atomic32_load_acquire(&started)) 
        return JOB_RESULT_ERROR_THREAD_ALREADY_STARTES;
    
    this->func = pFunc;

    tDesc.pFunc = &jobThreadFunc;
    tDesc.pData = this;
    skr_init_thread(&tDesc, &tHandle);
    
    // wait started
    while (!skr_atomic32_load_acquire(&started)) {}

    return JOB_RESULT_OK;
}

JobResult JobQueueThread::join() SKR_NOEXCEPT
{
    skr_join_thread(tHandle);
    return JOB_RESULT_OK;
}

bool JobQueueThread::is_alive() const SKR_NOEXCEPT
{
    return skr_atomic32_load_acquire(&alive);
}

SThreadID JobQueueThread::get_id() const SKR_NOEXCEPT
{
    return tID;
}

SThreadPriority JobQueueThread::change_priority(SThreadPriority priority) SKR_NOEXCEPT
{
    return skr_set_thread_priority(tHandle, priority);
}

JobThreadFunction* JobQueueThread::get_function() const SKR_NOEXCEPT
{
    return func;
}

JobResult JobQueueThread::initialize(const char8_t *n, int32_t p, uint32_t stackSize, const JobQueueThreadDesc *pdesc) SKR_NOEXCEPT
{
    name = skr::text::text::from_utf8(n);
    if (pdesc)
    {
        desc = *pdesc;
    }
    return JOB_RESULT_OK;
}

JobResult JobQueueThread::finalize() SKR_NOEXCEPT
{
    return JOB_RESULT_OK;
}

}

namespace skr
{
JobQueueCond::JobQueueCond(const char8_t* name, const JobQueueCondDesc* pDesc) SKR_NOEXCEPT
{
    initialize(name, pDesc);
}

JobQueueCond::~JobQueueCond() SKR_NOEXCEPT
{
    finalize();
}

JobResult JobQueueCond::initialize(const char8_t* name, const JobQueueCondDesc* pOption) SKR_NOEXCEPT
{
    bool ok = skr_init_condition_var(&cond);
    if (ok)
    {
        ok = skr_init_mutex(&mutex);
        if (ok)
            return JOB_RESULT_OK;
        return JOB_RESULT_ERROR_COND_MX_CREATE_FAILED;
    }
    return JOB_RESULT_ERROR_COND_CREATE_FAILED;
}

JobResult JobQueueCond::finalize() SKR_NOEXCEPT
{
    skr_destroy_condition_var(&cond);
    skr_destroy_mutex(&mutex);
    return JOB_RESULT_OK;
}

JobResult JobQueueCond::broadcast() SKR_NOEXCEPT
{
    skr_wake_all_condition_vars(&cond);
    return JOB_RESULT_OK;
}

JobResult JobQueueCond::signal() SKR_NOEXCEPT
{
    skr_wake_condition_var(&cond);
    return JOB_RESULT_OK;
}

JobResult JobQueueCond::wait(uint32_t milliseconds) SKR_NOEXCEPT
{
    ThreadResult R = THREAD_RESULT_FAILED;
    if (milliseconds) {
        R = skr_wait_condition_vars(&cond, &mutex, milliseconds);
        if (R == THREAD_RESULT_TIMEOUT)
            return JOB_RESULT_ERROR_TIMEOUT;
    }
    else {
        R = skr_wait_condition_vars(&cond, &mutex, UINT32_MAX);
    }
    if (R == THREAD_RESULT_OK)
        return JOB_RESULT_OK;
    return JOB_RESULT_ERROR_UNKNOWN;
}

JobResult JobQueueCond::lock() SKR_NOEXCEPT
{
    skr_acquire_mutex(&mutex);
    return JOB_RESULT_OK;
}

JobResult JobQueueCond::unlock() SKR_NOEXCEPT
{
    skr_release_mutex(&mutex);
    return JOB_RESULT_OK;
}

}