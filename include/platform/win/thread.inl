#include <process.h> // _beginthreadex
#include <assert.h>

typedef struct SCallOnceFnWrapper {
    SCallOnceFn fn;
} SCallOnceFnWrapper;

FORCEINLINE static BOOL callOnceImpl(
PINIT_ONCE initOnce,
PVOID pWrapper,
PVOID* ppContext)
{
    SCallOnceFn fn = ((SCallOnceFnWrapper*)pWrapper)->fn;
    if (fn)
        fn();
    return TRUE;
}

FORCEINLINE static void skr_call_once(SCallOnceGuard* pGuard, SCallOnceFn pFn)
{
    DECLARE_ZERO(SCallOnceFnWrapper, wrapper)
    wrapper.fn = pFn;
    InitOnceExecuteOnce(pGuard, callOnceImpl, &wrapper, NULL);
}

FORCEINLINE static bool skr_init_mutex(SMutex* mutex)
{
    return InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&mutex->mHandle, (DWORD)MUTEX_DEFAULT_SPIN_COUNT);
}

FORCEINLINE static void skr_destroy_mutex(SMutex* mutex)
{
    CRITICAL_SECTION* cs = (CRITICAL_SECTION*)&mutex->mHandle;
    DeleteCriticalSection(cs);
    memset(&mutex->mHandle, 0, sizeof(mutex->mHandle));
}

FORCEINLINE static void skr_acquire_mutex(SMutex* mutex) { EnterCriticalSection((CRITICAL_SECTION*)&mutex->mHandle); }

FORCEINLINE static bool skr_try_acquire_mutex(SMutex* mutex) { return TryEnterCriticalSection((CRITICAL_SECTION*)&mutex->mHandle); }

FORCEINLINE static void skr_release_mutex(SMutex* mutex) { LeaveCriticalSection((CRITICAL_SECTION*)&mutex->mHandle); }

FORCEINLINE static bool skr_init_condition_var(SConditionVariable* cv)
{
    cv->pHandle = (CONDITION_VARIABLE*)sakura_calloc(1, sizeof(CONDITION_VARIABLE));
    InitializeConditionVariable((PCONDITION_VARIABLE)cv->pHandle);
    return true;
}

FORCEINLINE static void skr_destroy_condition_var(SConditionVariable* cv) { sakura_free(cv->pHandle); }

FORCEINLINE static void skr_wait_condition_vars(SConditionVariable* cv, const SMutex* pMutex, uint32_t ms)
{
    SleepConditionVariableCS((PCONDITION_VARIABLE)cv->pHandle, (PCRITICAL_SECTION)&pMutex->mHandle, ms);
}

FORCEINLINE static void skr_wake_condition_var(SConditionVariable* cv) { WakeConditionVariable((PCONDITION_VARIABLE)cv->pHandle); }

FORCEINLINE static void skr_wake_all_condition_vars(SConditionVariable* cv) { WakeAllConditionVariable((PCONDITION_VARIABLE)cv->pHandle); }

FORCEINLINE static void skr_thread_sleep(unsigned mMilliSecs) { Sleep(mMilliSecs); }

FORCEINLINE static unsigned int skr_cpu_cores_count(void)
{
    struct _SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    return systemInfo.dwNumberOfProcessors;
}

FORCEINLINE static unsigned WINAPI ThreadFunctionStatic(void* data)
{
    SThreadDesc* pDesc = (SThreadDesc*)data;
    pDesc->pFunc(pDesc->pData);
    return 0;
}

FORCEINLINE static void skr_init_thread(SThreadDesc* pDesc, SThreadHandle* pHandle)
{
    assert(pHandle != NULL);
    SThreadHandle handle = (SThreadHandle)_beginthreadex(0, 0, ThreadFunctionStatic, pDesc, 0, 0);
    assert(handle != NULL);
    *pHandle = handle;
}

static const int priorities[SKR_THREAD_PRIORITY_COUNT] = {
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_LOWEST,
    THREAD_PRIORITY_BELOW_NORMAL,
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_ABOVE_NORMAL,
    THREAD_PRIORITY_HIGHEST,
    THREAD_PRIORITY_TIME_CRITICAL
};

FORCEINLINE static void skr_set_thread_priority(SThreadHandle handle, SThreadPriority priority)
{
    SetThreadPriority((HANDLE)handle, priorities[priority]);
}

FORCEINLINE static void skr_destroy_thread(SThreadHandle handle)
{
    assert(handle != NULL);
    WaitForSingleObject((HANDLE)handle, INFINITE);
    CloseHandle((HANDLE)handle);
    handle = 0;
}

FORCEINLINE static void skr_join_thread(SThreadHandle handle) { WaitForSingleObject((HANDLE)handle, INFINITE); }