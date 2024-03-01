#include <sys/sysctl.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

SKR_FORCEINLINE static void callOnce(SCallOnceGuard* pGuard, SCallOnceFn pFn)
{
    pthread_once(pGuard, pFn);
}

SKR_FORCEINLINE static bool skr_init_mutex(SMutex* pMutex)
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

SKR_FORCEINLINE static bool skr_init_mutex_recursive(SMutex* pMutex)
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

SKR_FORCEINLINE static void skr_destroy_mutex(SMutex* pMutex) { pthread_mutex_destroy(&pMutex->pHandle); }

SKR_FORCEINLINE static void skr_mutex_acquire(SMutex* pMutex)
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

SKR_FORCEINLINE static bool skr_mutex_try_acquire(SMutex* pMutex) { return pthread_mutex_trylock(&pMutex->pHandle) == 0; }

SKR_FORCEINLINE static void skr_mutex_release(SMutex* pMutex) { pthread_mutex_unlock(&pMutex->pHandle); }

SKR_FORCEINLINE static bool skr_init_condition_var(SConditionVariable* pCv)
{
    pCv->pHandle = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    int res = pthread_cond_init(&pCv->pHandle, NULL);
    assert(res == 0);
    return res == 0;
}

SKR_FORCEINLINE static void skr_destroy_condition_var(SConditionVariable* pCv) { pthread_cond_destroy(&pCv->pHandle); }

SKR_FORCEINLINE static ThreadResult skr_wait_condition_vars(SConditionVariable* pCv, const SMutex* mutex, uint32_t ms)
{
    pthread_mutex_t* mutexHandle = (pthread_mutex_t*)&mutex->pHandle;
    int ret = 0;
    if (ms == TIMEOUT_INFINITE)
    {
        ret = pthread_cond_wait(&pCv->pHandle, mutexHandle);
    }
    else
    {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000;
        ret = pthread_cond_timedwait(&pCv->pHandle, mutexHandle, &ts);
        if (ret == ETIMEDOUT) return THREAD_RESULT_TIMEOUT;
    }
    if (ret == 0) return THREAD_RESULT_OK;
    return THREAD_RESULT_FAILED;
}

SKR_FORCEINLINE static void skr_wake_condition_var(SConditionVariable* pCv) { pthread_cond_signal(&pCv->pHandle); }

SKR_FORCEINLINE static void skr_wake_all_condition_vars(SConditionVariable* pCv) { pthread_cond_broadcast(&pCv->pHandle); }

SKR_FORCEINLINE static void skr_thread_sleep(unsigned mMilliSecs) { usleep(mMilliSecs * 1000); }

// threading class (Static functions)
SKR_FORCEINLINE static unsigned int skr_cpu_cores_count(void)
{
    size_t len;
    unsigned int ncpu;
    ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    return ncpu;
}

SKR_FORCEINLINE static void* ThreadFunctionStatic(void* data)
{
    SThreadDesc* pItem = (SThreadDesc*)(data);
    pItem->pFunc(pItem->pData);
    return 0;
}

SKR_FORCEINLINE static void skr_thread_set_affinity(SThreadHandle handle, uint64_t affinityMask)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (uint32_t i = 0; i < 64; ++i)
    {
        if (affinityMask & (1 << i))
        {
            CPU_SET(i, &cpuset);
        }
    }
    pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
}

SKR_FORCEINLINE static void skr_init_thread(SThreadDesc* pData, SThreadHandle* pHandle)
{
    int res = pthread_create(pHandle, NULL, ThreadFunctionStatic, pData);
    assert(res == 0);
}

SKR_FORCEINLINE static void skr_destroy_thread(SThreadHandle handle)
{
    pthread_join(handle, NULL);
    handle = NULL;
}

SKR_FORCEINLINE static void skr_join_thread(SThreadHandle handle) { pthread_join(handle, NULL); }

SKR_FORCEINLINE SThreadID skr_current_thread_id(void)
{
    return skrGetCurrentPthreadID();
}