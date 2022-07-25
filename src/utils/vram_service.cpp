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
    struct TaskBatch;
    struct CGPUUploadTask
    {
        CGPUQueueId queue = nullptr;
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

    struct CGPUDStorageTask {
        CGPUDStorageQueueId storage_queue = nullptr;
        CGPUBufferId dst_buffer = nullptr;
        CGPUDStorageFileHandle ds_file = nullptr;
        bool finished = false;
    };
    struct DStorageBufferTask
    {
        skr_vram_buffer_io_t buffer_io;
        skr_vram_buffer_request_t* buffer_request;
        CGPUDStorageTask* dstorage_task;
    };

    struct Task : public TaskBase {
        eastl::string path;
        eastl::variant<BufferTask, DStorageBufferTask> resource_task;
        EVramTaskStep step;
        TaskBatch* task_batch;
        bool isDStorage() const
        {
            return eastl::get_if<DStorageBufferTask>(&resource_task);
        }
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

    // dstorage resource
    void dstorageResource(Task& task) SKR_NOEXCEPT;
    void tryDStorageBufferResource(Task& task) SKR_NOEXCEPT;
    // dstorage resource

    // status check
    bool vramIOFinished(Task& task) SKR_NOEXCEPT;
    // status check

    // cgpu helpers
    CGPUUploadTask* allocateCGPUUploadTask(CGPUDeviceId, CGPUQueueId, CGPUSemaphoreId) SKR_NOEXCEPT; 
    void freeCGPUUploadTask(CGPUUploadTask* task) SKR_NOEXCEPT; 
    CGPUDStorageTask* allocateCGPUDStorageTask(CGPUDeviceId, CGPUDStorageQueueId, CGPUDStorageFileHandle) SKR_NOEXCEPT; 
    void freeCGPUDStorageTask(CGPUDStorageTask* task) SKR_NOEXCEPT; 
    // cgpu helpers

    const eastl::string name;
    // task containers
    TaskContainer<Task> tasks;
    struct TaskBatch
    {
        eastl::vector<Task> tasks;
        eastl::vector_map<CGPUQueueId, CGPUFenceId> fences;
        eastl::vector_map<CGPUQueueId, CGPUCommandPoolId> cmd_pools;
        eastl::vector_map<CGPUQueueId, CGPUCommandBufferId> cmds;
        bool submitted = false;
        uint64_t id;

        CGPUFenceId get_fence(CGPUQueueId queue) SKR_NOEXCEPT
        {
            auto it = fences.find(queue);
            if (it != fences.end()) return it->second;
            auto fence = cgpu_create_fence(queue->device);
            fences[queue] = fence;
            return fence;
        }
        CGPUCommandPoolId get_cmd_pool(CGPUQueueId queue) SKR_NOEXCEPT
        {
            auto it = cmd_pools.find(queue);
            if (it != cmd_pools.end()) return it->second;
            CGPUCommandPoolDescriptor cmd_pool_desc = {};
            auto cmd_pool = cgpu_create_command_pool(queue, &cmd_pool_desc);
            cmd_pools[queue] = cmd_pool;
            return cmd_pool;
        }
        CGPUCommandBufferId get_cmd(CGPUQueueId queue) SKR_NOEXCEPT
        {
            auto it = cmds.find(queue);
            if (it != cmds.end()) return it->second;
            CGPUCommandBufferDescriptor cmd_desc = {};
            cmd_desc.is_secondary = false;
            auto cmd = cgpu_create_command_buffer(get_cmd_pool(queue), &cmd_desc);
            cmds[queue] = cmd;
            cgpu_cmd_begin(cmd);
            return cmd;
        }
        ~TaskBatch()
        {
            for (auto& fence : fences)
            {
                cgpu_free_fence(fence.second);
            }
            for (auto& cmd : cmds)
            {
                cgpu_free_command_buffer(cmd.second);
            }
            for (auto& cmd_pool : cmd_pools)
            {
                cgpu_free_command_pool(cmd_pool.second);
            }
        }
    };
    eastl::vector_map<uint64_t, TaskBatch*> upload_batch_queue;
    eastl::vector_map<uint64_t, TaskBatch*> dstorage_batch_queue;
    struct AsyncThreadBatchService : public AsyncThreadedService
    {
        AsyncThreadBatchService(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
            : AsyncThreadedService(sleep_time, lockless)
        {

        }
        virtual void sleep_() SKR_NOEXCEPT override
        {
            AsyncThreadedService::sleep_();
            batch_id++;
        }
        uint64_t batch_id = 0;
    };
    AsyncThreadBatchService threaded_service;
    // CGPU Objects
    eastl::vector<CGPUUploadTask*> resource_uploads;
    eastl::vector<CGPUDStorageTask*> dstorage_uploads;
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
    if (auto ds_buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::DStorageBufferTask>(&task.resource_task))
    {
        SKR_ASSERT( (ds_buffer_task->buffer_io.path) && "buffer_io.path must be set");
        if (ds_buffer_task->buffer_io.path)
        {
            const auto& buffer_io = ds_buffer_task->buffer_io;
            auto ds_file = cgpu_dstorage_open_file(buffer_io.dstorage_queue, task.path.c_str());
            ds_buffer_task->dstorage_task = allocateCGPUDStorageTask(buffer_io.device, buffer_io.dstorage_queue, ds_file);
            CGPUDStorageFileInfo finfo = {};
            cgpu_dstorage_query_file_info(ds_buffer_task->dstorage_task->storage_queue, ds_buffer_task->dstorage_task->ds_file, &finfo);
            auto buffer_desc = make_zeroed<CGPUBufferDescriptor>();
            buffer_desc.size = finfo.file_size;
            buffer_desc.name = buffer_io.buffer_name;
            buffer_desc.descriptors = buffer_io.resource_types;
            buffer_desc.memory_usage = buffer_io.memory_usage;
            buffer_desc.format = buffer_io.format;
            buffer_desc.flags = buffer_io.flags;
            buffer_desc.start_state = buffer_io.start_state;
            buffer_desc.prefer_on_device = buffer_io.prefer_on_device;
            buffer_desc.prefer_on_host = buffer_io.prefer_on_host;
            auto buffer = cgpu_create_buffer(ds_buffer_task->buffer_io.device, &buffer_desc);
            // return resource object
            ds_buffer_task->buffer_request->out_buffer = buffer;
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
        SKR_ASSERT( task.step == kStepResourceCreated && "task.step must be kStepResourceCreated");

        const auto& buffer_io = buffer_task->buffer_io;
        const auto& buffer_request = buffer_task->buffer_request;
        CGPUUploadTask* upload = allocateCGPUUploadTask(buffer_io.device, buffer_io.transfer_queue, buffer_io.opt_semaphore);
        eastl::string name = buffer_io.buffer_name;
        name += "-upload";
        upload->upload_buffer = cgpux_create_mapped_upload_buffer(buffer_io.device, buffer_io.size, name.c_str());
        upload->dst_buffer = buffer_request->out_buffer;

        if (buffer_io.bytes)
        {
            memcpy((uint8_t*)upload->upload_buffer->cpu_mapped_address + buffer_io.offset, buffer_io.bytes, buffer_io.size);
        }
        
        auto cmd = task.task_batch->get_cmd(buffer_io.transfer_queue);
        
        if (buffer_io.bytes)
        {
            CGPUBufferToBufferTransfer vb_cpy = {};
            vb_cpy.dst = buffer_request->out_buffer;
            vb_cpy.dst_offset = 0;
            vb_cpy.src = upload->upload_buffer;
            vb_cpy.src_offset = 0;
            vb_cpy.size = buffer_io.size;
            cgpu_cmd_transfer_buffer_to_buffer(cmd, &vb_cpy);
        }
        auto buffer_barrier = make_zeroed<CGPUBufferBarrier>();
        buffer_barrier.buffer = buffer_request->out_buffer;
        buffer_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        buffer_barrier.dst_state = buffer_io.start_state;
        // acquire
        if (buffer_io.owner_queue->type != buffer_io.transfer_queue->type)
        {
            buffer_barrier.queue_release = true;
            buffer_barrier.queue_type = buffer_io.transfer_queue->type;
        }
        
        auto barrier = make_zeroed<CGPUResourceBarrierDescriptor>();
        barrier.buffer_barriers = &buffer_barrier;
        barrier.buffer_barriers_count = 1;
        cgpu_cmd_resource_barrier(cmd, &barrier);

        buffer_task->upload_task = upload;
        resource_uploads.emplace_back(upload);
    }
}
// upload resource


// dstorage resource
void skr::io::VRAMServiceImpl::dstorageResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    tryDStorageBufferResource(task);
}

void skr::io::VRAMServiceImpl::tryDStorageBufferResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto ds_buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::DStorageBufferTask>(&task.resource_task))
    {
        SKR_ASSERT( task.step == kStepResourceCreated && "task.step must be kStepResourceCreated");

        const auto& buffer_io = ds_buffer_task->buffer_io;
        const auto& buffer_request = ds_buffer_task->buffer_request;

    }
}
// dstorage resource

// status check
bool skr::io::VRAMServiceImpl::vramIOFinished(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
        SKR_ASSERT(buffer_task->upload_task != nullptr);
        auto status = cgpu_query_fence_status(task.task_batch->get_fence(buffer_task->buffer_io.transfer_queue));
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            buffer_task->upload_task->finished = true;
            return true;
        }
    }
    if (auto ds_buffer_task = eastl::get_if<skr::io::VRAMServiceImpl::DStorageBufferTask>(&task.resource_task))
    {
        return true;
    }
    return false;
}
// status check

// cgpu helpers
skr::io::VRAMServiceImpl::CGPUUploadTask* skr::io::VRAMServiceImpl::allocateCGPUUploadTask(CGPUDeviceId device, CGPUQueueId queue, CGPUSemaphoreId semaphore) SKR_NOEXCEPT
{
    auto upload = SkrNew<skr::io::VRAMServiceImpl::CGPUUploadTask>();
    upload->queue = queue;
    upload->semaphore = semaphore;
    return upload;
}

void skr::io::VRAMServiceImpl::freeCGPUUploadTask(skr::io::VRAMServiceImpl::CGPUUploadTask* upload) SKR_NOEXCEPT
{
    if (upload->upload_buffer) cgpu_free_buffer(upload->upload_buffer);
    SkrDelete(upload);
}

skr::io::VRAMServiceImpl::CGPUDStorageTask* skr::io::VRAMServiceImpl::allocateCGPUDStorageTask(CGPUDeviceId device, CGPUDStorageQueueId storage_queue, CGPUDStorageFileHandle file) SKR_NOEXCEPT
{
    auto dstorage = SkrNew<skr::io::VRAMServiceImpl::CGPUDStorageTask>();
    dstorage->storage_queue = storage_queue;
    dstorage->ds_file = file;
    return dstorage;
}

void skr::io::VRAMServiceImpl::freeCGPUDStorageTask(CGPUDStorageTask* task) SKR_NOEXCEPT
{
    SkrDelete(task);
}
// cgpu helpers

void __ioThreadTask_VRAM_execute(skr::io::VRAMServiceImpl* service)
{
    // 1.visit task
    service->tasks.update_(&service->threaded_service);
    auto upload_batch = SkrNew<skr::io::VRAMServiceImpl::TaskBatch>();
    auto dstorage_batch = SkrNew<skr::io::VRAMServiceImpl::TaskBatch>();
    upload_batch->id = service->threaded_service.batch_id;
    dstorage_batch->id = service->threaded_service.batch_id;
    while (auto iter = service->tasks.peek_())
    {
        if (!iter.has_value()) break;
        if (iter.value().isDStorage())
        {
            dstorage_batch->tasks.emplace_back(iter.value()).task_batch = dstorage_batch;
        }
        else
        {
            upload_batch->tasks.emplace_back(iter.value()).task_batch = upload_batch;
        }
    }
    if (!upload_batch->tasks.empty())
    {
        service->upload_batch_queue[upload_batch->id] = upload_batch;
    }
    if (!dstorage_batch->tasks.empty())
        service->dstorage_batch_queue[upload_batch->id] = dstorage_batch;
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
                if (task.isDStorage())
                {
                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_VRAM_LOADING);
                    service->dstorageResource(task);
                    task.step = kStepDirectStorage;
                }
                else
                {
                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_VRAM_LOADING);
                    service->uploadResource(task);
                    task.step = kStepUploading;
                }
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
                if (service->vramIOFinished(task))
                {
                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
                    task.step = kStepFinished;
                }
                else task.step = kStepDirectStorage; // continue uploading
                break;
            case kStepResourceBarrier:
                SKR_UNIMPLEMENTED_FUNCTION();
            case kStepFinished:
                return;
        }
    };
    for (auto [batch_id, batch] : service->upload_batch_queue)
    {
        auto earliest_step = kStepResourceCreated;
        auto latest_step = kStepResourceCreated;
        for (auto& task : batch->tasks)
        {
            foreach_task(task);
            latest_step = eastl::max(latest_step, task.step);
            earliest_step = eastl::min(latest_step, task.step);
        }
        if (latest_step == kStepUploading && earliest_step == kStepUploading && !batch->submitted)
        {
            for (auto&& [queue, cmd] : batch->cmds)
            {
                cgpu_cmd_end(cmd);
                CGPUQueueSubmitDescriptor submit_desc = {};
                submit_desc.cmds = &cmd;
                submit_desc.cmds_count = 1;
                // TODO: Additional semaphores
                submit_desc.signal_semaphore_count = 0;
                submit_desc.signal_semaphores = nullptr;
                submit_desc.signal_fence = batch->get_fence(queue);
                cgpu_submit_queue(queue, &submit_desc);
            }
        }
    }
    for (auto [batch_id, batch] : service->dstorage_batch_queue)
        for (auto& task : batch->tasks)
            foreach_task(task);
    // 2.sweep task batches
    eastl::for_each(service->upload_batch_queue.begin(), service->upload_batch_queue.end(),
        [](auto& batch){
            bool ready = batch.second->submitted;
            for (auto [queue, batch_fence] : batch.second->fences)
            {
                if (cgpu_query_fence_status(batch_fence) != CGPU_FENCE_STATUS_COMPLETE) ready = false;
            }
            if (ready)
            {
                SkrDelete(batch.second);
                batch.second = nullptr;
            }
        });
    service->upload_batch_queue.erase(
    eastl::remove_if(service->upload_batch_queue.begin(), service->upload_batch_queue.end(),
        [](auto& batch){
            return batch.second == nullptr;
        }), service->upload_batch_queue.end());

    eastl::for_each(service->dstorage_batch_queue.begin(), service->dstorage_batch_queue.end(),
        [](auto& batch){
            bool ready = batch.second->submitted;
            for (auto [queue, batch_fence] : batch.second->fences)
            {
                if (cgpu_query_fence_status(batch_fence) != CGPU_FENCE_STATUS_COMPLETE) ready = false;
            }
            if (ready)
            {
                SkrDelete(batch.second);
                batch.second = nullptr;
            }
        });
    service->dstorage_batch_queue.erase(
    eastl::remove_if(service->dstorage_batch_queue.begin(), service->dstorage_batch_queue.end(),
        [](auto& batch){
            return batch.second == nullptr;
        }), service->dstorage_batch_queue.end());
    // 3.sweep upload tasks
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
    eastl::string name = "ioVRAMServiceThread-";
    name.append(eastl::to_string(taskIndex++));
    tracy::SetThreadName(name.c_str());
#endif
    auto service = reinterpret_cast<skr::io::VRAMServiceImpl*>(arg);
    for (; service->threaded_service.getThreadStatus() != _SKR_IO_THREAD_STATUS_QUIT;)
    {
        if (service->threaded_service.getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND)
        {
            ZoneScopedN("ioVRAMServiceSuspend");
            for (; service->threaded_service.getThreadStatus() == _SKR_IO_THREAD_STATUS_SUSPEND;)
            {
            }
        }
        {
            ZoneScopedN("ioVRAMServiceWork");
            __ioThreadTask_VRAM_execute(service);
        }
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
    if (info->dstorage_queue)
    {
        io_task.resource_task = make_zeroed<DStorageBufferTask>();
        auto&& ds_buffer_task = eastl::get<DStorageBufferTask>(io_task.resource_task);
        ds_buffer_task.buffer_io = *info;
        ds_buffer_task.buffer_request = buffer_request;
    }
    else
    {
        io_task.resource_task = make_zeroed<BufferTask>();
        auto&& buffer_task = eastl::get<BufferTask>(io_task.resource_task);
        buffer_task.buffer_io = *info;
        buffer_task.buffer_request = buffer_request;
    }
    tasks.enqueue_(io_task, threaded_service.criticalTaskCount, name.c_str());
    threaded_service.request_();
}

skr::io::VRAMService* skr::io::VRAMService::create(const skr_vram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto service = SkrNew<skr::io::VRAMServiceImpl>(desc->sleep_time, desc->lockless);
    service->threaded_service.create_(desc->sleep_mode);
    service->threaded_service.sortMethod = desc->sort_method;
    service->threaded_service.sleepMode = desc->sleep_mode;
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