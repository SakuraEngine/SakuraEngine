#pragma once
#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/thread.h"
#include "misc/log.h"
#include "io/io.h"
#include "misc/defer.hpp"
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/deque.h>
#include <EASTL/optional.h>
#include <EASTL/sort.h>
#include "containers/concurrent_queue.h"

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

typedef enum SkrAsyncServiceSortMethod
{
    SKR_ASYNC_SERVICE_SORT_METHOD_NEVER = 0,
    SKR_ASYNC_SERVICE_SORT_METHOD_STABLE = 1,
    SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL = 2,
    SKR_ASYNC_SERVICE_SORT_METHOD_COUNT,
    SKR_ASYNC_SERVICE_SORT_METHOD_MAX_ENUM = INT32_MAX
} SkrAsyncServiceSortMethod;

class AsyncServiceBase
{
public:
    AsyncServiceBase(uint32_t sleep_time) SKR_NOEXCEPT
        : _sleepTime(sleep_time)

    {
        skr_init_mutex_recursive(&sleepMutex);
        skr_init_condition_var(&sleepCv);
    }

    virtual ~AsyncServiceBase()
    {
        skr_destroy_mutex(&sleepMutex);
    }

    // running status
    void setServiceStatus(SkrAsyncServiceStatus status)
    {
        skr_atomicu32_store_relaxed(&_running_status, status);
    }
    SkrAsyncServiceStatus getServiceStatus() const
    {
        return (SkrAsyncServiceStatus)skr_atomicu32_load_acquire(&_running_status);
    }

    // util methods

    virtual void create_() SKR_NOEXCEPT
    {
        auto service = this;
        service->setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
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
        const auto timeout = 10u;
        uint32_t ms = 0;
        for (; getServiceStatus() != SKR_ASYNC_SERVICE_STATUS_SLEEPING;)
        {
            skr_thread_sleep(1);
            ms++;
            if (ms > timeout * 1000u)
            {
                SKR_LOG_ERROR("drain timeout, force quit");
                break;
            }
        }
    }

    void set_sleep_time_(uint32_t ms) SKR_NOEXCEPT
    {
        skr_atomicu32_store_relaxed(&_sleepTime, ms);
    }

    // running modes
    const bool isLockless = false;
    const bool criticalTaskCount = false;
    // for condvar mode sleep
    SMutex sleepMutex;
    SConditionVariable sleepCv;
    SAtomicU32 _sleepTime = 30 /*ms*/;
    // service settings & states
    SAtomicU32 _running_status /*SkrAsyncServiceStatus*/;
    // can be simply exchanged by atomic vars to support runtime mode modify
    SkrAsyncServiceSortMethod sortMethod = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
};

RUNTIME_API extern const char* kIOTaskQueueName;
struct TaskBase
{
    SkrAsyncServicePriority priority;
    float sub_priority;
    skr_io_callback_t callbacks[SKR_IO_STAGE_COUNT];
    void* callback_datas[SKR_IO_STAGE_COUNT];
    skr_io_future_t* request;

    bool operator<(const TaskBase& rhs) const
    {
        if (rhs.priority != priority)
            return priority > rhs.priority;
        return sub_priority > rhs.sub_priority;
    }

    ESkrIOStage getTaskStatus() const
    {
        return (ESkrIOStage)skr_atomicu32_load_acquire(&request->status);
    }
    
    void setTaskStatus(ESkrIOStage value)
    {
        if (callbacks[value] != nullptr)
            callbacks[value](request, nullptr, callback_datas[value]);
        skr_atomicu32_store_relaxed(&request->status, value);
    }
};

template<typename Task>
struct TaskContainer
{
    static_assert(std::is_base_of_v<TaskBase, Task>, "Task must be derived from TaskBase");

    TaskContainer(bool is_lockless) SKR_NOEXCEPT
        : isLockless(is_lockless)
    {
        skr_init_mutex_recursive(&taskMutex);
    }

    ~TaskContainer()
    {
        skr_destroy_mutex(&taskMutex);
    }

    // task 
    void optionalLockTasks() SKR_NOEXCEPT
    {
        if (!isLockless)
            skr_mutex_acquire(&taskMutex);
    }

    void optionalUnlockTasks() SKR_NOEXCEPT
    {
        if (!isLockless)
            skr_mutex_release(&taskMutex);
    }

    void update_(AsyncServiceBase* service) SKR_NOEXCEPT
    {
        // 0.if lockless dequeue_bulk the requests to vector
        optionalLockTasks();
        SKR_DEFER({ optionalUnlockTasks(); });
        if (isLockless)
        {
            ZoneScopedN("ioServiceDequeueRequests");
            Task tsk;
            while (task_requests.try_dequeue(tsk))
            {
                tasks.emplace_back(tsk);
            }
        }
        // 1.defer cancel tasks
        if (tasks.size())
        {
            ZoneScopedN("ioServiceCancels");
            tasks.erase(
                eastl::remove_if(tasks.begin(), tasks.end(),
                    [](Task& t) {
                        bool cancelled = skr_atomicu32_load_relaxed(&t.request->request_cancel);
                        cancelled &= t.request->is_cancelled() || t.request->is_enqueued();
                        if (cancelled)
                            t.setTaskStatus(SKR_IO_STAGE_CANCELLED);
                        return cancelled;
                    }), tasks.end());
        }
        // 2.try fetch a new task
        {
            // empty sleep
            if (!tasks.size())
            {
                service->sleep_();
            }
            else // do sort
            {
                ZoneScopedN("ioServiceSort");
                service->setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
                switch (service->sortMethod)
                {
                    case SKR_ASYNC_SERVICE_SORT_METHOD_STABLE:
                        eastl::stable_sort(tasks.begin(), tasks.end());
                        break;
                    case SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL:
                        eastl::partial_sort(tasks.begin(),
                        tasks.begin() + tasks.size() / 2,
                        tasks.end());
                        break;
                    case SKR_ASYNC_SERVICE_SORT_METHOD_NEVER:
                    default:
                        break;
                }
            }
        }
    }

    // get front
    eastl::optional<Task> peek_() SKR_NOEXCEPT
    {
        optionalLockTasks();
        SKR_DEFER({ optionalUnlockTasks(); });
        if (tasks.size() != 0)
        {
            auto res = tasks.front();
            tasks.pop_front();
            return res;
        }
        return eastl::nullopt;
    }

    void visit_(eastl::function<void(Task&)> kernel) SKR_NOEXCEPT
    {
        optionalLockTasks();
        SKR_DEFER({ optionalUnlockTasks(); });
        eastl::for_each(tasks.begin(), tasks.end(), kernel);
        tasks.erase(
            eastl::remove_if(tasks.begin(), tasks.end(),
            [&](Task& t) {
                const auto status = t.getTaskStatus();
                return status == SKR_IO_STAGE_LOADED || status == SKR_IO_STAGE_CANCELLED;
            }),
        tasks.end());
    }

    void enqueue_(Task& back, bool criticalTaskCount, const char* name)
    {
        if (!isLockless)
        {
            optionalLockTasks();
            SKR_DEFER({ optionalUnlockTasks(); });
            TracyCZone(requestZone, 1);
            TracyCZoneName(requestZone, "ioRequest(Locked)", strlen("ioRequest(Locked)"));
            if (tasks.size() >= SKR_IO_SERVICE_MAX_TASK_COUNT)
            {
                if (criticalTaskCount)
                {
                    SKR_LOG_WARN(
                        "ioService %s enqueued too many tasks(over %d)!",
                        name, SKR_IO_SERVICE_MAX_TASK_COUNT);
                    return;
                }
            }
            back.setTaskStatus(SKR_IO_STAGE_ENQUEUED);
            tasks.emplace_back(back);
            skr_atomicu32_store_release(&back.request->request_cancel, 0);
            TracyCZoneEnd(requestZone);
        }
        else
        {
            TracyCZone(requestZone, 1);
            TracyCZoneName(requestZone, "ioRequest(Lockless)", strlen("ioRequest(Lockless)"));
            back.setTaskStatus(SKR_IO_STAGE_ENQUEUED);
            {
                ZoneScopedN("EnqueueLockless");
                task_requests.enqueue(back);
            }
            skr_atomicu32_store_release(&back.request->request_cancel, 0);
            TracyCZoneEnd(requestZone);
        }
    }

    bool try_cancel_(skr_io_future_t* request) SKR_NOEXCEPT
    {
        if (request->is_enqueued() && !isLockless)
        {
            optionalLockTasks();
            SKR_DEFER({ optionalUnlockTasks(); });
            bool cancelled = false;
            tasks.erase(
            eastl::remove_if(tasks.begin(), tasks.end(),
            [&](Task& t) {
                const bool stateCancellable = request->is_enqueued() || request->is_cancelled();
                cancelled = (t.request == request || request == nullptr) && stateCancellable;
                if (cancelled)
                    t.setTaskStatus(SKR_IO_STAGE_CANCELLED);
                return cancelled;
            }),
            tasks.end());
            return cancelled;
        }
        return false;
    }

    void defer_cancel_(skr_io_future_t* request) 
    {
        skr_atomicu32_store_release(&request->request_cancel, 1);
    }

    const bool isLockless = false;
    SMutex taskMutex;
    struct RUNTIME_API IOTaskConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
    {
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static inline void* malloc(size_t size) { return sakura_mallocN(size, kIOTaskQueueName); }
        static inline void free(void* ptr) { return sakura_freeN(ptr, kIOTaskQueueName); }
    };
    skr::ConcurrentQueue<Task, IOTaskConcurrentQueueTraits> task_requests;
    eastl::deque<Task> tasks;
};


class AsyncThreadedService : public AsyncServiceBase
{
public:
    AsyncThreadedService(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : AsyncServiceBase(sleep_time)
    {

    }

    void create_() SKR_NOEXCEPT override
    {
        auto service = this;
        AsyncServiceBase::create_();
        service->setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
    }

    virtual void sleep_() SKR_NOEXCEPT override
    {
        auto service = this;
        AsyncServiceBase::sleep_();
        const auto sleepTimeVal = skr_atomicu32_load_acquire(&service->_sleepTime);
        {
            service->setServiceStatus(SKR_ASYNC_SERVICE_STATUS_SLEEPING);

            // use condition variable to sleep
            TracyCZoneC(sleepZone, tracy::Color::Gray43, 1);
            TracyCZoneName(sleepZone, "ioServiceSleep(Cond)", strlen("ioServiceSleep(Cond)"));
            {
                SMutexLock sleepLock(service->sleepMutex);
                skr_wait_condition_vars(&service->sleepCv, &service->sleepMutex, sleepTimeVal);
                skr_init_condition_var(&service->sleepCv);
            }
            TracyCZoneEnd(sleepZone);

            auto serviceQuiting = getServiceStatus();
            if (serviceQuiting == SKR_ASYNC_SERVICE_STATUS_QUITING)
            {
                service->setThreadStatus(_SKR_IO_THREAD_STATUS_QUIT);
                return;
            }
        }
    }

    virtual void request_() SKR_NOEXCEPT override
    {
        // unlock cv
        [[maybe_unused]] const auto sleepTimeVal = skr_atomicu32_load_acquire(&_sleepTime);
            
        SMutexLock sleepLock(sleepMutex);
        skr_wake_all_condition_vars(&sleepCv);
    }

    virtual void destroy_() SKR_NOEXCEPT override
    {
        auto service = this;
        AsyncServiceBase::destroy_();
        service->setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
        {
            while (service->getThreadStatus() != _SKR_IO_THREAD_STATUS_QUIT)
            {
                SMutexLock sleepLock(service->sleepMutex);
                skr_wake_all_condition_vars(&service->sleepCv);
            }
        }
        skr_join_thread(service->serviceThread);
        skr_destroy_thread(service->serviceThread);
    }
    
    virtual void run_() SKR_NOEXCEPT override
    {
        if (getThreadStatus() != _SKR_IO_THREAD_STATUS_SUSPEND)
            return;
        setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
    }

    virtual void stop_(bool wait_drain) SKR_NOEXCEPT override
    {
        if (getThreadStatus() != _SKR_IO_THREAD_STATUS_RUNNING) return;
        if (wait_drain) drain_(); // sleep -> hung
        // else running -> hung
        setThreadStatus(_SKR_IO_THREAD_STATUS_SUSPEND);
    }

    // thread status
    void setThreadStatus(_SkrIOThreadStatus status)
    {   
        skr_atomicu32_store_relaxed(&_thread_status, status);
    }

    _SkrIOThreadStatus getThreadStatus() const
    {
        return (_SkrIOThreadStatus)skr_atomicu32_load_acquire(&_thread_status);
    }

    // thread items
    SThreadDesc threadItem = {};
    SThreadHandle serviceThread;
    SAtomicU32 _thread_status = _SKR_IO_THREAD_STATUS_RUNNING /*IOThreadStatus*/;
};

}
}