#pragma once
#include "job/thread_job.hpp"

namespace skr
{

struct JobThreadFunction
{
    virtual ~JobThreadFunction() SKR_NOEXCEPT;
    virtual JobResult run() SKR_NOEXCEPT = 0;
};

struct JobQueueThread
{
public:
    JobQueueThread() SKR_NOEXCEPT;
    JobQueueThread(const char8_t *name, JobQueuePriority priority, uint32_t stackSize, const JobQueueThreadDesc *desc = nullptr) SKR_NOEXCEPT;
    virtual ~JobQueueThread() SKR_NOEXCEPT;
	
    // start the thread.
    // @retval JOB_RESULT_OK if success
    JobResult start(JobThreadFunction *pFunc) SKR_NOEXCEPT;

    // wait for thread completion.
    // @retval JOB_RESULT_OK if success
    JobResult join() SKR_NOEXCEPT;

    // check if thread is alive.
    bool is_alive() const SKR_NOEXCEPT;

    // get thread id.
    SThreadID get_id() const SKR_NOEXCEPT;

    // change thread priority.
    SThreadPriority change_priority(SThreadPriority priority) SKR_NOEXCEPT;

    // get thread function object.
    JobThreadFunction* get_function() const SKR_NOEXCEPT;

    // initlaize thread.
    // @retval JOB_RESULT_OK if success
    JobResult initialize(const char8_t *name, int32_t priority, uint32_t stack_size, const JobQueueThreadDesc *pdesc = nullptr) SKR_NOEXCEPT;
    
    // finalize thread.
    // @retval JOB_RESULT_OK if success
    JobResult finalize() SKR_NOEXCEPT;

private:
    skr::text::text tname;
    JobQueueThreadDesc desc = {};

    static void jobThreadFunc(void* args);
    SThreadDesc tDesc;
    SThreadID tID;
    SThreadHandle tHandle;
    SAtomic32 started = false;
    SAtomic32 alive = false;
    JobThreadFunction* func = nullptr;
};

struct JobQueueCondDesc
{
    int32_t __nothing__;
};

struct JobQueueCond
{
    JobQueueCond() SKR_NOEXCEPT {}
    JobQueueCond(const char8_t *name, const JobQueueCondDesc *JobQueueCondDesc = nullptr) SKR_NOEXCEPT;
    virtual ~JobQueueCond() SKR_NOEXCEPT;

    // @retval JOB_RESULT_OK if success
	JobResult initialize(const char8_t *name, const JobQueueCondDesc *pDesc = nullptr) SKR_NOEXCEPT;
    
    // @retval JOB_RESULT_OK if success
    JobResult finalize() SKR_NOEXCEPT;

    // wake up all threads waiting on condition variable.
    // @retval JOB_RESULT_OK if success
    JobResult broadcast() SKR_NOEXCEPT;

    // wake up a thread waiting on condition variable.
    // @retval JOB_RESULT_OK if success
    JobResult signal() SKR_NOEXCEPT;

    // wait by condition variable
    // @retval JOB_RESULT_OK if success
    JobResult wait(uint32_t milliseconds = UINT32_MAX) SKR_NOEXCEPT;

    // lock a mutex to be paired with a condition variable
    // @retval JOB_RESULT_OK if success
    JobResult lock() SKR_NOEXCEPT;

    // unlock a mutex to be paired with a condition variable
    // @retval JOB_RESULT_OK if success
    JobResult unlock() SKR_NOEXCEPT;

private:
    SConditionVariable cond;
    SMutex mutex;
};

}