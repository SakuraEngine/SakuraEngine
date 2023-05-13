#pragma once
#include "configure.h"
#include "platform/memory.h"

#if defined(_WIN32) || defined(XBOX)
typedef unsigned long SThreadID;
    #define THREAD_ID_MAX ULONG_MAX
    #define THREAD_ID_MIN ((unsigned long)0)
#else
    #include <pthread.h>
    #if !defined(NX64)
typedef uint32_t SThreadID;
        #define THREAD_ID_MAX UINT32_MAX
        #define THREAD_ID_MIN ((uint32_t)0)
    #endif // !NX64
    RUNTIME_EXTERN_C RUNTIME_API SThreadID skrGetCurrentPthreadID();
#endif

#if defined(_WIN32) || defined(XBOX)
    #define THREADS_API RUNTIME_API
typedef struct SCallOnceGuard {
    // INIT_ONCE
    unsigned char gdStorage_[sizeof(void*)];
} SCallOnceGuard;
#endif
#ifndef THREADS_API
    #define THREADS_API
typedef pthread_once_t SCallOnceGuard;
#endif

#define INVALID_THREAD_ID 0

#if defined(_WIN32) || defined(XBOX)
    #define THREAD_LOCAL __declspec(thread)
#else
    #define THREAD_LOCAL __thread
#endif

#define INVALID_THREAD_ID 0
#define TIMEOUT_INFINITE UINT32_MAX

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SCallOnceFn)(void);

typedef struct SMutex {
#if defined(_WIN32) || defined(XBOX)
    unsigned char muStorage_[sizeof(void*)];
    uint32_t isSRW;
#else
    pthread_mutex_t pHandle;
    uint32_t mSpinCount;
#endif
} SMutex;

typedef struct SRWMutex {
#if defined(_WIN32) || defined(XBOX)
    SMutex m;
#else
    pthread_rwlock_t pHandle;
    uint32_t mSpinCount;
#endif
} SRWMutex;

typedef struct SConditionVariable {
#if defined(_WIN32) || defined(XBOX)
    unsigned char cvStorage_[sizeof(void*)];
#elif defined(NX64)
    ConditionVariableTypeNX mCondPlatformNX;
#else
    pthread_cond_t pHandle;
#endif
} SConditionVariable;

typedef enum SThreadPriority
{
    SKR_THREAD_DEFAULT,
    SKR_THREAD_LOWEST,
    SKR_THREAD_BELOW_NORMAL,
    SKR_THREAD_NORMAL,
    SKR_THREAD_ABOVE_NORMAL,
    SKR_THREAD_HIGH,
    SKR_THREAD_TIME_CRITICAL,
    SKR_THREAD_PRIORITY_COUNT
} SThreadPriority;

typedef void (*SThreadFunction)(void*);
typedef struct SThreadDesc {
#if defined(NX64)
    SThreadHandle hThread;
    void* pThreadStack;
    const char* pThreadName;
    int preferredCore;
    bool migrateEnabled;
#endif
    /// Work item description and thread index (Main thread => 0)
    SThreadFunction pFunc;
    void* pData;
} SThreadDesc;

typedef int32_t ThreadResult;
#define THREAD_RESULT_OK 1
#define THREAD_RESULT_FAILED 0
#define THREAD_RESULT_TIMEOUT -1

#if defined(_WIN32) || defined(XBOX)
typedef void* SThreadHandle;
#elif !defined(NX64)
typedef pthread_t SThreadHandle;
#endif

#define MUTEX_DEFAULT_SPIN_COUNT 1500
#if defined(_WIN32)
    #include "win/thread.inl"
#elif defined(__APPLE__)
    #include "apple/thread.inl"
#elif defined(__EMSCRIPTEN__) || defined(__wasi__)
    #include "linux/thread.inl"
#endif

/// call once
THREADS_API void skr_init_call_once_guard(SCallOnceGuard* pGuard);
THREADS_API void skr_call_once(SCallOnceGuard* pGuard, SCallOnceFn pFn);

/// Operating system mutual exclusion primitive.
THREADS_API bool skr_init_mutex(SMutex* pMutex);
THREADS_API bool skr_init_mutex_recursive(SMutex* pMutex);
THREADS_API void skr_destroy_mutex(SMutex* pMutex);
THREADS_API void skr_acquire_mutex(SMutex* pMutex);
THREADS_API bool skr_try_acquire_mutex(SMutex* pMutex);
THREADS_API void skr_release_mutex(SMutex* pMutex);

// rw mutex
THREADS_API bool skr_init_mutex_rw(SRWMutex* pMutex);
THREADS_API void skr_destroy_rw_mutex(SRWMutex* pMutex);
THREADS_API void skr_acquire_mutex_r(SRWMutex* pMutex);
THREADS_API void skr_acquire_mutex_w(SRWMutex* pMutex);
THREADS_API void skr_release_rw_mutex(SRWMutex* pMutex);

/// cv
THREADS_API bool skr_init_condition_var(SConditionVariable* cv);
THREADS_API void skr_destroy_condition_var(SConditionVariable* cv);
THREADS_API ThreadResult skr_wait_condition_vars(SConditionVariable* cv, const SMutex* pMutex, uint32_t timeout);
THREADS_API void skr_wake_all_condition_vars(SConditionVariable* cv);
THREADS_API void skr_wake_condition_var(SConditionVariable* cv);

/// thread
THREADS_API void skr_init_thread(SThreadDesc* pItem, SThreadHandle* pHandle);
THREADS_API SThreadPriority skr_set_thread_priority(SThreadHandle, SThreadPriority);
THREADS_API void skr_destroy_thread(SThreadHandle handle);
THREADS_API void skr_join_thread(SThreadHandle handle);
THREADS_API SThreadID skr_current_thread_id(void);
THREADS_API void skr_thread_sleep(unsigned mMilliSecs);
THREADS_API unsigned int skr_cpu_cores_count(void);

#ifdef __cplusplus
}
struct SMutexLock {
    SMutexLock(SMutex& rhs)
        : mMutex(rhs)
    {
        skr_acquire_mutex(&rhs);
    }
    ~SMutexLock() { skr_release_mutex(&mMutex); }

    /// Prevent copy construction.
    SMutexLock(const SMutexLock& rhs) = delete;
    /// Prevent assignment.
    SMutexLock& operator=(const SMutexLock& rhs) = delete;

    SMutex& mMutex;
};
struct SMutexObject {
    SMutex mMutex;
    SMutexObject()
    {
        skr_init_mutex(&mMutex);
    }
    ~SMutexObject()
    {
        skr_destroy_mutex(&mMutex);
    }
};
#endif
