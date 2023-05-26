#include "async/condlock.hpp"

namespace skr
{
CondLock::CondLock() SKR_NOEXCEPT
{
}

CondLock::~CondLock() SKR_NOEXCEPT
{
    finalize();
}

CondLockResult CondLock::initialize(const char8_t* name, const CondLockDesc& pOption) SKR_NOEXCEPT
{
    bool ok = skr_init_condition_var(&cond);
    if (ok)
    {
        ok = skr_init_mutex(&mutex);
        if (ok)
            return ASYNC_RESULT_OK;
        return ASYNC_RESULT_ERROR_COND_MX_CREATE_FAILED;
    }
    return ASYNC_RESULT_ERROR_COND_CREATE_FAILED;
}

CondLockResult CondLock::finalize() SKR_NOEXCEPT
{
    skr_destroy_condition_var(&cond);
    skr_destroy_mutex(&mutex);
    return ASYNC_RESULT_OK;
}

CondLockResult CondLock::broadcast() SKR_NOEXCEPT
{
    skr_wake_all_condition_vars(&cond);
    return ASYNC_RESULT_OK;
}

CondLockResult CondLock::signal() SKR_NOEXCEPT
{
    skr_wake_condition_var(&cond);
    return ASYNC_RESULT_OK;
}

CondLockResult CondLock::wait(uint32_t milliseconds) SKR_NOEXCEPT
{
    ThreadResult R = THREAD_RESULT_FAILED;
    if (milliseconds) {
        R = skr_wait_condition_vars(&cond, &mutex, milliseconds);
        if (R == THREAD_RESULT_TIMEOUT)
            return ASYNC_RESULT_ERROR_TIMEOUT;
    }
    else {
        R = skr_wait_condition_vars(&cond, &mutex, UINT32_MAX);
    }
    if (R == THREAD_RESULT_OK)
        return ASYNC_RESULT_OK;
    return ASYNC_RESULT_ERROR_UNKNOWN;
}

CondLockResult CondLock::lock() SKR_NOEXCEPT
{
    skr_mutex_acquire(&mutex);
    return ASYNC_RESULT_OK;
}

CondLockResult CondLock::unlock() SKR_NOEXCEPT
{
    skr_mutex_release(&mutex);
    return ASYNC_RESULT_OK;
}

}