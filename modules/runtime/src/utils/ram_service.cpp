#include <containers/string.hpp>
#include "platform/vfs.h"
#include "utils/io.h"
#include "io_service_util.hpp"

// TODO: implement with new async_task
#include "utils/async_task.hpp"
skr::FutureStatus s;

// RAM Service
namespace skr
{
namespace io
{
class RAMServiceImpl final : public skr_io_ram_service_t
{
public:
    struct Task : public TaskBase {
        skr_vfs_t* vfs;
        skr::string path;
        uint64_t offset;
        skr_async_ram_destination_t* destination;
    };
    ~RAMServiceImpl() SKR_NOEXCEPT = default;
    RAMServiceImpl(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : tasks(lockless), threaded_service(sleep_time, lockless)

    {
    }
    void request(skr_vfs_t*, const skr_ram_io_t* info, skr_async_request_t* async_request, skr_async_ram_destination_t* dst) SKR_NOEXCEPT final;
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

    const skr::string name;
    // task containers
    TaskContainer<Task> tasks;
    AsyncThreadedService threaded_service;
};

void __ioThreadTask_RAM_execute(skr::io::RAMServiceImpl* service)
{
    // 1.peek task
    service->tasks.update_(&service->threaded_service);
    auto task = service->tasks.peek_();
    // 2.load file
    if (task.has_value())
    {
        ZoneScopedN("ioServiceReadFile");
        skr_vfile_t* vf = nullptr;
        if (task->vfs)
        {
            task->setTaskStatus(SKR_ASYNC_IO_STATUS_CREATING_RESOURCE);
            {
                ZoneScopedNC("FOpen", tracy::Color::LightBlue);
                vf = skr_vfs_fopen(task->vfs, (const char8_t*)task->path.c_str(),
                    ESkrFileMode::SKR_FM_READ_BINARY, ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
            }
            if (task->destination->bytes == nullptr)
            {
                ZoneScopedNC("FileMemoryAllocate", tracy::Color::LightBlue);
                TracyMessage(task->path.c_str(), task->path.size());
                // allocate
                auto fsize = skr_vfs_fsize(vf);
                task->destination->size = fsize;
                task->destination->bytes = (uint8_t*)sakura_malloc(fsize);
            }
            {
                ZoneScopedN("BeforeLoadingCallback");
                task->setTaskStatus(SKR_ASYNC_IO_STATUS_RAM_LOADING);
            }
            {
                ZoneScopedNC("FRead", tracy::Color::LightBlue);
                skr_vfs_fread(vf, task->destination->bytes, task->offset, task->destination->size);
            }
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
        {
            ZoneScopedN("LoadingOKCallback");
            task->setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
        }
        skr_vfs_fclose(vf);
    }
}

void __ioThreadTask_RAM(void* arg)
{
#ifdef TRACY_ENABLE
    static uint32_t taskIndex = 0;
    skr::string name = "ioRAMServiceThread-";
    name.append(skr::to_string(taskIndex++));
    tracy::SetThreadName(name.c_str());
#endif
    auto service = reinterpret_cast<skr::io::RAMServiceImpl*>(arg);
    for (; service->threaded_service.getThreadStatus() != _SKR_IO_THREAD_STATUS_QUIT;)
    {
        if (service->threaded_service.getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND)
        {
            ZoneScopedN("ioServiceSuspend");
            for (; service->threaded_service.getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND;)
            {
            }
        }
        __ioThreadTask_RAM_execute(service);
    }
    return;
}

void skr::io::RAMServiceImpl::request(skr_vfs_t* vfs, const skr_ram_io_t* info, 
    skr_async_request_t* async_request, skr_async_ram_destination_t* dst) SKR_NOEXCEPT
{
    ZoneScopedN("ioRAMRequest");

    // try push back new request
    Task back = {};
    back.vfs = vfs;
    back.path = skr::string((const char*)info->path);
    back.offset = info->offset;
    back.request = async_request;
    back.destination = dst;
    back.priority = info->priority;
    back.sub_priority = info->sub_priority;
    for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
    {
        back.callbacks[i] = info->callbacks[i];
        back.callback_datas[i] = info->callback_datas[i];
    }
    tasks.enqueue_(back, threaded_service.criticalTaskCount, name.c_str());
    threaded_service.request_();
}

bool skr::io::RAMServiceImpl::try_cancel(skr_async_request_t* request) SKR_NOEXCEPT
{
    return tasks.try_cancel_(request);
}

void skr::io::RAMServiceImpl::defer_cancel(skr_async_request_t* request) SKR_NOEXCEPT
{
    tasks.defer_cancel_(request);
}

void skr::io::RAMServiceImpl::drain() SKR_NOEXCEPT
{
    threaded_service.drain_();
}

void skr::io::RAMServiceImpl::set_sleep_time(uint32_t time) SKR_NOEXCEPT
{
    threaded_service.set_sleep_time_(time);
}

void skr::io::RAMServiceImpl::stop(bool wait_drain) SKR_NOEXCEPT
{
    threaded_service.stop_(wait_drain);
}

void skr::io::RAMServiceImpl::run() SKR_NOEXCEPT
{
    threaded_service.run_();
}

} // namespace io
} // namespace skr


skr_io_ram_service_t* skr_io_ram_service_t::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto service = SkrNew<skr::io::RAMServiceImpl>(desc->sleep_time, desc->lockless);
    service->threaded_service.create_(desc->sleep_mode);
    service->threaded_service.sortMethod = desc->sort_method;
    service->threaded_service.threadItem.pData = service;
    service->threaded_service.threadItem.pFunc = &skr::io::__ioThreadTask_RAM;
    skr_init_thread(&service->threaded_service.threadItem, &service->threaded_service.serviceThread);
    skr_thread_set_priority(service->threaded_service.serviceThread, SKR_THREAD_ABOVE_NORMAL);
    return service;
}

void skr_io_ram_service_t::destroy(skr_io_ram_service_t* s) SKR_NOEXCEPT
{
    auto service = static_cast<skr::io::RAMServiceImpl*>(s);
    s->drain();
    service->threaded_service.destroy_();
    SkrDelete(service);
}
