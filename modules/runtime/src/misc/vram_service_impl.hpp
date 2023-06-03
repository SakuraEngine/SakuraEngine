#pragma once
#include "cgpu/io.h"
#include "misc/make_zeroed.hpp"
#include "io_service_util.hpp"
#include <containers/string.hpp>
#include <containers/variant.hpp>
#include <EASTL/vector_map.h>

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

class VRAMServiceImpl final : public skr_io_vram_service_t
{
public:
    struct TaskBatch;
    struct CGPUUploadTask
    {
        CGPUQueueId queue = nullptr;
        CGPUSemaphoreId semaphore = nullptr;
        CGPUBufferId upload_buffer = nullptr;
        CGPUTextureId dst_texture = nullptr;
        bool finished = false;
    };
    struct BufferTask {
        skr_vram_buffer_io_t buffer_io;
        skr_async_vbuffer_destination_t* destination;
        CGPUUploadTask* upload_task;
    };
    struct TextureTask {
        skr_vram_texture_io_t texture_io;
        skr_async_vtexture_destination_t* destination;
        CGPUUploadTask* upload_task;
    };

    struct CGPUDStorageTask {
        CGPUDStorageQueueId storage_queue = nullptr;
        CGPUDStorageFileHandle ds_file = nullptr;
        uint64_t file_size;
        bool finished = false;
    };
    struct DStorageBufferTask
    {
        skr_vram_buffer_io_t buffer_io;
        skr_async_vbuffer_destination_t* destination;
        CGPUDStorageTask* dstorage_task;
    };
    struct DStorageTextureTask
    {
        skr_vram_texture_io_t texture_io;
        skr_async_vtexture_destination_t* destination;
        CGPUDStorageTask* dstorage_task;
    };

    struct Task : public TaskBase {
        skr::string path;
        skr::variant<BufferTask, DStorageBufferTask, TextureTask, DStorageTextureTask> resource_task;
        EVramTaskStep step;
        TaskBatch* task_batch;
        bool isDStorage() const
        {
            return skr::get_if<DStorageBufferTask>(&resource_task) || skr::get_if<DStorageTextureTask>(&resource_task);
        }
    };
    ~VRAMServiceImpl() SKR_NOEXCEPT = default;
    VRAMServiceImpl(uint32_t sleep_time, bool lockless) SKR_NOEXCEPT
        : tasks(lockless), threaded_service(sleep_time, lockless)

    {
    }
    void request(const skr_vram_buffer_io_t* info, skr_io_future_t* async_request, skr_async_vbuffer_destination_t* buffer_request) SKR_NOEXCEPT final;
    void request(const skr_vram_texture_io_t* info, skr_io_future_t* async_request, skr_async_vtexture_destination_t* texture_request) SKR_NOEXCEPT final;
    bool try_cancel(skr_io_future_t* request) SKR_NOEXCEPT final;
    void defer_cancel(skr_io_future_t* request) SKR_NOEXCEPT final;
    void drain() SKR_NOEXCEPT final;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT final;
    void stop(bool wait_drain = false) SKR_NOEXCEPT final;
    void run() SKR_NOEXCEPT final;

    SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT final
    {
        return threaded_service.getServiceStatus();
    }

    // create resource
    CGPUBufferId createCGPUBuffer(const skr_vram_buffer_io_t& buffer_io, uint64_t backfill_size) SKR_NOEXCEPT;
    CGPUTextureId createCGPUTexture(const skr_vram_texture_io_t& texture_io) SKR_NOEXCEPT;

    void createResource(Task& task) SKR_NOEXCEPT;
    void tryCreateBufferResource(Task& task) SKR_NOEXCEPT;
    void tryCreateTextureResource(Task& task) SKR_NOEXCEPT;
    // create resource

    // upload resource
    void uploadResource(Task& task) SKR_NOEXCEPT;
    void tryUploadBufferResource(Task& task) SKR_NOEXCEPT;
    void tryUploadTextureResource(Task& task) SKR_NOEXCEPT;
    // upload resource

    // dstorage resource
    void dstorageResource(Task& task) SKR_NOEXCEPT;
    void tryDStorageBufferResource(Task& task) SKR_NOEXCEPT;
    void tryDStorageTextureResource(Task& task) SKR_NOEXCEPT;
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

    const skr::string name;
    // task containers
    TaskContainer<Task> tasks;
    struct TaskBatch
    {
        // Tasks
        eastl::vector<Task> tasks;
        // Upload Resources
        eastl::vector<CGPUBufferBarrier> buffer_barriers;
        eastl::vector<CGPUTextureBarrier> texture_barriers;
        eastl::vector_map<CGPUQueueId, CGPUFenceId> fences;
        eastl::vector_map<CGPUQueueId, CGPUCommandPoolId> cmd_pools;
        eastl::vector_map<CGPUQueueId, CGPUCommandBufferId> cmds;
        // DStorage Resources
        eastl::vector_map<CGPUDStorageQueueId, CGPUFenceId> ds_fences;
        bool submitted = false;
        uint64_t id;

        CGPUFenceId get_fence(CGPUDStorageQueueId queue) SKR_NOEXCEPT
        {
            auto it = ds_fences.find(queue);
            if (it != ds_fences.end()) 
                return it->second;
            else
            {
                ZoneScopedN("CreateFence(DStorage)");
                auto fence = cgpu_create_fence(queue->device);
                ds_fences[queue] = fence;
                return fence;
            }
        }
        CGPUFenceId get_fence(CGPUQueueId queue) SKR_NOEXCEPT
        {
            auto it = fences.find(queue);
            if (it != fences.end()) 
                return it->second;
            else
            {
                ZoneScopedN("CreateFence(QueueUpload)");
                auto fence = cgpu_create_fence(queue->device);
                fences[queue] = fence;
                return fence;
            }
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
            else
            {
                ZoneScopedN("CreateCommandBuffer");
                CGPUCommandBufferDescriptor cmd_desc = {};
                cmd_desc.is_secondary = false;
                auto cmd = cgpu_create_command_buffer(get_cmd_pool(queue), &cmd_desc);
                cmds[queue] = cmd;
                {
                    ZoneScopedN("Begin");
                    cgpu_cmd_begin(cmd);
                }
                return cmd;
            }
        }
        ~TaskBatch()
        {
            uint32_t ds_fence_c = (uint32_t)ds_fences.size();
            uint32_t ds_idx = 0;
            for (auto& fence : ds_fences)
            {
                ds_idx++;
                cgpu_free_fence(fence.second);
            }
            SKR_ASSERT(ds_idx == ds_fence_c);
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
    void foreach_batch(const eastl::function<void(eastl::pair<uint64_t, TaskBatch*>&)>& func) SKR_NOEXCEPT
    {
        eastl::for_each(upload_batch_queue.begin(), upload_batch_queue.end(), func);
        eastl::for_each(dstorage_batch_queue.begin(), dstorage_batch_queue.end(), func);
    }
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
        }
    };
    AsyncThreadBatchService threaded_service;
    // CGPU Objects
    eastl::vector<CGPUUploadTask*> resource_uploads;
    eastl::vector<CGPUDStorageTask*> dstorage_uploads;
};

}
}