#include "utils/io.h"
#include "utils/io.hpp"
#include "utils/log.h"
#include "platform/memory.h"
#include "platform/thread.h"
#include <EASTL/unique_ptr.h>
#include <EASTL/deque.h>
#include <EASTL/sort.h>

bool skr_async_io_request_t::is_ready() const RUNTIME_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_OK;
}
bool skr_async_io_request_t::is_enqueued() const RUNTIME_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_ENQUEUED;
}
bool skr_async_io_request_t::is_cancelled() const RUNTIME_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_CANCELLED;
}
bool skr_async_io_request_t::is_ram_loading() const RUNTIME_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_RAM_LOADING;
}
bool skr_async_io_request_t::is_vram_loading() const RUNTIME_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_VRAM_LOADING;
}

SkrAsyncIOStatus skr_async_io_request_t::get_status() const RUNTIME_NOEXCEPT
{
    return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&status);
}

// RAM Service
namespace skr
{
namespace io
{
#define SKR_IO_THREAD_STATUS_RUNNING 0
#define SKR_IO_THREAD_STATUS_QUIT 1
#define SKR_IO_THREAD_STATUS_SUSPEND 2
class RAMServiceImpl final : public RAMService
{
public:
    struct Task {
        skr_vfs_t* vfs;
        eastl::string path;
        uint8_t* bytes;
        uint64_t offset;
        uint64_t size;
        SAtomic32* _status;
        SkrIOServicePriority priority;
        float sub_priority;
        bool operator<(const Task& rhs) const
        {
            if (rhs.priority != priority)
                return priority > rhs.priority;
            return sub_priority > rhs.sub_priority;
        }
        SkrAsyncIOStatus getTaskStatus() const
        {
            return (SkrAsyncIOStatus)skr_atomic32_load_acquire(_status);
        }
        void setTaskStatus(SkrAsyncIOStatus value)
        {
            skr_atomic32_store_relaxed(_status, value);
        }
    };
    ~RAMServiceImpl() RUNTIME_NOEXCEPT = default;
    RAMServiceImpl(uint32_t sleep_time) RUNTIME_NOEXCEPT
        : _sleepTime(sleep_time)
    {
    }
    void request(skr_vfs_t*, const skr_ram_io_t* info, skr_async_io_request_t* async_request) RUNTIME_NOEXCEPT final;
    bool try_cancel(skr_async_io_request_t* request) RUNTIME_NOEXCEPT final;
    void drain() RUNTIME_NOEXCEPT final;
    void set_sleep_time(uint32_t time) RUNTIME_NOEXCEPT final;
    void stop(bool wait_drain = false) RUNTIME_NOEXCEPT final;
    void run() RUNTIME_NOEXCEPT final;

    SkrAsyncIOServiceStatus get_service_status() const RUNTIME_NOEXCEPT final
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

    SMutex taskMutex;
    SThreadDesc threadItem = {};
    SThreadHandle serviceThread;
    eastl::deque<Task> tasks;
    SAtomic32 _running_status /*SkrAsyncIOServiceStatus*/;
    SAtomic32 _thread_status = SKR_IO_THREAD_STATUS_RUNNING /*IOThreadStatus*/;
    SAtomic32 _sleepTime = 30 /*ms*/;
    const bool criticalTaskCount = false;
    const eastl::string name;
    // state
    RAMServiceImpl::Task current;
};

void ioThreadTask_execute(skr::io::RAMServiceImpl* service)
{
    // try fetch a new task
    {
        SMutexLock lock(service->taskMutex);
        // empty sleep
        if (!service->tasks.size())
        {
            service->setRunningStatus(SKR_ASYNC_IO_SERVICE_STATUS_SLEEPING);
            const auto sleepTimeVal = skr_atomic32_load_acquire(&service->_sleepTime);
            if (sleepTimeVal != SKR_ASYNC_IO_SERVICE_SLEEP_TIME_NEVER)
            {
                auto sleepTime = eastl::min(sleepTimeVal, 100u);
                sleepTime = eastl::max(sleepTimeVal, 1u);
                skr_thread_sleep(sleepTime);
            }
            return;
        }
        else // do sort
        {
            service->setRunningStatus(SKR_ASYNC_IO_SERVICE_STATUS_RUNNING);
            eastl::sort(service->tasks.begin(), service->tasks.end());
        }
        // pop current task
        service->current = service->tasks.front();
        service->tasks.pop_front();
    }
    // do io work
    service->current.setTaskStatus(SKR_ASYNC_IO_STATUS_RAM_LOADING);
    auto vf = skr_vfs_fopen(service->current.vfs, service->current.path.c_str(),
    ESkrFileMode::SKR_FM_READ, ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
    skr_vfs_fread(vf, service->current.bytes, service->current.offset, service->current.size);
    service->current.setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
    skr_vfs_fclose(vf);
}

void ioThreadTask(void* arg)
{
    auto service = reinterpret_cast<skr::io::RAMServiceImpl*>(arg);
    for (; skr_atomic32_load_acquire(&service->_thread_status) != SKR_IO_THREAD_STATUS_QUIT;)
    {
        for (; skr_atomic32_load_acquire(&service->_thread_status) == SKR_IO_THREAD_STATUS_SUSPEND;)
        {
        }
        ioThreadTask_execute(service);
    }
}

void skr::io::RAMServiceImpl::request(skr_vfs_t* vfs, const skr_ram_io_t* info, skr_async_io_request_t* async_request) RUNTIME_NOEXCEPT
{
    // try push back new request
    SMutexLock lock(taskMutex);
    if (tasks.size() >= SKR_ASYNC_IO_SERVICE_MAX_TASK_COUNT)
    {
        SKR_LOG_WARN(
        "ioRAMService %s enqueued too many tasks(over %d)!",
        name.c_str(), SKR_ASYNC_IO_SERVICE_MAX_TASK_COUNT);
        if (criticalTaskCount)
        {
            skr_release_mutex(&taskMutex);
            return;
        }
    }
    auto& back = tasks.push_back();
    back.vfs = vfs;
    back.path = info->path;
    back.bytes = info->bytes;
    back.offset = info->offset;
    back.size = info->size;
    back._status = &async_request->status;
    back.priority = info->priority;
    back.sub_priority = info->sub_priority;
    skr_atomic32_store_relaxed(&async_request->status, SKR_ASYNC_IO_STATUS_ENQUEUED);
}

skr::io::RAMService* skr::io::RAMService::create(const skr_ram_io_service_desc_t* desc) RUNTIME_NOEXCEPT
{
    auto service = SkrNew<skr::io::RAMServiceImpl>(desc->sleep_time);
    service->threadItem.pData = service;
    service->threadItem.pFunc = &ioThreadTask;
    skr_init_mutex(&service->taskMutex);
    service->setRunningStatus(SKR_ASYNC_IO_SERVICE_STATUS_RUNNING);
    skr_atomic32_store_relaxed(&service->_thread_status, SKR_IO_THREAD_STATUS_RUNNING);
    skr_init_thread(&service->threadItem, &service->serviceThread);
    return service;
}

void RAMService::destroy(RAMService* s) RUNTIME_NOEXCEPT
{
    auto service = static_cast<skr::io::RAMServiceImpl*>(s);
    s->drain();
    skr_atomic32_store_relaxed(&service->_thread_status, SKR_IO_THREAD_STATUS_QUIT);
    skr_join_thread(service->serviceThread);
    skr_destroy_mutex(&service->taskMutex);
    skr_destroy_thread(service->serviceThread);
    SkrDelete(service);
}

bool skr::io::RAMServiceImpl::try_cancel(skr_async_io_request_t* request) RUNTIME_NOEXCEPT
{
    if (request->is_enqueued())
    {
        SMutexLock lock(taskMutex);
        bool cancel = false;
        // erase cancelled task
        tasks.erase(
        eastl::remove_if(tasks.begin(), tasks.end(),
        [=, &cancel](Task& t) {
            cancel = t._status == &request->status;
            if (cancel)
                t.setTaskStatus(SKR_ASYNC_IO_STATUS_CANCELLED);
            return cancel;
        }),
        tasks.end());
        return cancel;
    }
    return false;
}

void skr::io::RAMServiceImpl::drain() RUNTIME_NOEXCEPT
{
    // wait for sleep
    for (; getRunningStatus() != SKR_ASYNC_IO_SERVICE_STATUS_SLEEPING;)
    {
        //...
    }
}

void skr::io::RAMServiceImpl::set_sleep_time(uint32_t time) RUNTIME_NOEXCEPT
{
    skr_atomic32_store_relaxed(&_sleepTime, time);
}

void skr::io::RAMServiceImpl::stop(bool wait_drain) RUNTIME_NOEXCEPT
{
    auto destroyed = skr_atomic32_load_acquire(&_thread_status);
    if (destroyed) return;
    if (wait_drain) drain(); // sleep -> hung
    // else running -> hung
    skr_atomic32_store_relaxed(&_thread_status, SKR_IO_THREAD_STATUS_SUSPEND);
}

void skr::io::RAMServiceImpl::run() RUNTIME_NOEXCEPT
{
    if (skr_atomic32_load_acquire(&_thread_status) != SKR_IO_THREAD_STATUS_SUSPEND)
        return;
    skr_atomic32_store_relaxed(&_thread_status, SKR_IO_THREAD_STATUS_RUNNING);
}

} // namespace io
} // namespace skr

// C API
