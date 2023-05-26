#pragma once
#include "async/result.hpp"
#include "platform/thread.h"
#include "platform/atomic.h"

namespace skr
{
using CondLockResult = AsyncResult;

struct CondLockDesc
{
    int32_t __nothing__;
};

struct RUNTIME_STATIC_API CondLock
{
    CondLock() SKR_NOEXCEPT;
    virtual ~CondLock() SKR_NOEXCEPT;

    // @retval ASYNC_RESULT_OK if success
	CondLockResult initialize(const char8_t *name, const CondLockDesc& pDesc = {}) SKR_NOEXCEPT;
    
    // @retval ASYNC_RESULT_OK if success
    CondLockResult finalize() SKR_NOEXCEPT;

    // wake up all threads waiting on condition variable.
    // @retval ASYNC_RESULT_OK if success
    CondLockResult broadcast() SKR_NOEXCEPT;

    // wake up a thread waiting on condition variable.
    // @retval ASYNC_RESULT_OK if success
    CondLockResult signal() SKR_NOEXCEPT;

    // wait by condition variable
    // @retval ASYNC_RESULT_OK if success
    CondLockResult wait(uint32_t milliseconds = UINT32_MAX) SKR_NOEXCEPT;

    // lock a mutex to be paired with a condition variable
    // @retval ASYNC_RESULT_OK if success
    CondLockResult lock() SKR_NOEXCEPT;

    // unlock a mutex to be paired with a condition variable
    // @retval ASYNC_RESULT_OK if success
    CondLockResult unlock() SKR_NOEXCEPT;

private:
    SConditionVariable cond;
    SMutex mutex;
};

}