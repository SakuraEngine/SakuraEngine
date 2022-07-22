#pragma once
#include "platform/memory.h"
#include "platform/thread.h"
#include "utils/log.h"
#include "utils/io.h"
#include "utils/io.hpp"
#include "utils/concurrent_queue.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

// RAM Service
namespace skr
{
namespace io
{
typedef enum _SkrIOThreadStatus
{
    _SKR_IO_THREAD_STATUS_RUNNING = 0,
    _SKR_IO_THREAD_STATUS_QUIT = 1,
    _SKR_IO_THREAD_STATUS_SUSPEND = 2
} _SkrIOThreadStatus;

class AsyncServiceBase
{
public:
    AsyncServiceBase(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : isLockless(lockless),
          _sleepTime(sleep_time)

    {
        skr_init_mutex(&taskMutex);
        skr_init_mutex(&sleepMutex);
        skr_init_condition_var(&sleepCv);
    }

    virtual ~AsyncServiceBase()
    {
        skr_destroy_mutex(&taskMutex);
        skr_destroy_mutex(&sleepMutex);
    }
    // task 
    void optionalLockTasks() SKR_NOEXCEPT
    {
        if (!isLockless)
            skr_acquire_mutex(&taskMutex);
    }

    void optionalUnlockTasks() SKR_NOEXCEPT
    {
        if (!isLockless)
            skr_release_mutex(&taskMutex);
    }

    // running status
    void setRunningStatus(SkrAsyncIOServiceStatus status)
    {
        skr_atomic32_store_relaxed(&_running_status, status);
    }
    SkrAsyncIOServiceStatus getRunningStatus() const
    {
        return (SkrAsyncIOServiceStatus)skr_atomic32_load_acquire(&_running_status);
    }

    // thread status
    void setThreadStatus(_SkrIOThreadStatus status)
    {   
        skr_atomic32_store_relaxed(&_thread_status, status);
    }

    _SkrIOThreadStatus getThreadStatus() const
    {
        return (_SkrIOThreadStatus)skr_atomic32_load_acquire(&_thread_status);
    }

    // sleep
    void serviceSleep()
    {
        auto service = this;
        service->optionalUnlockTasks();
        const auto sleepTimeVal = skr_atomic32_load_acquire(&service->_sleepTime);
        {
            service->setRunningStatus(SKR_IO_SERVICE_STATUS_SLEEPING);
            if (service->sleepMode == SKR_IO_SERVICE_SLEEP_MODE_SLEEP &&
                sleepTimeVal != 0)
            {
                auto sleepTime = eastl::min(sleepTimeVal, 100u);
                sleepTime = eastl::max(sleepTimeVal, 1u);
                TracyCZone(sleepZone, 1);
                TracyCZoneName(sleepZone, "ioServiceSleep(Sleep)", strlen("ioServiceSleep(Sleep)"));
                skr_thread_sleep(sleepTime);
                TracyCZoneEnd(sleepZone);
            }
            if (service->sleepMode == SKR_IO_SERVICE_SLEEP_MODE_COND_VAR)
            {
                // use condition variable to sleep
                TracyCZone(sleepZone, 1);
                TracyCZoneName(sleepZone, "ioServiceSleep(Cond)", strlen("ioServiceSleep(Cond)"));
                SMutexLock sleepLock(service->sleepMutex);
                skr_wait_condition_vars(&service->sleepCv, &service->sleepMutex, sleepTimeVal);
                TracyCZoneEnd(sleepZone);
            }
        }
    }

    void create_(SkrAsyncIOServiceSleepMode sleep_mode)
    {
        auto service = this;
        service->setRunningStatus(SKR_IO_SERVICE_STATUS_RUNNING);
        service->setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
        sleep_mode = sleepMode;
    }

    void destroy_()
    {
        auto service = this;
        service->setThreadStatus(_SKR_IO_THREAD_STATUS_QUIT);
    }

    void drain_()
    {
        // wait for sleep
        for (; getRunningStatus() != SKR_IO_SERVICE_STATUS_SLEEPING;)
        {
            //...
        }
    }

    void set_sleep_time_(uint32_t ms)
    {
        skr_atomic32_store_relaxed(&_sleepTime, ms);
    }

    // running modes
    const bool isLockless = false;
    const bool criticalTaskCount = false;
    // task containers
    SMutex taskMutex;
    // for condvar mode sleep
    SMutex sleepMutex;
    SConditionVariable sleepCv;
    SAtomic32 _sleepTime = 30 /*ms*/;
    // service settings & states
    SAtomic32 _running_status /*SkrAsyncIOServiceStatus*/;
    SAtomic32 _thread_status = _SKR_IO_THREAD_STATUS_RUNNING /*IOThreadStatus*/;
    // can be simply exchanged by atomic vars to support runtime mode modify
    SkrIOServiceSortMethod sortMethod;
    SkrAsyncIOServiceSleepMode sleepMode;
};
}
}