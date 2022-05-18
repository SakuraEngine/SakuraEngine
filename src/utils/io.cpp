#include "utils/io.h"
#include "utils/io.hpp"
#include "utils/log.h"
#include "platform/memory.h"
#include "platform/thread.h"
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/deque.h>
#include <EASTL/sort.h>
#include "utils/concurrent_queue.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

bool skr_async_io_request_t::is_ready() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_OK;
}
bool skr_async_io_request_t::is_enqueued() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_ENQUEUED;
}
bool skr_async_io_request_t::is_cancelled() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_CANCELLED;
}
bool skr_async_io_request_t::is_ram_loading() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_RAM_LOADING;
}
bool skr_async_io_request_t::is_vram_loading() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_VRAM_LOADING;
}

SkrAsyncIOStatus skr_async_io_request_t::get_status() const SKR_NOEXCEPT
{
    return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&status);
}

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

class RAMServiceImpl final : public RAMService
{
public:
    struct Task {
        skr_vfs_t* vfs;
        std::string path;
        uint64_t offset;
        skr_async_io_request_t* request;
        SkrIOServicePriority priority;
        float sub_priority;
        skr_async_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
        void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
        bool operator<(const Task& rhs) const
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
                callbacks[value](callback_datas[value]);
        }
    };
    ~RAMServiceImpl() SKR_NOEXCEPT = default;
    RAMServiceImpl(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        :  isLockless(lockless), _sleepTime(sleep_time)
         
    {
    }
    void request(skr_vfs_t*, const skr_ram_io_t* info, skr_async_io_request_t* async_request) SKR_NOEXCEPT final;
    bool try_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT final;
    void defer_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT final;
    void drain() SKR_NOEXCEPT final;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT final;
    void stop(bool wait_drain = false) SKR_NOEXCEPT final;
    void run() SKR_NOEXCEPT final;

    SkrAsyncIOServiceStatus get_service_status() const SKR_NOEXCEPT final
    {
        return getRunningStatus();
    }

    void setRunningStatus(SkrAsyncIOServiceStatus status)
    {
        skr_atomic32_store_relaxed(&_running_status, status);
    }
    SkrAsyncIOServiceStatus getRunningStatus() const
    {
        return (SkrAsyncIOServiceStatus)skr_atomic32_load_acquire(&_running_status);
    }

    void setThreadStatus(_SkrIOThreadStatus status)
    {
        skr_atomic32_store_relaxed(&_thread_status, status);
    }
    _SkrIOThreadStatus getThreadStatus() const
    {
        return (_SkrIOThreadStatus)skr_atomic32_load_acquire(&_thread_status);
    }

    void optionalLock() SKR_NOEXCEPT
    {
        if (!isLockless)
            skr_acquire_mutex(&taskMutex);
    }
    void optionalUnlock() SKR_NOEXCEPT
    {
        if (!isLockless)
            skr_release_mutex(&taskMutex);
    }
    bool doCancel(skr_async_io_request_t* request) SKR_NOEXCEPT;

    const bool isLockless = false;
    const bool criticalTaskCount = false;
    const eastl::string name;
    // thread items
    SThreadDesc threadItem = {};
    SThreadHandle serviceThread;
    // task containers
    SMutex taskMutex;
    moodycamel::ConcurrentQueue<Task> task_requests;
    eastl::deque<Task> tasks;
    // for condvar mode sleep
    SMutex sleepMutex;
    SConditionVariable sleepCv;
    // current task
    RAMServiceImpl::Task current;
    // service settings & states
    SAtomic32 _running_status /*SkrAsyncIOServiceStatus*/;
    SAtomic32 _thread_status = _SKR_IO_THREAD_STATUS_RUNNING /*IOThreadStatus*/;
    SAtomic32 _sleepTime = 30 /*ms*/;
    // running modes
    // can be simply exchanged by atomic vars to support runtime mode modify
    SkrIOServiceSortMethod sortMethod;
    SkrAsyncIOServiceSleepMode sleepMode;
};

void ioThreadTask_execute(skr::io::RAMServiceImpl* service)
{
    // 0.if lockless dequeue_bulk the requests to vector
    service->optionalLock();
    if (service->isLockless)
    {
        RAMServiceImpl::Task tsk;
        while (service->task_requests.try_dequeue(tsk))
        {
            TracyCZone(dequeueZone, 1);
            TracyCZoneName(dequeueZone, "ioServiceDequeueRequests", strlen("ioServiceDequeueRequests"));
            service->tasks.emplace_back(tsk);
            TracyCZoneEnd(dequeueZone);
        }
    }
    // 1.defer cancel tasks
    if(service->tasks.size())
    {
        TracyCZone(cancelZone, 1);
        TracyCZoneName(cancelZone, "ioServiceCancels", strlen("ioServiceCancels"));
        service->tasks.erase(
            eastl::remove_if(service->tasks.begin(), service->tasks.end(),
            [](RAMServiceImpl::Task& t) {
                bool cancelled = skr_atomic32_load_relaxed(&t.request->request_cancel);
                cancelled &= t.request->is_cancelled() || t.request->is_enqueued();
                if (cancelled)
                    t.setTaskStatus(SKR_ASYNC_IO_STATUS_CANCELLED);
                return cancelled;
            }),
        service->tasks.end());
        TracyCZoneEnd(cancelZone);
    }
    // 2.try fetch a new task
    {
        // empty sleep
        if (!service->tasks.size())
        {
            service->optionalUnlock();
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
            return;
        }
        else // do sort
        {
            TracyCZone(sortZone, 1);
            TracyCZoneName(sortZone, "ioServiceSort", strlen("ioServiceSort"));
            service->setRunningStatus(SKR_IO_SERVICE_STATUS_RUNNING);
            switch (service->sortMethod)
            {
                case SKR_IO_SERVICE_SORT_METHOD_STABLE:
                    eastl::stable_sort(service->tasks.begin(), service->tasks.end());
                    break;
                case SKR_IO_SERVICE_SORT_METHOD_PARTIAL:
                    eastl::partial_sort(service->tasks.begin(),
                    service->tasks.begin() + service->tasks.size() / 2,
                    service->tasks.end());
                    break;
                case SKR_IO_SERVICE_SORT_METHOD_NEVER:
                default:
                    break;
            }
            TracyCZoneEnd(sortZone);
        }
        // pop current task
        service->current = service->tasks.front();
        service->tasks.pop_front();
    }
    service->optionalUnlock();
    // 3.load file
    {
        TracyCZoneC(readZone, tracy::Color::LightYellow, 1);
        TracyCZoneName(readZone, "ioServiceReadFile", strlen("ioServiceReadFile"));
        service->current.setTaskStatus(SKR_ASYNC_IO_STATUS_RAM_LOADING);
        auto vf = skr_vfs_fopen(service->current.vfs, service->current.path.c_str(),
        ESkrFileMode::SKR_FM_READ, ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
        if (service->current.request->bytes == nullptr)
        {
            // allocate
            auto fsize = skr_vfs_fsize(vf);
            service->current.request->size = fsize;
            service->current.request->bytes = (char8_t*)sakura_malloc(fsize);
        }
        skr_vfs_fread(vf, service->current.request->bytes, service->current.offset, service->current.request->size);
        service->current.setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
        skr_vfs_fclose(vf);
        TracyCZoneEnd(readZone);
    }
}

void ioThreadTask(void* arg)
{
#ifdef TRACY_ENABLE
    static uint32_t taskIndex = 0;
    eastl::string name = "ioRAMServiceThread-";
    name.append(eastl::to_string(taskIndex++));
    tracy::SetThreadName(name.c_str());
#endif
    auto service = reinterpret_cast<skr::io::RAMServiceImpl*>(arg);
    for (; service->getThreadStatus() != _SKR_IO_THREAD_STATUS_QUIT;)
    {
        if (service->getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND)
        {
            ZoneScopedN("ioServiceSuspend");
            for (; service->getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND;)
            {
            }
        }
        ioThreadTask_execute(service);
    }
}

void skr::io::RAMServiceImpl::request(skr_vfs_t* vfs, const skr_ram_io_t* info, skr_async_io_request_t* async_request) SKR_NOEXCEPT
{
    // try push back new request
    if (!isLockless)
    {
        optionalLock();
        TracyCZone(requestZone, 1);
        TracyCZoneName(requestZone, "ioRequest(Locked)", strlen("ioRequest(Locked)"));
        if (tasks.size() >= SKR_IO_SERVICE_MAX_TASK_COUNT)
        {
            if (criticalTaskCount)
            {
                SKR_LOG_WARN(
                "ioRAMService %s enqueued too many tasks(over %d)!",
                name.c_str(), SKR_IO_SERVICE_MAX_TASK_COUNT);
                optionalUnlock();
                return;
            }
        }
        auto& back = tasks.push_back();
        back.vfs = vfs;
        back.path = std::string(info->path);
        back.offset = info->offset;
        back.request = async_request;
        back.request->bytes = (char8_t*)info->bytes;
        back.request->size = info->size;
        back.priority = info->priority;
        back.sub_priority = info->sub_priority;
        for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
        {
            back.callbacks[i] = info->callbacks[i];
            back.callback_datas[i] = info->callback_datas[i];
        }
        skr_atomic32_store_relaxed(&async_request->status, SKR_ASYNC_IO_STATUS_ENQUEUED);
        skr_atomic32_store_relaxed(&async_request->request_cancel, 0);
        TracyCZoneEnd(requestZone);
        optionalUnlock();
    }
    else
    {
        TracyCZone(requestZone, 1);
        TracyCZoneName(requestZone, "ioRequest(Lockless)", strlen("ioRequest(Lockless)"));
        Task back = {};
        back.vfs = vfs;
        back.path = std::string(info->path);
        back.offset = info->offset;
        back.request = async_request;
        back.request->bytes = (char8_t*)info->bytes;
        back.request->size = info->size;
        back.priority = info->priority;
        back.sub_priority = info->sub_priority;
        for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
        {
            back.callbacks[i] = info->callbacks[i];
            back.callback_datas[i] = info->callback_datas[i];
        }
        task_requests.enqueue(eastl::move(back));
        skr_atomic32_store_relaxed(&async_request->status, SKR_ASYNC_IO_STATUS_ENQUEUED);
        skr_atomic32_store_relaxed(&async_request->request_cancel, 0);
        TracyCZoneEnd(requestZone);
    }
    // unlock cv
    const auto sleepTimeVal = skr_atomic32_load_acquire(&_sleepTime);
    if(sleepTimeVal == SKR_IO_SERVICE_SLEEP_TIME_MAX)
    {
        skr_wake_condition_var(&sleepCv);
    }
}

skr::io::RAMService* skr::io::RAMService::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto service = SkrNew<skr::io::RAMServiceImpl>(desc->sleep_time, desc->lockless);
    service->sortMethod = desc->sort_method;
    service->sleepMode = desc->sleep_mode;
    service->threadItem.pData = service;
    service->threadItem.pFunc = &ioThreadTask;
    skr_init_mutex(&service->taskMutex);
    skr_init_mutex(&service->sleepMutex);
    service->setRunningStatus(SKR_IO_SERVICE_STATUS_RUNNING);
    service->setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
    skr_init_thread(&service->threadItem, &service->serviceThread);
    skr_set_thread_priority(service->serviceThread, SKR_THREAD_TIME_CRITICAL);
    return service;
}

void RAMService::destroy(RAMService* s) SKR_NOEXCEPT
{
    auto service = static_cast<skr::io::RAMServiceImpl*>(s);
    s->drain();
    service->setThreadStatus(_SKR_IO_THREAD_STATUS_QUIT);
    skr_join_thread(service->serviceThread);
    skr_destroy_mutex(&service->taskMutex);
    skr_destroy_mutex(&service->sleepMutex);
    skr_destroy_thread(service->serviceThread);
    SkrDelete(service);
}

bool skr::io::RAMServiceImpl::doCancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
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
    return cancelled;
}

bool skr::io::RAMServiceImpl::try_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
    if (request->is_enqueued() && !isLockless)
    {
        optionalLock();
        bool cancel = doCancel(request);
        optionalUnlock();
        return cancel;
    }
    return false;
}

void skr::io::RAMServiceImpl::defer_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
    skr_atomic32_store_relaxed(&request->request_cancel, 1);
}

void skr::io::RAMServiceImpl::drain() SKR_NOEXCEPT
{
    // wait for sleep
    for (; getRunningStatus() != SKR_IO_SERVICE_STATUS_SLEEPING;)
    {
        //...
    }
}

void skr::io::RAMServiceImpl::set_sleep_time(uint32_t time) SKR_NOEXCEPT
{
    skr_atomic32_store_relaxed(&_sleepTime, time);
}

void skr::io::RAMServiceImpl::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (getThreadStatus() != _SKR_IO_THREAD_STATUS_RUNNING) return;
    if (wait_drain) drain(); // sleep -> hung
    // else running -> hung
    setThreadStatus(_SKR_IO_THREAD_STATUS_SUSPEND);
}

void skr::io::RAMServiceImpl::run() SKR_NOEXCEPT
{
    if (getThreadStatus() != _SKR_IO_THREAD_STATUS_SUSPEND)
        return;
    setThreadStatus(_SKR_IO_THREAD_STATUS_RUNNING);
}

} // namespace io
} // namespace skr

// C API
