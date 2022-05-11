#include "utils/io.h"
#include "utils/io.hpp"
#include "utils/log.h"
#include "platform/memory.h"
#include "platform/thread.h"
#include <EASTL/unique_ptr.h>
#include <EASTL/deque.h>

bool skr_async_io_request_t::is_ready() const
{
    return SKR_ASYNC_IO_STATUS_OK == skr_atomic32_load_acquire(&status);
}
bool skr_async_io_request_t::is_enqueued() const
{
    return SKR_ASYNC_IO_STATUS_ENQUEUED == skr_atomic32_load_acquire(&status);
}
bool skr_async_io_request_t::is_cancelled() const
{
    return SKR_ASYNC_IO_STATUS_CANCELLED == skr_atomic32_load_acquire(&status);
}
bool skr_async_io_request_t::is_ram_loading() const
{
    return SKR_ASYNC_IO_STATUS_RAM_LOADING == skr_atomic32_load_acquire(&status);
}
bool skr_async_io_request_t::is_vram_loading() const
{
    return SKR_ASYNC_IO_STATUS_VRAM_LOADING == skr_atomic32_load_acquire(&status);
}

// RAM Service
namespace skr
{
namespace io
{
class RAMServiceImpl final : public RAMService
{
public:
    struct Task {
        skr_vfs_t* vfs;
        eastl::string path;
        uint8_t* bytes;
        uint64_t offset;
        uint64_t size;
        SAtomic32* status;
    };

    ~RAMServiceImpl() = default;
    RAMServiceImpl() = default;
    void request(skr_vfs_t*, const skr_ram_io_t* info, skr_async_io_request_t* async_request) RUNTIME_NOEXCEPT final;
    bool try_cancel(skr_async_io_request_t* request) RUNTIME_NOEXCEPT final;
    void drain() RUNTIME_NOEXCEPT final;

    SMutex taskMutex;
    SThreadDesc threadItem = {};
    SThreadHandle serviceThread;
    eastl::deque<Task> tasks;
    SAtomic32 running;
    const eastl::string name;
    bool criticalTaskCount = false;
    RAMServiceImpl::Task current;
};

void ioThreadTask_execute(skr::io::RAMServiceImpl* service)
{
    // try fetch a new task
    {
        SMutexLock lock(service->taskMutex);
        if (!service->tasks.size())
        {
            skr_thread_sleep(1);
            return;
        }
        service->current = service->tasks.front();
        service->tasks.pop_front();
    }
    // do io work
    auto vf = skr_vfs_fopen(service->current.vfs, service->current.path.c_str(),
    ESkrFileMode::SKR_FM_READ, ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
    skr_atomic32_store_relaxed(service->current.status, SKR_ASYNC_IO_STATUS_RAM_LOADING);
    skr_vfs_fread(vf, service->current.bytes, service->current.offset, service->current.size);
    skr_atomic32_store_relaxed(service->current.status, SKR_ASYNC_IO_STATUS_OK);
    skr_vfs_fclose(vf);
}

void ioThreadTask(void* arg)
{
    auto service = reinterpret_cast<skr::io::RAMServiceImpl*>(arg);
    for (; skr_atomic32_load_acquire(&service->running);)
    {
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
    back.status = &async_request->status;
    skr_atomic32_store_relaxed(&async_request->status, SKR_ASYNC_IO_STATUS_ENQUEUED);
}

skr::io::RAMService* skr::io::RAMService::create(const skr_ram_io_service_desc_t* desc) RUNTIME_NOEXCEPT
{
    auto service = SkrNew<skr::io::RAMServiceImpl>();
    service->threadItem.pData = service;
    service->threadItem.pFunc = &ioThreadTask;
    skr_init_mutex(&service->taskMutex);
    skr_atomic32_store_relaxed(&service->running, true);
    skr_init_thread(&service->threadItem, &service->serviceThread);
    return service;
}

void RAMService::destroy(RAMService* s) RUNTIME_NOEXCEPT
{
    auto service = static_cast<skr::io::RAMServiceImpl*>(s);
    s->drain();
    skr_atomic32_store_relaxed(&service->running, false);
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
            cancel = t.status == &request->status;
            if (cancel)
                skr_atomic32_store_relaxed(t.status, SKR_ASYNC_IO_STATUS_CANCELLED);
            return cancel;
        }),
        tasks.end());
        return cancel;
    }
    return false;
}

void skr::io::RAMServiceImpl::drain() RUNTIME_NOEXCEPT
{
    bool drain = false;
    while (!drain)
    {
        bool acquired = skr_try_acquire_mutex(&taskMutex);
        if (acquired)
        {
            if (current.status)
            {
                auto status = skr_atomic32_load_acquire(current.status);
                drain = !tasks.size() &&
                        (status == SKR_ASYNC_IO_STATUS_OK || status == SKR_ASYNC_IO_STATUS_CANCELLED);
            }
            else
            {
                drain = !tasks.size();
            }
            skr_release_mutex(&taskMutex);
        }
    }
}

} // namespace io
} // namespace skr

// C API
