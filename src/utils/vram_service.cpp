#include "cgpu/io.hpp"
#include "utils/make_zeroed.hpp"
#include "io_service_util.hpp"
#include <EASTL/variant.h>
#include <EASTL/vector_map.h>

// RAM Service
namespace skr
{
namespace io
{
enum EVramTaskStep
{
    kStepNone = 0,
    kStepResourceCreated,
    kStepUploading,
    kStepDirectStorage,
    kStepResourceBarrier,
    kStepFinished
};

class VRAMServiceImpl final : public VRAMService
{
public:
    struct CGPUUploadTask
    {
        CGPUQueueId queue = nullptr;
        CGPUCommandPoolId command_pool = nullptr;
        CGPUCommandBufferId command_buffer = nullptr;
        CGPUFenceId fence = nullptr;
        CGPUSemaphoreId semaphore = nullptr;
        CGPUBufferId upload_buffer = nullptr;
        CGPUBufferId dst_buffer = nullptr;
        CGPUTextureId dst_texture = nullptr;
        bool finished = false;
    };
    struct BufferTask {
        skr_vram_buffer_io_t buffer_io;
        skr_vram_buffer_request_t* buffer_request;
        CGPUUploadTask* upload_task;
    };
    struct Task : public TaskBase {
        eastl::string path;
        eastl::variant<BufferTask> resource_task;
        EVramTaskStep step;
    };
    ~VRAMServiceImpl() SKR_NOEXCEPT = default;
    VRAMServiceImpl(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : tasks(lockless), threaded_service(sleep_time, lockless)

    {
    }
    void request(const skr_vram_buffer_io_t* info, skr_async_io_request_t* async_request, skr_vram_buffer_request_t* buffer_request) SKR_NOEXCEPT final;
    bool try_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT final;
    void defer_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT final;
    void drain() SKR_NOEXCEPT final;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT final;
    void stop(bool wait_drain = false) SKR_NOEXCEPT final;
    void run() SKR_NOEXCEPT final;

    SkrAsyncIOServiceStatus get_service_status() const SKR_NOEXCEPT final
    {
        return threaded_service.getRunningStatus();
    }

    // create resource
    void createResource(Task& task) SKR_NOEXCEPT;
    void tryCreateBufferResource(Task& task) SKR_NOEXCEPT;
    // create resource

    // upload resource
    void uploadResource(Task& task) SKR_NOEXCEPT;
    void tryUploadBufferResource(Task& task) SKR_NOEXCEPT;
    // upload resource

    // status check
    bool vramIOFinished(Task& task) SKR_NOEXCEPT;
    // status check

    // cgpu helpers
    CGPUUploadTask* allocateCGPUUploadTask(CGPUDeviceId, CGPUQueueId, CGPUSemaphoreId) SKR_NOEXCEPT; 
    void freeCGPUUploadTask(CGPUUploadTask* task) SKR_NOEXCEPT; 
    // cgpu helpers

    const eastl::string name;
    // task containers
    TaskContainer<Task> tasks;
    AsyncThreadedService threaded_service;
    // CGPU Objects
    eastl::vector<CGPUUploadTask*> resource_uploads;
};

// create resource
void skr::io::VRAMServiceImpl::createResource(skr::io::VRAMServiceImpl::Task &task) SKR_NOEXCEPT
{
    tryCreateBufferResource(task);
}

void skr::io::VRAMServiceImpl::tryCreateBufferResource(skr::io::VRAMServiceImpl::Task &task) SKR_NOEXCEPT
{
    if (auto buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
        SKR_ASSERT( (buffer_task->buffer_io.size || buffer_task->buffer_io.path) && "buffer_io.size or buffer_io.path must be set");
        if (buffer_task->buffer_io.size)
        {
            auto buffer_desc = make_zeroed<CGPUBufferDescriptor>();
            const auto& buffer_io = buffer_task->buffer_io;
            buffer_desc.size = buffer_io.buffer_size;
            buffer_desc.name = buffer_io.buffer_name;
            buffer_desc.descriptors = buffer_io.resource_types;
            buffer_desc.memory_usage = buffer_io.memory_usage;
            buffer_desc.format = buffer_io.format;
            buffer_desc.flags = buffer_io.flags;
            buffer_desc.start_state = buffer_io.start_state;
            buffer_desc.prefer_on_device = buffer_io.prefer_on_device;
            buffer_desc.prefer_on_host = buffer_io.prefer_on_host;
            auto buffer = cgpu_create_buffer(buffer_task->buffer_io.device, &buffer_desc);
            // return resource object
            buffer_task->buffer_request->out_buffer = buffer;
        }
        else if (buffer_task->buffer_io.path)
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
    }
}
// create resource

// upload resource
void skr::io::VRAMServiceImpl::uploadResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    tryUploadBufferResource(task);
}

void skr::io::VRAMServiceImpl::tryUploadBufferResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
        SKR_ASSERT( task.step == kStepUploading && "task.step must be kStepUploading");

        const auto& buffer_io = buffer_task->buffer_io;
        const auto& buffer_request = buffer_task->buffer_request;
        CGPUUploadTask* upload = allocateCGPUUploadTask(buffer_io.device, buffer_io.transfer_queue, buffer_io.opt_semaphore);
        eastl::string name = buffer_io.buffer_name;
        name += "-upload";
        upload->upload_buffer = cgpux_create_mapped_upload_buffer(buffer_io.device, buffer_request->out_buffer->size, name.c_str());
        upload->dst_buffer = buffer_request->out_buffer;

        memcpy((uint8_t*)upload->upload_buffer->cpu_mapped_address + buffer_io.offset, buffer_io.bytes, buffer_io.size);

        cgpu_cmd_begin(upload->command_buffer);
        CGPUBufferToBufferTransfer vb_cpy = {};
        vb_cpy.dst = buffer_request->out_buffer;
        vb_cpy.dst_offset = 0;
        vb_cpy.src = upload->upload_buffer;
        vb_cpy.src_offset = 0;
        vb_cpy.size = buffer_io.size;
        cgpu_cmd_transfer_buffer_to_buffer(upload->command_buffer, &vb_cpy);
        
        auto buffer_barrier = make_zeroed<CGPUBufferBarrier>();
        buffer_barrier.buffer = buffer_request->out_buffer;
        buffer_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        buffer_barrier.dst_state = buffer_io.start_state;
        // acquire
        if (buffer_io.owner_queue->type != buffer_io.transfer_queue->type)
        {
            buffer_barrier.queue_acquire = true;
            buffer_barrier.queue_type = buffer_io.owner_queue->type;
        }
        
        auto barrier = make_zeroed<CGPUResourceBarrierDescriptor>();
        barrier.buffer_barriers = &buffer_barrier;
        barrier.buffer_barriers_count = 1;
        cgpu_cmd_resource_barrier(upload->command_buffer, &barrier);
        cgpu_cmd_end(upload->command_buffer);

        CGPUQueueSubmitDescriptor submit_desc = {};
        submit_desc.cmds = &upload->command_buffer;
        submit_desc.cmds_count = 1;
        submit_desc.signal_semaphore_count = upload->semaphore ? 1 : 0;
        submit_desc.signal_semaphores = upload->semaphore ? &upload->semaphore : nullptr;
        submit_desc.signal_fence = upload->fence;
        cgpu_submit_queue(buffer_io.transfer_queue, &submit_desc);

        buffer_task->upload_task = upload;

        resource_uploads.emplace_back(upload);
    }
}
// upload resource

// status check
bool skr::io::VRAMServiceImpl::vramIOFinished(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
        SKR_ASSERT(buffer_task->upload_task != nullptr);
        if (auto status = cgpu_query_fence_status(buffer_task->upload_task->fence))
        {
            if (status == CGPU_FENCE_STATUS_COMPLETE)
            {
                buffer_task->upload_task->finished = true;
                return true;
            }
        }
    }
    return false;
}
// status check

// cgpu helpers
skr::io::VRAMServiceImpl::CGPUUploadTask* skr::io::VRAMServiceImpl::allocateCGPUUploadTask(CGPUDeviceId device, CGPUQueueId queue, CGPUSemaphoreId semaphore) SKR_NOEXCEPT
{
    auto upload = SkrNew<skr::io::VRAMServiceImpl::CGPUUploadTask>();
    auto cmd_pool_desc = make_zeroed<CGPUCommandPoolDescriptor>();
    upload->command_pool = cgpu_create_command_pool(queue, &cmd_pool_desc);
    auto cmd_desc = make_zeroed<CGPUCommandBufferDescriptor>();
    cmd_desc.is_secondary = false;
    upload->command_buffer = cgpu_create_command_buffer(upload->command_pool, &cmd_desc);
    upload->queue = queue;
    upload->fence = cgpu_create_fence(device);
    upload->semaphore = semaphore;
    return upload;
}

void skr::io::VRAMServiceImpl::freeCGPUUploadTask(skr::io::VRAMServiceImpl::CGPUUploadTask* upload) SKR_NOEXCEPT
{
#ifdef _DEBUG
    auto fenceStatus = cgpu_query_fence_status(upload->fence);
    if (fenceStatus != CGPU_FENCE_STATUS_COMPLETE)
    {
        SKR_LOG_WARN("fence status is not complete");
        cgpu_wait_fences(&upload->fence, 1);
    }
#endif
    if (upload->upload_buffer) cgpu_free_buffer(upload->upload_buffer);
    cgpu_free_command_buffer(upload->command_buffer);
    cgpu_free_command_pool(upload->command_pool);
    cgpu_free_fence(upload->fence);
    SkrDelete(upload);
}
// cgpu helpers

void __ioThreadTask_VRAM_execute(skr::io::VRAMServiceImpl* service)
{
    // 1.visit task
    service->tasks.update_(&service->threaded_service);
    auto foreach_task = [service](auto& task)
    {
        const auto vramIOStep = task.step;
        switch (vramIOStep)
        {
            case kStepNone: // start create resource
                task.setTaskStatus(SKR_ASYNC_IO_STATUS_CREATING_RESOURCE);
                service->createResource(task);
                task.step = kStepResourceCreated;
                break;
            case kStepResourceCreated: // start uploading
                task.setTaskStatus(SKR_ASYNC_IO_STATUS_VRAM_LOADING);
                service->uploadResource(task);
                task.step = kStepUploading;
                break;
            case kStepUploading:
                if (service->vramIOFinished(task))
                {
                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
                    task.step = kStepFinished;
                }
                else task.step = kStepUploading; // continue uploading
                break;
            case kStepDirectStorage:
            case kStepResourceBarrier:
                SKR_UNIMPLEMENTED_FUNCTION();
            case kStepFinished:
                return;
        }
       

    };
    service->tasks.visit_(foreach_task);
    // 2.sweep upload tasks
    eastl::for_each(service->resource_uploads.begin(), service->resource_uploads.end(),
        [service](auto& upload){
            if (upload->finished)
            {
                service->freeCGPUUploadTask(upload);
                upload = nullptr;
            }
        });
    service->resource_uploads.erase(
        eastl::remove_if(service->resource_uploads.begin(), service->resource_uploads.end(),
        [](auto& upload){
            return upload == nullptr;
        }), service->resource_uploads.end());
}

void __ioThreadTask_VRAM(void* arg)
{
#ifdef TRACY_ENABLE
    static uint32_t taskIndex = 0;
    eastl::string name = "ioRAMServiceThread-";
    name.append(eastl::to_string(taskIndex++));
    tracy::SetThreadName(name.c_str());
#endif
    auto service = reinterpret_cast<skr::io::VRAMServiceImpl*>(arg);
    for (; service->threaded_service.getThreadStatus() != _SKR_IO_THREAD_STATUS_QUIT;)
    {
        if (service->threaded_service.getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND)
        {
            ZoneScopedN("ioServiceSuspend");
            for (; service->threaded_service.getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND;)
            {
            }
        }
        __ioThreadTask_VRAM_execute(service);
    }
}

void skr::io::VRAMServiceImpl::request(const skr_vram_buffer_io_t* info, skr_async_io_request_t* async_request, skr_vram_buffer_request_t* buffer_request) SKR_NOEXCEPT
{
    // try push back new request
    auto io_task = make_zeroed<Task>();
    io_task.priority = info->priority;
    io_task.sub_priority = info->sub_priority;
    io_task.request = async_request;
    for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
    {
        io_task.callbacks[i] = info->callbacks[i];
        io_task.callback_datas[i] = info->callback_datas[i];
    }

    io_task.path = info->path;
    io_task.resource_task = make_zeroed<BufferTask>();
    auto&& buffer_task = eastl::get<BufferTask>(io_task.resource_task);
    buffer_task.buffer_io = *info;
    buffer_task.buffer_request = buffer_request;

    tasks.enqueue_(io_task, threaded_service.criticalTaskCount, name.c_str());
    threaded_service.request_();
}

skr::io::VRAMService* skr::io::VRAMService::create(const skr_vram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto service = SkrNew<skr::io::VRAMServiceImpl>(desc->sleep_time, desc->lockless);
    service->threaded_service.create_(desc->sleep_mode);
    service->threaded_service.sortMethod = desc->sort_method;
    service->threaded_service.threadItem.pData = service;
    service->threaded_service.threadItem.pFunc = &__ioThreadTask_VRAM;
    skr_init_thread(&service->threaded_service.threadItem, &service->threaded_service.serviceThread);
    skr_set_thread_priority(service->threaded_service.serviceThread, SKR_THREAD_ABOVE_NORMAL);
    return service;
}

void VRAMService::destroy(VRAMService* s) SKR_NOEXCEPT
{
    auto service = static_cast<skr::io::VRAMServiceImpl*>(s);
    s->drain();
    service->threaded_service.destroy_();
    SkrDelete(service);
}

bool skr::io::VRAMServiceImpl::try_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
    return tasks.try_cancel_(request);
}

void skr::io::VRAMServiceImpl::defer_cancel(skr_async_io_request_t* request) SKR_NOEXCEPT
{
    tasks.defer_cancel_(request);
}

void skr::io::VRAMServiceImpl::drain() SKR_NOEXCEPT
{
    threaded_service.drain_();
}

void skr::io::VRAMServiceImpl::set_sleep_time(uint32_t time) SKR_NOEXCEPT
{
    threaded_service.set_sleep_time_(time);
}

void skr::io::VRAMServiceImpl::stop(bool wait_drain) SKR_NOEXCEPT
{
    threaded_service.stop_(wait_drain);
}

void skr::io::VRAMServiceImpl::run() SKR_NOEXCEPT
{
    threaded_service.run_();
}
}
}