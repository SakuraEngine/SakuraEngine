#include <sys/sysctl.h>
#include <time.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <unistd.h>

FORCEINLINE static void callOnce(SCallOnceGuard* pGuard, SCallOnceFn pFn)
{
    pthread_once(pGuard, pFn);
}

FORCEINLINE static bool skr_init_mutex(SMutex* pMutex)
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

FORCEINLINE static void skr_destroy_mutex(SMutex* pMutex) { pthread_mutex_destroy(&pMutex->pHandle); }

FORCEINLINE static void skr_acquire_mutex(SMutex* pMutex)
{
    uint32_t count = 0;

    while (count < pMutex->mSpinCount && pthread_mutex_trylock(&pMutex->pHandle) != 0)
        ++count;

    if (count == pMutex->mSpinCount)
    {
        int r = pthread_mutex_lock(&pMutex->pHandle);
        (void)r;
        assert(r == 0 && "Mutex::Acquire failed to take the lock");
    }
}

FORCEINLINE static bool skr_try_acquire_mutex(SMutex* pMutex) { return pthread_mutex_trylock(&pMutex->pHandle) == 0; }

FORCEINLINE static void skr_release_mutex(SMutex* pMutex) { pthread_mutex_unlock(&pMutex->pHandle); }

FORCEINLINE static bool skr_init_condition_var(SConditionVariable* pCv)
{
    pCv->pHandle = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    int res = pthread_cond_init(&pCv->pHandle, NULL);
    assert(res == 0);
    return res == 0;
}

FORCEINLINE static void skr_destroy_condition_var(SConditionVariable* pCv) { pthread_cond_destroy(&pCv->pHandle); }

FORCEINLINE static void skr_wait_condition_vars(SConditionVariable* pCv, const SMutex* mutex, uint32_t ms)
{
    pthread_mutex_t* mutexHandle = (pthread_mutex_t*)&mutex->pHandle;

    if (ms == TIMEOUT_INFINITE)
    {
        pthread_cond_wait(&pCv->pHandle, mutexHandle);
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

        pthread_cond_timedwait(&pCv->pHandle, mutexHandle, &time);
    }
}

FORCEINLINE static void skr_wake_condition_var(SConditionVariable* pCv) { pthread_cond_signal(&pCv->pHandle); }

FORCEINLINE static void skr_wake_all_condition_vars(SConditionVariable* pCv) { pthread_cond_broadcast(&pCv->pHandle); }

FORCEINLINE static void skr_thread_sleep(unsigned mMilliSecs) { usleep(mMilliSecs * 1000); }

// threading class (Static functions)
FORCEINLINE static unsigned int skr_cpu_cores_count(void)
{
    size_t len;
    unsigned int ncpu;
    len = sizeof(ncpu);
    sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0);
    return ncpu;
}

FORCEINLINE static void* ThreadFunctionStatic(void* data)
{
    SThreadDesc* pItem = (SThreadDesc*)(data);
    pItem->pFunc(pItem->pData);
    return 0;
}

const uint32_t priorities[SKR_THREAD_PRIORITY_COUNT] = { 
    0, 1, 25, 50, 75, 75, 99 
};

FORCEINLINE static void skr_set_thread_priority(SThreadHandle handle, SThreadPriority priority)
{
    struct sched_param param = {};
    param.sched_priority = priorities[priority];
    if (priority == SKR_THREAD_DEFAULT)
        return;
    else if(priority > SKR_THREAD_ABOVE_NORMAL)
    {
        pthread_setschedparam(handle, SCHED_FIFO, &param);
    }
    else
    {
        pthread_setschedparam(handle, SCHED_RR, &param);
    }
}

FORCEINLINE static void skr_init_thread(SThreadDesc* pData, SThreadHandle* pHandle)
{
    int res = pthread_create(pHandle, NULL, ThreadFunctionStatic, pData);
    skr_set_thread_priority(*pHandle, SKR_THREAD_NORMAL);
    UNREF_PARAM(res);
    assert(res == 0);
}

FORCEINLINE static void skr_destroy_thread(SThreadHandle handle)
{
    pthread_join(handle, NULL);
    handle = NULL;
}

FORCEINLINE static void skr_join_thread(SThreadHandle handle) { pthread_join(handle, NULL); }
