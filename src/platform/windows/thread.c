#include "platform/thread.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "synchapi.h"
#include <process.h> // _beginthreadex
#include <assert.h>

/// implementation of callonce

typedef struct SCallOnceFnWrapper {
    SCallOnceFn fn;
} SCallOnceFnWrapper;

FORCEINLINE static BOOL callOnceImpl(
PINIT_ONCE initOnce,
PVOID pWrapper,
PVOID* ppContext)
{
    SCallOnceFn fn = ((SCallOnceFnWrapper*)pWrapper)->fn;
    if (fn) fn();
    return TRUE;
}

void skr_call_once(SCallOnceGuard* pGuard, SCallOnceFn pFn)
{
    INIT_ONCE* once_ = (INIT_ONCE*)pGuard->gdStorage_;
    DECLARE_ZERO(SCallOnceFnWrapper, wrapper)
    wrapper.fn = pFn;
    InitOnceExecuteOnce(once_, callOnceImpl, &wrapper, NULL);
}

void skr_init_call_once_guard(SCallOnceGuard* pGuard)
{
    INIT_ONCE* once_ = (INIT_ONCE*)pGuard->gdStorage_;
    InitOnceInitialize(once_);
}

/// implementation of mutex

static_assert(sizeof(SRWLOCK) == sizeof(void*), "`mu_storage_` does not have the same size as SRWLOCK");
static_assert(_Alignof(SRWLOCK) == _Alignof(void*), "`mu_storage_` does not have the same alignment as SRWLOCK");

#if (_WIN32_WINNT >= 0x0600)
bool skr_init_mutex(SMutex* mutex)
{
    PSRWLOCK pSrwlock = (PSRWLOCK)mutex->muStorage_;
    InitializeSRWLock(pSrwlock);
    return true;
}

void skr_destroy_mutex(SMutex* mutex)
{
    memset(mutex->muStorage_, 0, sizeof(mutex->muStorage_));
}

void skr_acquire_mutex(SMutex* mutex)
{
    AcquireSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
}

bool skr_try_acquire_mutex(SMutex* mutex)
{
    return TryAcquireSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
}

void skr_release_mutex(SMutex* mutex)
{
    ReleaseSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
}
#endif

/// implementation of cv

bool skr_init_condition_var(SConditionVariable* cv)
{
    CONDITION_VARIABLE* cv_ = (CONDITION_VARIABLE*)(cv->cvStorage_);
    InitializeConditionVariable((PCONDITION_VARIABLE)cv_);
    return true;
}

void skr_destroy_condition_var(SConditionVariable* cv)
{
}

void skr_wait_condition_vars(SConditionVariable* cv, const SMutex* pMutex, uint32_t ms)
{
    CONDITION_VARIABLE* cv_ = (CONDITION_VARIABLE*)(cv->cvStorage_);
    PSRWLOCK pSrwlock = (PSRWLOCK)(pMutex->muStorage_);

    BOOLEAN acquired = TryAcquireSRWLockExclusive(pSrwlock);
    SleepConditionVariableSRW((PCONDITION_VARIABLE)cv_, pSrwlock, ms, 0);
    if (acquired) ReleaseSRWLockExclusive(pSrwlock);
}

void skr_wake_condition_var(SConditionVariable* cv)
{
    CONDITION_VARIABLE* cv_ = (CONDITION_VARIABLE*)(cv->cvStorage_);

    WakeConditionVariable((PCONDITION_VARIABLE)cv_);
}

void skr_wake_all_condition_vars(SConditionVariable* cv)
{
    CONDITION_VARIABLE* cv_ = (CONDITION_VARIABLE*)(cv->cvStorage_);

    WakeAllConditionVariable((PCONDITION_VARIABLE)cv_);
}

/// implementation of threads
void skr_thread_sleep(unsigned mMilliSecs) { Sleep(mMilliSecs); }

unsigned int skr_cpu_cores_count(void)
{
    struct _SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    return systemInfo.dwNumberOfProcessors;
}

unsigned WINAPI ThreadFunctionStatic(void* data)
{
    SThreadDesc* pDesc = (SThreadDesc*)data;
    pDesc->pFunc(pDesc->pData);
    return 0;
}

void skr_init_thread(SThreadDesc* pDesc, SThreadHandle* pHandle)
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

void skr_set_thread_priority(SThreadHandle handle, SThreadPriority priority)
{
    SetThreadPriority((HANDLE)handle, priorities[priority]);
}

void skr_destroy_thread(SThreadHandle handle)
{
    assert(handle != NULL);
    WaitForSingleObject((HANDLE)handle, INFINITE);
    CloseHandle((HANDLE)handle);
    handle = 0;
}

void skr_join_thread(SThreadHandle handle) { WaitForSingleObject((HANDLE)handle, INFINITE); }