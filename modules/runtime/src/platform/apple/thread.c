#include "SkrRT/platform/thread.h"
#include "SkrBase/misc/debug.h" 
#include <sys/sysctl.h>
#include <time.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>

void skr_call_once(SCallOnceGuard* pGuard, SCallOnceFn pFn)
{
    pthread_once(pGuard, pFn);
}

void skr_init_call_once_guard(SCallOnceGuard* pGuard)
{
    pthread_once_t once_ = PTHREAD_ONCE_INIT;
    pthread_once_t* pOnce = (pthread_once_t*)pGuard;
    *pOnce = once_;
}

bool skr_init_mutex(SMutex* pMutex)
{
    pMutex->mSpinCount = MUTEX_DEFAULT_SPIN_COUNT;
    pMutex->pHandle = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pthread_mutexattr_t attr;
    int status = pthread_mutexattr_init(&attr);
    status |= pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    status |= pthread_mutex_init(&pMutex->pHandle, &attr);
    status |= pthread_mutexattr_destroy(&attr);
    return status == 0;
}

bool skr_init_mutex_recursive(SMutex* pMutex)
{
    pMutex->mSpinCount = MUTEX_DEFAULT_SPIN_COUNT;
    pMutex->pHandle = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pthread_mutexattr_t attr;
    int status = pthread_mutexattr_init(&attr);
    status |= pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    status |= pthread_mutex_init(&pMutex->pHandle, &attr);
    status |= pthread_mutexattr_destroy(&attr);
    return status == 0;
}

void skr_destroy_mutex(SMutex* pMutex) 
{ 
    pthread_mutex_destroy(&pMutex->pHandle); 
}

void skr_mutex_acquire(SMutex* pMutex)
{
    uint32_t count = 0;

    while (count < pMutex->mSpinCount && pthread_mutex_trylock(&pMutex->pHandle) != 0)
        ++count;

    if (count == pMutex->mSpinCount)
    {
        int r = pthread_mutex_lock(&pMutex->pHandle);
        (void)r;
        SKR_ASSERT(r == 0 && "Mutex::Acquire failed to take the lock");
    }
}

bool skr_mutex_try_acquire(SMutex* pMutex) 
{ 
    return pthread_mutex_trylock(&pMutex->pHandle) == 0; 
}

void skr_mutex_release(SMutex* pMutex) 
{ 
    pthread_mutex_unlock(&pMutex->pHandle); 
}

/// implementation of rw mutex

bool skr_init_rw_mutex(SRWMutex* pMutex)
{
    pMutex->mSpinCount = MUTEX_DEFAULT_SPIN_COUNT;
    int err = pthread_rwlock_init(&pMutex->pHandle, NULL);
    SKR_ASSERT(err == 0 && "RWMutex::Init: failed to initialize the rw lock");
    return err == 0;
}

void skr_destroy_rw_mutex(SRWMutex* pMutex) 
{ 
    pthread_rwlock_destroy(&pMutex->pHandle); 
}

void skr_rw_mutex_acquire_r(SRWMutex* pMutex)
{
    uint32_t count = 0;

    while (count < pMutex->mSpinCount && pthread_rwlock_tryrdlock(&pMutex->pHandle) != 0)
        ++count;

    if (count == pMutex->mSpinCount)
    {
        int r = pthread_rwlock_rdlock(&pMutex->pHandle);
        (void)r;
        SKR_ASSERT(r != EINVAL && "RWMutex::AcquireR: The value specified by rwlock does not refer to an initialised read-write lock object.");
        SKR_ASSERT(r != EDEADLK && "RWMutex::AcquireR: Current thread already owns the read-write lock for writing or reading.");
        SKR_ASSERT(r == 0 && "RWMutex::AcquireR: failed to take the read lock");
    }
}

void skr_rw_mutex_acquire_w(SRWMutex* pMutex)
{
    uint32_t count = 0;

    while (count < pMutex->mSpinCount && pthread_rwlock_trywrlock(&pMutex->pHandle) != 0)
        ++count;

    if (count == pMutex->mSpinCount)
    {
        int r = pthread_rwlock_wrlock(&pMutex->pHandle);
        SKR_ASSERT(r != EINVAL && "RWMutex::AcquireW: The value specified by rwlock does not refer to an initialised read-write lock object.");
        SKR_ASSERT(r != EDEADLK && "RWMutex::AcquireW: Current thread already owns the read-write lock for writing or reading.");
        SKR_ASSERT(r == 0 && "RWMutex::AcquireW: failed to take the write lock");
    }
}

void skr_rw_mutex_release_w(SRWMutex* pMutex)
{
    pthread_rwlock_unlock(&pMutex->pHandle); 
}

void skr_rw_mutex_release_r(SRWMutex* pMutex)
{
    pthread_rwlock_unlock(&pMutex->pHandle); 
}

/// implementation of cv

bool skr_init_condition_var(SConditionVariable* pCv)
{
    pCv->pHandle = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    int res = pthread_cond_init(&pCv->pHandle, NULL);
    SKR_ASSERT(res == 0);
    return res == 0;
}

void skr_destroy_condition_var(SConditionVariable* pCv) { pthread_cond_destroy(&pCv->pHandle); }

ThreadResult skr_wait_condition_vars(SConditionVariable* pCv, const SMutex* mutex, uint32_t ms)
{
    pthread_mutex_t* mutexHandle = (pthread_mutex_t*)&mutex->pHandle;
    int ret = 0;
    if (ms == TIMEOUT_INFINITE)
    {
        ret = pthread_cond_wait(&pCv->pHandle, mutexHandle);
    }
    else
    {
        struct timespec time;
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        time.tv_sec = mts.tv_sec + ms / 1000;
        time.tv_nsec = mts.tv_nsec + (ms % 1000) * 1000;

        ret = pthread_cond_timedwait(&pCv->pHandle, mutexHandle, &time);
        if (ret == ETIMEDOUT) return THREAD_RESULT_TIMEOUT;
    }
    if (ret == 0) return THREAD_RESULT_OK;
    return THREAD_RESULT_FAILED;
}

void skr_wake_condition_var(SConditionVariable* pCv) { pthread_cond_signal(&pCv->pHandle); }

void skr_wake_all_condition_vars(SConditionVariable* pCv) { pthread_cond_broadcast(&pCv->pHandle); }

void skr_thread_sleep(unsigned mMilliSecs) { usleep(mMilliSecs * 1000); }

// threading class (Static functions)
unsigned int skr_cpu_cores_count(void)
{
    size_t len;
    unsigned int ncpu;
    len = sizeof(ncpu);
    sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0);
    return ncpu;
}

void* ThreadFunctionStatic(void* data)
{
    SThreadDesc* pItem = (SThreadDesc*)(data);
    pItem->pFunc(pItem->pData);
    return 0;
}

static const uint32_t priorities[SKR_THREAD_PRIORITY_COUNT] = {
    0, 1, 25, 50, 75, 75, 99
};

SThreadPriority skr_thread_set_priority(SThreadHandle handle, SThreadPriority priority)
{
    struct sched_param param = {};
    param.sched_priority = priorities[priority];
    if (priority == SKR_THREAD_DEFAULT)
        return priority;
    else if (priority > SKR_THREAD_ABOVE_NORMAL)
    {
        pthread_setschedparam(handle, SCHED_FIFO, &param);
    }
    else
    {
        pthread_setschedparam(handle, SCHED_RR, &param);
    }
    return priority;
}

void skr_thread_set_affinity(SThreadHandle handle, uint64_t affinityMask)
{
    thread_affinity_policy_data_t policy = { (int32_t)affinityMask };
    thread_policy_set(pthread_mach_thread_np(handle), THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
}

char8_t* thread_name()
{
	THREAD_LOCAL static char8_t name[SKR_MAX_THREAD_NAME_LENGTH + 1];
	return name;
}

void skr_current_thread_set_name(const char8_t* pName)
{
    pthread_setname_np(pName);
    strcpy(thread_name(), pName);
}

const char8_t* skr_current_thread_get_name()
{
    return thread_name();
}

void skr_init_thread(SThreadDesc* pData, SThreadHandle* pHandle)
{
    int res = pthread_create(pHandle, NULL, ThreadFunctionStatic, pData);
    skr_thread_set_priority(*pHandle, SKR_THREAD_NORMAL);
    SKR_UNREF_PARAM(res);
    SKR_ASSERT(res == 0);
}

SThreadHandle skr_get_current_thread()
{
    return (SThreadHandle)pthread_self();
}

void skr_destroy_thread(SThreadHandle handle)
{
    pthread_join(handle, NULL);
    handle = NULL;
}

void skr_join_thread(SThreadHandle handle) { pthread_join(handle, NULL); }

SThreadID skr_current_thread_id(void)
{
    return skrGetCurrentPthreadID();
}
