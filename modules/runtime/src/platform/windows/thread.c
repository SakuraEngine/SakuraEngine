#include "winheaders.h"
#include <process.h> // _beginthreadex

#include "platform/thread.h"
#include "platform/debug.h"
#include <assert.h>
#include <synchapi.h>

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

void skr_mutex_acquire_cs(SMutex* mutex)
{
    EnterCriticalSection(*(CRITICAL_SECTION**)mutex->muStorage_);
}

bool skr_mutex_try_acquire_cs(SMutex* mutex)
{
    return TryEnterCriticalSection(*(CRITICAL_SECTION**)mutex->muStorage_);
}

void skr_mutex_release_cs(SMutex* mutex)
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

void skr_mutex_acquire_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    AcquireSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
#else
    skr_mutex_acquire_cs(mutex);
#endif
}

void skr_mutex_acquire_srw_shared(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    AcquireSRWLockShared((PSRWLOCK)mutex->muStorage_);
#else
    skr_mutex_acquire_cs(mutex);
#endif
}

bool skr_mutex_try_acquire_srw(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    return TryAcquireSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
#else
    return skr_mutex_try_acquire_cs(mutex);
#endif
}

bool skr_mutex_try_acquire_srw_shared(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    return TryAcquireSRWLockShared((PSRWLOCK)mutex->muStorage_);
#else
    return skr_mutex_try_acquire_cs(mutex);
#endif
}

void skr_mutex_release_srw_exclusive(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    ReleaseSRWLockExclusive((PSRWLOCK)mutex->muStorage_);
#else
    skr_mutex_release_cs(mutex);
#endif
}

void skr_mutex_release_srw_shared(SMutex* mutex)
{
#if (_WIN32_WINNT >= 0x0600)
    ReleaseSRWLockShared((PSRWLOCK)mutex->muStorage_);
#else
    skr_mutex_release_cs(mutex);
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

void skr_mutex_acquire(SMutex* mutex)
{
    if (mutex->isSRW)
        skr_mutex_acquire_srw(mutex);
    else
        skr_mutex_acquire_cs(mutex);
}

bool skr_mutex_try_acquire(SMutex* mutex)
{
    if (mutex->isSRW)
        return skr_mutex_try_acquire_srw(mutex);
    else
        return skr_mutex_try_acquire_cs(mutex);
}

void skr_mutex_release(SMutex* mutex)
{
    if (mutex->isSRW)
        skr_mutex_release_srw_exclusive(mutex);
    else
        skr_mutex_release_cs(mutex);
}

/// implementation of rw mutex

bool skr_init_rw_mutex(SRWMutex* pMutex)
{
    return skr_init_mutex_srw(&pMutex->m);
}

void skr_destroy_rw_mutex(SRWMutex* pMutex) 
{ 
    skr_destroy_mutex(&pMutex->m);
}

void skr_rw_mutex_acquire_r(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_mutex_acquire_srw_shared(&pMutex->m);
    else
        skr_mutex_acquire_cs(&pMutex->m);
}

void skr_rw_mutex_acquire_w(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_mutex_acquire_srw(&pMutex->m);
    else
        skr_mutex_acquire_cs(&pMutex->m);
}

void skr_rw_mutex_release_w(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_mutex_release_srw_exclusive(&pMutex->m);
    else
        skr_mutex_release_cs(&pMutex->m);
}

void skr_rw_mutex_release_r(SRWMutex* pMutex)
{
    if (pMutex->m.isSRW)
        skr_mutex_release_srw_shared(&pMutex->m);
    else
        skr_mutex_release_cs(&pMutex->m);
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

ThreadResult skr_wait_condition_vars(SConditionVariable* cv, const SMutex* pMutex, uint32_t ms)
{
    CONDITION_VARIABLE* cv_ = (CONDITION_VARIABLE*)(cv->cvStorage_);
    BOOL R = FALSE;
    if (pMutex->isSRW)
    {
        PSRWLOCK pSrwlock = (PSRWLOCK)(pMutex->muStorage_);
        R = SleepConditionVariableSRW((PCONDITION_VARIABLE)cv_, pSrwlock, ms, 0);
    }
    else
    {
        PCRITICAL_SECTION* ppCS = (PCRITICAL_SECTION*)(pMutex->muStorage_);
        R = SleepConditionVariableCS((PCONDITION_VARIABLE)cv_, *ppCS, ms);
    }
    if (R) return THREAD_RESULT_OK;
    if (GetLastError() == ERROR_TIMEOUT) return THREAD_RESULT_TIMEOUT;
    return THREAD_RESULT_FAILED;
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
    SThreadHandle handle = (SThreadHandle)_beginthreadex(
        0, 0, &ThreadFunctionStatic, 
        pDesc, 0, 0);
    assert(handle != NULL);
    *pHandle = handle;
}

SThreadHandle skr_get_current_thread()
{
    return (SThreadHandle)GetCurrentThread();
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

SThreadPriority skr_thread_set_priority(SThreadHandle handle, SThreadPriority priority)
{
    BOOL ok = SetThreadPriority((HANDLE)handle, priorities[priority]);
    if (ok) return priority;
    return GetThreadPriority((HANDLE)handle);
}

void skr_thread_set_affinity(SThreadHandle handle, uint64_t affinityMask)
{
    SetThreadAffinityMask((HANDLE)handle, (DWORD_PTR)affinityMask);
}

char8_t* thread_name()
{
	THREAD_LOCAL static char8_t name[SKR_MAX_THREAD_NAME_LENGTH + 1];
	return name;
}

void skr_current_thread_set_name(const char8_t* pName)
{
    size_t len = strlen(pName);
    wchar_t* buffer = (wchar_t*)sakura_malloc((len + 1) * sizeof(wchar_t));

    size_t resultLength = MultiByteToWideChar(CP_UTF8, 0, pName, (int)len, buffer, (int)len);
    buffer[resultLength] = 0;

    SThreadHandle handle = skr_get_current_thread();
    SetThreadDescription((HANDLE)handle, buffer);
    strcpy_s(thread_name(), SKR_MAX_THREAD_NAME_LENGTH + 1, pName);

    sakura_free(buffer);
}

const char8_t* skr_current_thread_get_name()
{
    return thread_name();
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