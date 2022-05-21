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
#endif

#define INVALID_THREAD_ID 0

#if defined(_WIN32) || defined(XBOX)
    #define WIN32_LEAN_AND_MEAN
    #include "windows.h"
    #include "synchapi.h"
    #define THREAD_LOCAL __declspec(thread)
    #define INIT_CALL_ONCE_GUARD INIT_ONCE_STATIC_INIT
typedef INIT_ONCE SCallOnceGuard;
#else
    #define THREAD_LOCAL __thread
    #define INIT_CALL_ONCE_GUARD PTHREAD_ONCE_INIT
typedef pthread_once_t SCallOnceGuard;
#endif

#define INVALID_THREAD_ID 0

#if defined(_WIN32) || defined(XBOX)
    #define THREAD_LOCAL __declspec(thread)
    #define INIT_CALL_ONCE_GUARD INIT_ONCE_STATIC_INIT
typedef INIT_ONCE SCallOnceGuard;
#else
    #define THREAD_LOCAL __thread
    #define INIT_CALL_ONCE_GUARD PTHREAD_ONCE_INIT
typedef pthread_once_t SCallOnceGuard;
#endif

#define TIMEOUT_INFINITE UINT32_MAX

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*SCallOnceFn)(void);

typedef struct SMutex {
#if defined(_WIN32) || defined(XBOX)
    CRITICAL_SECTION mHandle;
#elif defined(NX64)
    MutexTypeNX mMutexPlatformNX;
    uint32_t mSpinCount;
#else
    pthread_mutex_t pHandle;
    uint32_t mSpinCount;
#endif
} SMutex;

typedef struct SConditionVariable {
#if defined(_WIN32) || defined(XBOX)
    void* pHandle;
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

/*
 * Brief:
 *   Guaranties that SCallOnceFn will be called once in a thread-safe way.
 * Notes:
 *   SCallOnceGuard has to be a pointer to a global variable initialized with INIT_CALL_ONCE_GUARD
 */
void skr_call_once(SCallOnceGuard* pGuard, SCallOnceFn pFn);

/// Operating system mutual exclusion primitive.

bool skr_init_mutex(SMutex* pMutex);
void skr_destroy_mutex(SMutex* pMutex);

void skr_acquire_mutex(SMutex* pMutex);
bool skr_try_acquire_mutex(SMutex* pMutex);
void skr_release_mutex(SMutex* pMutex);

bool skr_init_condition_var(SConditionVariable* cv);
void skr_destroy_condition_var(SConditionVariable* cv);

void skr_wait_condition_vars(SConditionVariable* cv, const SMutex* pMutex, uint32_t timeout);
void skr_wake_all_condition_vars(SConditionVariable* cv);
void skr_wake_condition_var(SConditionVariable* cv);

///
void skr_init_thread(SThreadDesc* pItem, SThreadHandle* pHandle);
void skr_set_thread_priority(SThreadHandle, SThreadPriority);
void skr_destroy_thread(SThreadHandle handle);
void skr_join_thread(SThreadHandle handle);

SThreadID skr_current_thread_id(void);
void skr_thread_sleep(unsigned mMilliSecs);
unsigned int skr_cpu_cores_count(void);

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
