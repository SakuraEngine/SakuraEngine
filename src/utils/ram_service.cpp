#include "io_service_util.hpp"

// RAM Service
namespace skr
{
namespace io
{
class RAMServiceImpl final : public RAMService, public AsyncThreadedService
{
public:
    struct Task : public TaskBase {
        skr_vfs_t* vfs;
        std::string path;
        uint64_t offset;
    };
    ~RAMServiceImpl() SKR_NOEXCEPT = default;
    RAMServiceImpl(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : AsyncThreadedService(sleep_time, lockless), tasks(lockless)

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

    const eastl::string name;
    // task containers
    TaskContainer<Task> tasks;
};

void ioThreadTask_execute(skr::io::RAMServiceImpl* service)
{
    // 1.peek task
    auto task = service->tasks.peek_(service);
    // 2.load file
    if (task.has_value())
    {
        TracyCZoneC(readZone, tracy::Color::LightYellow, 1);
        TracyCZoneName(readZone, "ioServiceReadFile", strlen("ioServiceReadFile"));
        task->setTaskStatus(SKR_ASYNC_IO_STATUS_RAM_LOADING);
        auto vf = skr_vfs_fopen(task->vfs, task->path.c_str(),
        ESkrFileMode::SKR_FM_READ, ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
        if (task->request->bytes == nullptr)
        {
            // allocate
            auto fsize = skr_vfs_fsize(vf);
            task->request->size = fsize;
            task->request->bytes = (uint8_t*)sakura_malloc(fsize);
        }
        skr_vfs_fread(vf, task->request->bytes, task->offset, task->request->size);
        task->setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
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
    Task back = {};
    back.vfs = vfs;
    back.path = std::string(info->path);
    back.offset = info->offset;
    back.request = async_request;
    back.request->bytes = (uint8_t*)info->bytes;
    back.request->size = info->size;
    back.priority = info->priority;
    back.sub_priority = info->sub_priority;
    for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
    {
        back.callbacks[i] = info->callbacks[i];
        back.callback_datas[i] = info->callback_datas[i];
    }
    tasks.enqueue_(back, criticalTaskCount, name.c_str());
    AsyncThreadedService::request_();
}

skr::io::RAMService* skr::io::RAMService::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto service = SkrNew<skr::io::RAMServiceImpl>(desc->sleep_time, desc->lockless);
    service->create_(desc->sleep_mode);
    service->sortMethod = desc->sort_method;
    service->threadItem.pData = service;
    service->threadItem.pFunc = &ioThreadTask;
    skr_init_thread(&service->threadItem, &service->serviceThread);
    skr_set_thread_priority(service->serviceThread, SKR_THREAD_ABOVE_NORMAL);
    return service;
}

void RAMService::destroy(RAMService* s) SKR_NOEXCEPT
{
    auto service = static_cast<skr::io::RAMServiceImpl*>(s);
    s->drain();
    service->destroy_();
    SkrDelete(service);
}

bool skr::io::RAMServiceImpl::try_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
    return tasks.try_cancel_(request);
}

void skr::io::RAMServiceImpl::defer_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
    tasks.defer_cancel_(request);
}

void skr::io::RAMServiceImpl::drain() SKR_NOEXCEPT
{
    AsyncThreadedService::drain_();
}

void skr::io::RAMServiceImpl::set_sleep_time(uint32_t time) SKR_NOEXCEPT
{
    AsyncThreadedService::set_sleep_time_(time);
}

void skr::io::RAMServiceImpl::stop(bool wait_drain) SKR_NOEXCEPT
{
    AsyncThreadedService::stop_(wait_drain);
}

void skr::io::RAMServiceImpl::run() SKR_NOEXCEPT
{
    AsyncThreadedService::run_();
}

} // namespace io
} // namespace skr