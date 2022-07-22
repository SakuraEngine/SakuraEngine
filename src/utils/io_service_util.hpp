#pragma once
#include "platform/memory.h"
#include "platform/thread.h"
#include "utils/log.h"
#include "utils/io.h"
#include "utils/io.hpp"
#include "utils/concurrent_queue.h"
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/deque.h>
#include <EASTL/optional.h>
#include <EASTL/sort.h>

#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

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
    AsyncServiceBase(uint32_t sleep_time) SKR_NOEXCEPT
        : _sleepTime(sleep_time)

    {
        skr_init_mutex(&sleepMutex);
        skr_init_condition_var(&sleepCv);
    }

    virtual ~AsyncServiceBase()
    {
        skr_destroy_mutex(&sleepMutex);
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

    // util methods

    virtual void create_(SkrAsyncIOServiceSleepMode sleep_mode) SKR_NOEXCEPT
    {
        auto service = this;
        service->setRunningStatus(SKR_IO_SERVICE_STATUS_RUNNING);
        sleep_mode = sleepMode;
    }

    virtual void sleep_() SKR_NOEXCEPT
    {
        
    }

    virtual void request_() SKR_NOEXCEPT
    {
        
    }

    virtual void destroy_() SKR_NOEXCEPT
    {
        
    }

    virtual void run_() SKR_NOEXCEPT
    {

    }

    virtual void stop_(bool wait_drain) SKR_NOEXCEPT
    {

    }

    virtual void drain_() SKR_NOEXCEPT
    {
        // wait for sleep
        for (; getRunningStatus() != SKR_IO_SERVICE_STATUS_SLEEPING;)
        {
            //...
        }
    }

    void set_sleep_time_(uint32_t ms) SKR_NOEXCEPT
    {
        skr_atomic32_store_relaxed(&_sleepTime, ms);
    }

    // running modes
    const bool isLockless = false;
    const bool criticalTaskCount = false;
    // for condvar mode sleep
    SMutex sleepMutex;
    SConditionVariable sleepCv;
    SAtomic32 _sleepTime = 30 /*ms*/;
    // service settings & states
    SAtomic32 _running_status /*SkrAsyncIOServiceStatus*/;
    // can be simply exchanged by atomic vars to support runtime mode modify
    SkrIOServiceSortMethod sortMethod;
    SkrAsyncIOServiceSleepMode sleepMode;
};

struct TaskBase
{
    SkrIOServicePriority priority;
    float sub_priority;
    skr_async_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
    skr_async_io_request_t* request;

    bool operator<(const TaskBase& rhs) const
    {
        if (rhs.priority != priority)
            return priority > rhs.priority;
        return sub_priority > rhs.sub_priority;
    }

    SkrAsyncIOStatus getTaskStatus() const
    {
        return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&request->status);
    }
    
    void setTaskStatus(SkrAsyncIOStatus value)
    {
        skr_atomic32_store_relaxed(&request->status, value);
        if (callbacks[value] != nullptr)
            callbacks[value](request, callback_datas[value]);
    }
};

template<typename Task>
struct TaskContainer
{
    static_assert(std::is_base_of_v<TaskBase, Task>, "Task must be derived from TaskBase");

    TaskContainer(bool is_lockless) SKR_NOEXCEPT
        : isLockless(is_lockless)
    {
        skr_init_mutex(&taskMutex);
    }

    ~TaskContainer()
    {
        skr_destroy_mutex(&taskMutex);
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

    // get front
    eastl::optional<Task> peek_(AsyncServiceBase* service) SKR_NOEXCEPT
    {
        // 0.if lockless dequeue_bulk the requests to vector
        optionalLockTasks();
        if (isLockless)
        {
            Task tsk;
            while (task_requests.try_dequeue(tsk))
            {
                TracyCZone(dequeueZone, 1);
                TracyCZoneName(dequeueZone, "ioServiceDequeueRequests", strlen("ioServiceDequeueRequests"));
                tasks.emplace_back(tsk);
                TracyCZoneEnd(dequeueZone);
            }
        }
        // 1.defer cancel tasks
        if (tasks.size())
        {
            TracyCZone(cancelZone, 1);
            TracyCZoneName(cancelZone, "ioServiceCancels", strlen("ioServiceCancels"));
            tasks.erase(
                eastl::remove_if(tasks.begin(), tasks.end(),
                    [](Task& t) {
                        bool cancelled = skr_atomic32_load_relaxed(&t.request->request_cancel);
                        cancelled &= t.request->is_cancelled() || t.request->is_enqueued();
                        if (cancelled)
                            t.setTaskStatus(SKR_ASYNC_IO_STATUS_CANCELLED);
                        return cancelled;
                    }), tasks.end());
            TracyCZoneEnd(cancelZone);
        }
        // 2.try fetch a new task
        {
            // empty sleep
            if (!tasks.size())
            {
                optionalUnlockTasks();
                service->sleep_();
                return eastl::nullopt;
            }
            else // do sort
            {
                TracyCZone(sortZone, 1);
                TracyCZoneName(sortZone, "ioServiceSort", strlen("ioServiceSort"));
                service->setRunningStatus(SKR_IO_SERVICE_STATUS_RUNNING);
                switch (service->sortMethod)
                {
                    case SKR_IO_SERVICE_SORT_METHOD_STABLE:
                        eastl::stable_sort(tasks.begin(), tasks.end());
                        break;
                    case SKR_IO_SERVICE_SORT_METHOD_PARTIAL:
                        eastl::partial_sort(tasks.begin(),
                        tasks.begin() + tasks.size() / 2,
                        tasks.end());
                        break;
                    case SKR_IO_SERVICE_SORT_METHOD_NEVER:
                    default:
                        break;
                }
                TracyCZoneEnd(sortZone);
            }
        }
        auto res = tasks.front();
        tasks.pop_front();
        optionalUnlockTasks();
        return res;
    }

    void enqueue_(Task& back, bool criticalTaskCount, const char* name)
    {
        if (!isLockless)
        {
            optionalLockTasks();
            TracyCZone(requestZone, 1);
            TracyCZoneName(requestZone, "ioRequest(Locked)", strlen("ioRequest(Locked)"));
            if (tasks.size() >= SKR_IO_SERVICE_MAX_TASK_COUNT)
            {
                if (criticalTaskCount)
                {
                    SKR_LOG_WARN(
                        "ioService %s enqueued too many tasks(over %d)!",
                        name, SKR_IO_SERVICE_MAX_TASK_COUNT);
                        optionalUnlockTasks();
                    return;
                }
            }
            back.setTaskStatus(SKR_ASYNC_IO_STATUS_ENQUEUED);
            tasks.emplace_back(back);
            skr_atomic32_store_relaxed(&back.request->request_cancel, 0);
            TracyCZoneEnd(requestZone);
            optionalUnlockTasks();
        }
        else
        {
            TracyCZone(requestZone, 1);
            TracyCZoneName(requestZone, "ioRequest(Lockless)", strlen("ioRequest(Lockless)"));
            back.setTaskStatus(SKR_ASYNC_IO_STATUS_ENQUEUED);
            task_requests.enqueue(back);
            skr_atomic32_store_relaxed(&back.request->request_cancel, 0);
            TracyCZoneEnd(requestZone);
        }
    }

    bool try_cancel_(skr_async_io_request_t* request) SKR_NOEXCEPT
    {
        if (request->is_enqueued() && !isLockless)
        {
            optionalLockTasks();
            bool cancelled = false;
            tasks.erase(
            eastl::remove_if(tasks.begin(), tasks.end(),
            [&](Task& t) {
                const bool stateCancellable = request->is_enqueued() || request->is_cancelled();
                cancelled = (t.request == request || request == nullptr) && stateCancellable;
                if (cancelled)
                    t.setTaskStatus(SKR_ASYNC_IO_STATUS_CANCELLED);
                return cancelled;
            }),
            tasks.end());
            optionalUnlockTasks();
            return cancelled;
        }
        return false;
    }

    void defer_cancel_(skr_async_io_request_t* request) 
    {
        skr_atomic32_store_relaxed(&request->request_cancel, 1);
    }

    const bool isLockless = false;
    SMutex taskMutex;
    moodycamel::ConcurrentQueue<Task> task_requests;
    eastl::deque<Task> tasks;
};


class AsyncThreadedService : public AsyncServiceBase
{
public:
    AsyncThreadedService(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : AsyncServiceBase(sleep_time)
    {

    }

    void create_(SkrAsyncIOServiceSleepMode sleep_mode) SKR_NOEXCEPT override
    {
        auto service = this;
        AsyncServiceBase::create_(sleep_mode);
        service->setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
    }

    void sleep_() SKR_NOEXCEPT override
    {
        auto service = this;
        AsyncServiceBase::sleep_();
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

    void request_() SKR_NOEXCEPT override
    {
        // unlock cv
        const auto sleepTimeVal = skr_atomic32_load_acquire(&_sleepTime);
        if (sleepTimeVal == SKR_IO_SERVICE_SLEEP_TIME_MAX && sleepMode == SKR_IO_SERVICE_SLEEP_MODE_COND_VAR)
        {
            skr_wake_condition_var(&sleepCv);
        }
    }

    void destroy_() SKR_NOEXCEPT override
    {
        auto service = this;
        AsyncServiceBase::destroy_();
        service->setThreadStatus(_SKR_IO_THREAD_STATUS_QUIT);
        skr_join_thread(service->serviceThread);
        skr_destroy_thread(service->serviceThread);
    }
    
    void run_() SKR_NOEXCEPT override
    {
        if (getThreadStatus() != _SKR_IO_THREAD_STATUS_SUSPEND)
            return;
        setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
    }

    void stop_(bool wait_drain) SKR_NOEXCEPT override
    {
        if (getThreadStatus() != _SKR_IO_THREAD_STATUS_RUNNING) return;
        if (wait_drain) drain_(); // sleep -> hung
        // else running -> hung
        setThreadStatus(_SKR_IO_THREAD_STATUS_SUSPEND);
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

    // thread items
    SThreadDesc threadItem = {};
    SThreadHandle serviceThread;
    SAtomic32 _thread_status = _SKR_IO_THREAD_STATUS_RUNNING /*IOThreadStatus*/;
};

}
}