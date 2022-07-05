#pragma once
#include "platform/configure.h"

#define SKR_WAITER_MODE_WIN32 0
#define SKR_WAITER_MODE_CONDVAR 1

#if defined(_WIN32) && _WIN32_WINNT >= _WIN32_WINNT_VISTA
    #define SKR_WAITER_MODE SKR_WAITER_MODE_WIN32
// TODO: MODE_FUTEX
// TODO: MODE_SEMAPHORE
#else
    #define SKR_WAITER_MODE SKR_WAITER_MODE_CONDVAR
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SThreadWaiter {
#if SKR_WAITER_MODE == SKR_WAITER_MODE_WIN32
    SKR_ALIGNAS(sizeof(void*))
    uint8_t mu_storage_[sizeof(void*)];
    SKR_ALIGNAS(sizeof(void*))
    uint8_t cv_storage_[sizeof(void*)];
    int waiter_count_;
    int wakeup_count_;
#elif SKR_WAITER_MODE == SKR_WAITER_MODE_CONDVAR
#endif
} SThreadWaiter;

#ifdef __cplusplus
}
#endif
