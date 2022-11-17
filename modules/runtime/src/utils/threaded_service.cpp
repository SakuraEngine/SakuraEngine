#include "utils/threaded_service.hpp"
#include <EASTL/string.h>
#include "platform/vfs.h"
#include "io_service_util.hpp"

// RAM Service
namespace skr
{
class ThreadedServiceImpl final : public ThreadedService
{
public:
    struct Task : public io::TaskBase {
    };
    ~ThreadedServiceImpl() SKR_NOEXCEPT = default;
    ThreadedServiceImpl(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : tasks(lockless), threaded_service(sleep_time, lockless)

    {
    }
    void request(const skr_service_task_t* task, skr_async_request_t* async_request) SKR_NOEXCEPT final;
    bool try_cancel(skr_async_request_t* request) SKR_NOEXCEPT final;
    void defer_cancel(skr_async_request_t* request) SKR_NOEXCEPT final;
    void drain() SKR_NOEXCEPT final;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT final;
    void stop(bool wait_drain = false) SKR_NOEXCEPT final;
    void run() SKR_NOEXCEPT final;

    SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT final
    {
        return threaded_service.getServiceStatus();
    }

    const eastl::string name;
    // task containers
    io::TaskContainer<Task> tasks;
    io::AsyncThreadedService threaded_service;
};

void __ioThreadTask_RAM_execute(ThreadedServiceImpl* service)
{
    // 1.peek task
    service->tasks.update_(&service->threaded_service);
    auto task = service->tasks.peek_();
    // 2.load file
    if (task.has_value())
    {
        ZoneScopedN("ServiceTask");
        task->setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
    }
}

void __ioThreadTask_RAM(void* arg)
{
#ifdef TRACY_ENABLE
    static uint32_t taskIndex = 0;
    eastl::string name = "ServiceThread-";
    name.append(eastl::to_string(taskIndex++));
    tracy::SetThreadName(name.c_str());
#endif
    auto service = reinterpret_cast<ThreadedServiceImpl*>(arg);
    for (; service->threaded_service.getThreadStatus() != io::_SKR_IO_THREAD_STATUS_QUIT;)
    {
        if (service->threaded_service.getThreadStatus() == io::_SKR_IO_THREAD_STATUS_SUSPEND)
        {
            ZoneScopedN("ServiceSuspend");
            for (; service->threaded_service.getThreadStatus() == io::_SKR_IO_THREAD_STATUS_SUSPEND;)
            {
            }
        }
        __ioThreadTask_RAM_execute(service);
    }
    return;
}

void ThreadedServiceImpl::request(const skr_service_task_t* task, skr_async_request_t* async_request) SKR_NOEXCEPT
{
    ZoneScopedN("ServiceRequest");

    // try push back new request
    Task back = {};
    back.request = async_request;
    for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
    {
        back.callbacks[i] = task->callbacks[i];
        back.callback_datas[i] = task->callback_datas[i];
    }
    tasks.enqueue_(back, threaded_service.criticalTaskCount, name.c_str());
    threaded_service.request_();
}

ThreadedService* ThreadedService::create(const skr_threaded_service_desc_t* desc) SKR_NOEXCEPT
{
    auto service = SkrNew<ThreadedServiceImpl>(desc->sleep_time, desc->lockless);
    service->threaded_service.create_(desc->sleep_mode);
    service->threaded_service.sortMethod = desc->sort_method;
    service->threaded_service.threadItem.pData = service;
    service->threaded_service.threadItem.pFunc = &__ioThreadTask_RAM;
    skr_init_thread(&service->threaded_service.threadItem, &service->threaded_service.serviceThread);
    skr_set_thread_priority(service->threaded_service.serviceThread, SKR_THREAD_ABOVE_NORMAL);
    return service;
}

void ThreadedService::destroy(ThreadedService* s) SKR_NOEXCEPT
{
    auto service = static_cast<ThreadedServiceImpl*>(s);
    s->drain();
    service->threaded_service.destroy_();
    SkrDelete(service);
}

bool ThreadedServiceImpl::try_cancel(skr_async_request_t* request) SKR_NOEXCEPT
{
    return tasks.try_cancel_(request);
}

void ThreadedServiceImpl::defer_cancel(skr_async_request_t* request) SKR_NOEXCEPT
{
    tasks.defer_cancel_(request);
}

void ThreadedServiceImpl::drain() SKR_NOEXCEPT
{
    threaded_service.drain_();
}

void ThreadedServiceImpl::set_sleep_time(uint32_t time) SKR_NOEXCEPT
{
    threaded_service.set_sleep_time_(time);
}

void ThreadedServiceImpl::stop(bool wait_drain) SKR_NOEXCEPT
{
    threaded_service.stop_(wait_drain);
}

void ThreadedServiceImpl::run() SKR_NOEXCEPT
{
    threaded_service.run_();
}

} // namespace skr