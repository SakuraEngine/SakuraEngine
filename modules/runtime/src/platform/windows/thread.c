#include "platform/thread.h"
#include "platform/debug.h"
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

#if (_WIN32_WINNT >= 0x0600)
static_assert(sizeof(SRWLOCK) == sizeof(void*), "`mu_storage_` does not have the same size as SRWLOCK");
static_assert(_Alignof(SRWLOCK) == _Alignof(void*), "`mu_storage_` does not have the same alignment as SRWLOCK");
#endif

// Critical Section Lock
bool skr_init_mutex_cs(SMutex* mutex)
{
    mutex->isSRW = 0;
    CRITICAL_SECTION** pCS = (CRITICAL_SECTION**)mutex->muStorage_;
    *pCS = sakura_calloc(1, sizeof(CRITICAL_SECTION));
    return InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)*pCS, (DWORD)MUTEX_DEFAULT_SPIN_COUNT);
}

void skr_destroy_mutex_cs(SMutex* mutex)
{
    CRITICAL_SECTION* cs = *(CRITICAL_SECTION**)mutex->muStorage_;
    DeleteCriticalSection(cs);
    sakura_free(cs);
}

void skr_acquire_mutex_cs(SMutex* mutex)
{
    EnterCriticalSection(*(CRITICAL_SECTION**)mutex->muStorage_);
}

bool skr_try_acquire_mutex_cs(SMutex* mutex)
{
    return TryEnterCriticalSection(*(CRITICAL_SECTION**)mutex->muStorage_);
}

void skr_release_mutex_cs(SMutex* mutex)
{
    LeaveCriticalSection(*(CRITICAL_SECTION**)mutex->muStorage_);
}

// SRW Lock
bool skr_init_mutex_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    PSRWLOCK pSrwlock = (PSRWLOCK)mutex->muStorage_;
    InitializeSRWLock(pSrwlock);
    mutex->isSRW = 1;
    return true;
#else
    return skr_init_mutex_cs(mutex);
#endif
}

void skr_destroy_mutex_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    memset(mutex->muStorage_, 0, sizeof(mutex->muStorage_));
#else
    skr_destroy_mutex_cs(mutex);
#endif
}

void skr_acquire_mutex_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    AcquireSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
#else
    skr_acquire_mutex_cs(mutex);
#endif
}

void skr_acquire_mutex_srw_shared(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    AcquireSRWLockShared((PSRWLOCK)mutex->muStorage_);
#else
    skr_acquire_mutex_cs(mutex);
#endif
}

bool skr_try_acquire_mutex_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    return TryAcquireSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
#else
    return skr_try_acquire_mutex_cs(mutex);
#endif
}

bool skr_try_acquire_mutex_srw_shared(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    return TryAcquireSRWLockShared((PSRWLOCK)mutex->muStorage_);
#else
    return skr_try_acquire_mutex_cs(mutex);
#endif
}

void skr_release_mutex_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    ReleaseSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
#else
    skr_release_mutex_cs(mutex);
#endif
}

bool skr_init_mutex(SMutex* mutex)
{
    return skr_init_mutex_srw(mutex);
}

bool skr_init_mutex_recursive(SMutex* mutex)
{
    return skr_init_mutex_cs(mutex);
}

void skr_destroy_mutex(SMutex* mutex)
{
    if (mutex->isSRW)
        skr_destroy_mutex_srw(mutex);
    else
        skr_destroy_mutex_cs(mutex);
}

void skr_acquire_mutex(SMutex* mutex)
{
    if (mutex->isSRW)
        skr_acquire_mutex_srw(mutex);
    else
        skr_acquire_mutex_cs(mutex);
}

bool skr_try_acquire_mutex(SMutex* mutex)
{
    if (mutex->isSRW)
        return skr_try_acquire_mutex_srw(mutex);
    else
        return skr_try_acquire_mutex_cs(mutex);
}

void skr_release_mutex(SMutex* mutex)
{
    if (mutex->isSRW)
        skr_release_mutex_srw(mutex);
    else
        skr_release_mutex_cs(mutex);
}

/// implementation of rw mutex

bool skr_init_mutex_rw(SRWMutex* pMutex)
{
    return skr_init_mutex_srw(&pMutex->m);
}

void skr_destroy_rw_mutex(SRWMutex* pMutex) 
{ 
    skr_destroy_mutex(&pMutex->m);
}

void skr_acquire_mutex_r(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_acquire_mutex_srw_shared(&pMutex->m);
    else
        skr_acquire_mutex_cs(&pMutex->m);
}

void skr_acquire_mutex_w(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_acquire_mutex_srw(&pMutex->m);
    else
        skr_acquire_mutex_cs(&pMutex->m);
}

void skr_release_rw_mutex(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_release_mutex_srw(&pMutex->m);
    else
        skr_release_mutex_cs(&pMutex->m);
}

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
    if (pMutex->isSRW)
    {
        PSRWLOCK pSrwlock = (PSRWLOCK)(pMutex->muStorage_);
        SleepConditionVariableSRW((PCONDITION_VARIABLE)cv_, pSrwlock, ms, 0);
    }
    else
    {
        PCRITICAL_SECTION* ppCS = (PCRITICAL_SECTION*)(pMutex->muStorage_);
        SleepConditionVariableCS((PCONDITION_VARIABLE)cv_, *ppCS, ms);
    }
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

SThreadID skr_current_thread_id(void)
{
    return GetCurrentThreadId();
}