#include "vram_service_impl.hpp"

// create resource
void skr::io::VRAMServiceImpl::createResource(skr::io::VRAMServiceImpl::Task &task) SKR_NOEXCEPT
{
    tryCreateBufferResource(task);
    tryCreateTextureResource(task);
}

CGPUBufferId skr::io::VRAMServiceImpl::createCGPUBuffer(const skr_vram_buffer_io_t& buffer_io, uint64_t backfill_size) SKR_NOEXCEPT
{
    auto buffer_desc = make_zeroed<CGPUBufferDescriptor>();
    buffer_desc.size = buffer_io.vbuffer.buffer_size ? buffer_io.vbuffer.buffer_size : backfill_size;
    buffer_desc.name = buffer_io.vbuffer.buffer_name;
    buffer_desc.descriptors = buffer_io.vbuffer.resource_types;
    buffer_desc.memory_usage = buffer_io.vbuffer.memory_usage;
    buffer_desc.flags = buffer_io.vbuffer.flags;
    buffer_desc.prefer_on_device = buffer_io.vbuffer.prefer_on_device;
    buffer_desc.prefer_on_host = buffer_io.vbuffer.prefer_on_host;

    buffer_desc.owner_queue = buffer_io.transfer_queue;
    buffer_desc.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    
    auto buffer = cgpu_create_buffer(buffer_io.device, &buffer_desc);
    return buffer;
}

CGPUTextureId skr::io::VRAMServiceImpl::createCGPUTexture(const skr_vram_texture_io_t &texture_io) SKR_NOEXCEPT
{
    auto texture_desc = make_zeroed<CGPUTextureDescriptor>();
    texture_desc.width = texture_io.vtexture.width;
    texture_desc.height = texture_io.vtexture.height;
    texture_desc.depth = texture_io.vtexture.depth;
    texture_desc.name = texture_io.vtexture.texture_name;
    texture_desc.descriptors = texture_io.vtexture.resource_types;
    texture_desc.flags = texture_io.vtexture.flags;
    texture_desc.format = texture_io.vtexture.format;

    texture_desc.owner_queue = texture_io.transfer_queue;
    texture_desc.start_state = CGPU_RESOURCE_STATE_COPY_DEST;

    auto texture = cgpu_create_texture(texture_io.device, &texture_desc);
    return texture;
}

void skr::io::VRAMServiceImpl::tryCreateBufferResource(skr::io::VRAMServiceImpl::Task &task) SKR_NOEXCEPT
{
    if (auto buffer_task = skr::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
        SKR_ASSERT( (buffer_task->buffer_io.src_memory.size) && "buffer_io.size must be set");
        const auto& buffer_io = buffer_task->buffer_io;
        if (buffer_task->destination->buffer) return;

        if (buffer_task->buffer_io.src_memory.size)
        {
            ZoneScopedN("CreateBufferResource");
            // return resource object
            buffer_task->destination->buffer = createCGPUBuffer(buffer_io, buffer_task->buffer_io.src_memory.size);
        }
        else //if (buffer_task->buffer_io.path)
        {
            SKR_UNREACHABLE_CODE();
        }
    }
    if (auto ds_buffer_task = skr::get_if<skr::io::VRAMServiceImpl::DStorageBufferTask>(&task.resource_task))
    {
        const auto& buffer_io = ds_buffer_task->buffer_io;
        const auto& destination = ds_buffer_task->destination;
        if (ds_buffer_task->buffer_io.dstorage.source_type == CGPU_DSTORAGE_SOURCE_FILE)
        {
            SKR_ASSERT( (ds_buffer_task->buffer_io.dstorage.path) && "buffer_io.path must be set");
            auto ds_file = cgpu_dstorage_open_file(buffer_io.dstorage.queue, task.path.c_str());
            ds_buffer_task->dstorage_task = allocateCGPUDStorageTask(buffer_io.device, buffer_io.dstorage.queue, ds_file);
            if (destination->buffer) 
            {
                ds_buffer_task->destination->buffer = destination->buffer;
            }
            else
            {
                ZoneScopedN("CreateBufferResource");
                // return resource object
                ds_buffer_task->destination->buffer = createCGPUBuffer(buffer_io, ds_buffer_task->dstorage_task->file_size);
            }
        }
        else // SOURCE_MEMORY
        {
            SKR_ASSERT( (ds_buffer_task->buffer_io.src_memory.bytes) && "buffer_io.bytes must be set");
            SKR_ASSERT( (ds_buffer_task->buffer_io.src_memory.size) && "buffer_io.size must be set");

            const auto& buffer_io = ds_buffer_task->buffer_io;
            const auto& destination = ds_buffer_task->destination;
            ds_buffer_task->dstorage_task =
                allocateCGPUDStorageTask(buffer_io.device, buffer_io.dstorage.queue, nullptr);
            if (destination->buffer) 
            {
                ds_buffer_task->destination->buffer = destination->buffer;
            }
            else
            {
                SKR_ASSERT( (ds_buffer_task->buffer_io.vbuffer.buffer_size) && "buffer_io.buffer_size must be set");
                ZoneScopedN("CreateBufferResource");
                // return resource object
                ds_buffer_task->destination->buffer = createCGPUBuffer(buffer_io, buffer_io.vbuffer.buffer_size);
            }
        }
    }
}

void skr::io::VRAMServiceImpl::tryCreateTextureResource(skr::io::VRAMServiceImpl::Task &task) SKR_NOEXCEPT
{
    if (auto texture_task = skr::get_if<skr::io::VRAMServiceImpl::TextureTask>(&task.resource_task))
    {
        if (texture_task->destination->texture) return;
        
        SKR_ASSERT( (texture_task->texture_io.src_memory.size) && "texture_io.size must be set");
        if (texture_task->texture_io.src_memory.size)
        {
            ZoneScopedN("CreateTextureResource");

            const auto& texture_io = texture_task->texture_io;
            // return resource object
            texture_task->destination->texture = createCGPUTexture(texture_io);
        }
        else // if (buffer_task->texture_io.path)
        {
            SKR_UNREACHABLE_CODE();
        }
    }
    if (auto ds_texture_task = skr::get_if<skr::io::VRAMServiceImpl::DStorageTextureTask>(&task.resource_task))
    {
        if (ds_texture_task->texture_io.dstorage.source_type == CGPU_DSTORAGE_SOURCE_FILE)
        {
            ZoneScopedN("CreateTextureResource");

            SKR_ASSERT( (ds_texture_task->texture_io.dstorage.path) && "texture_io.path must be set");
            const auto& texture_io = ds_texture_task->texture_io;
            const auto& destination = ds_texture_task->destination;
            auto ds_file = cgpu_dstorage_open_file(texture_io.dstorage.queue, task.path.c_str());
            ds_texture_task->dstorage_task = allocateCGPUDStorageTask(texture_io.device, texture_io.dstorage.queue, ds_file);
            if (destination->texture) 
            {
                ds_texture_task->destination->texture = destination->texture;
            }
            else
            {
                // return resource object
                ds_texture_task->destination->texture = createCGPUTexture(texture_io);
            }
        }
        else 
        {
            ZoneScopedN("CreateTextureResource");

            SKR_ASSERT( (ds_texture_task->texture_io.vtexture.width) && "buffer_io.buffer_size must be set");
            SKR_ASSERT( (ds_texture_task->texture_io.src_memory.bytes) && "buffer_io.bytes must be set");
            SKR_ASSERT( (ds_texture_task->texture_io.src_memory.size) && "buffer_io.size must be set");

            const auto& texture_io = ds_texture_task->texture_io;
            const auto& destination = ds_texture_task->destination;
            ds_texture_task->dstorage_task = allocateCGPUDStorageTask(texture_io.device, texture_io.dstorage.queue, nullptr);
            if (destination->texture) 
            {
                ds_texture_task->destination->texture = destination->texture;
            }
            else
            {
                // return resource object
                ds_texture_task->destination->texture = createCGPUTexture(texture_io);
            }
        }
    }
}
// create resource

// upload resource
void skr::io::VRAMServiceImpl::uploadResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    tryUploadBufferResource(task);
    tryUploadTextureResource(task);
}

void skr::io::VRAMServiceImpl::tryUploadBufferResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto buffer_task = skr::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
    #ifdef TRACY_ENABLE
        const auto BufferUpload = "BufferUpload-" + task.path;
        TracyMessage(BufferUpload.c_str(), BufferUpload.size());
    #endif
        SKR_ASSERT( task.step == kStepResourceCreated && "task.step must be kStepResourceCreated");

        const auto& buffer_io = buffer_task->buffer_io;
        const auto& destination = buffer_task->destination;
        CGPUUploadTask* upload = allocateCGPUUploadTask(buffer_io.device, buffer_io.transfer_queue, buffer_io.opt_semaphore);
        skr::string name = buffer_io.vbuffer.buffer_name ? buffer_io.vbuffer.buffer_name : "";
        name += "-upload";
        upload->upload_buffer = cgpux_create_mapped_upload_buffer(buffer_io.device, buffer_io.src_memory.size, name.c_str());

        if (buffer_io.src_memory.bytes)
        {
            memcpy((uint8_t*)upload->upload_buffer->cpu_mapped_address, 
                buffer_io.src_memory.bytes, buffer_io.src_memory.size);
        }
        
        auto cmd = task.task_batch->get_cmd(buffer_io.transfer_queue);
        
        if (buffer_io.src_memory.bytes)
        {
            CGPUBufferToBufferTransfer vb_cpy = {};
            vb_cpy.dst = destination->buffer;
            vb_cpy.dst_offset = buffer_io.vbuffer.offset;
            vb_cpy.src = upload->upload_buffer;
            vb_cpy.src_offset = 0;
            vb_cpy.size = buffer_io.src_memory.size;
            cgpu_cmd_transfer_buffer_to_buffer(cmd, &vb_cpy);
        }
        auto buffer_barrier = make_zeroed<CGPUBufferBarrier>();
        buffer_barrier.buffer = destination->buffer;
        buffer_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        buffer_barrier.dst_state = CGPU_RESOURCE_STATE_COMMON;
        // release
        if (buffer_io.transfer_queue->type == CGPU_QUEUE_TYPE_TRANSFER)
        {
            buffer_barrier.queue_release = true;
            buffer_barrier.queue_type = buffer_io.transfer_queue->type;
        }
        
        task.task_batch->buffer_barriers.emplace_back(buffer_barrier);

        buffer_task->upload_task = upload;
    }
}

void skr::io::VRAMServiceImpl::tryUploadTextureResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto texture_task = skr::get_if<skr::io::VRAMServiceImpl::TextureTask>(&task.resource_task))
    {
    #ifdef TRACY_ENABLE
        const auto TextureUpload = "TextureUpload-" + task.path;
        TracyMessage(TextureUpload.c_str(), TextureUpload.size());
    #endif
        SKR_ASSERT( task.step == kStepResourceCreated && "task.step must be kStepResourceCreated");

        const auto& texture_io = texture_task->texture_io;
        const auto& destination = texture_task->destination;
        CGPUUploadTask* upload = allocateCGPUUploadTask(texture_io.device, texture_io.transfer_queue, texture_io.opt_semaphore);
        skr::string name = texture_io.vtexture.texture_name ? texture_io.vtexture.texture_name : "";
        name += "-upload";
        upload->upload_buffer = cgpux_create_mapped_upload_buffer(texture_io.device, texture_io.src_memory.size, name.c_str());

        if (texture_io.src_memory.bytes)
        {
            memcpy((uint8_t*)upload->upload_buffer->cpu_mapped_address, texture_io.src_memory.bytes, texture_io.src_memory.size);
        }
        
        auto cmd = task.task_batch->get_cmd(texture_io.transfer_queue);
        
        if (texture_io.src_memory.bytes)
        {
            CGPUBufferToTextureTransfer tex_cpy = {};
            tex_cpy.dst = destination->texture;
            tex_cpy.dst_subresource.aspects = CGPU_TVA_COLOR;
            // TODO: texture array & mips
            tex_cpy.dst_subresource.base_array_layer = 0;
            tex_cpy.dst_subresource.layer_count = 1;
            tex_cpy.dst_subresource.mip_level = 0;
            tex_cpy.src = upload->upload_buffer;
            tex_cpy.src_offset = 0;
            cgpu_cmd_transfer_buffer_to_texture(cmd, &tex_cpy);
        }
        auto texture_barrier = make_zeroed<CGPUTextureBarrier>();
        texture_barrier.texture = destination->texture;
        texture_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        texture_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
        // release
        if (texture_io.transfer_queue->type == CGPU_QUEUE_TYPE_TRANSFER)
        {
            texture_barrier.queue_release = true;
            texture_barrier.queue_type = texture_io.transfer_queue->type;
        }
        
        task.task_batch->texture_barriers.emplace_back(texture_barrier);

        texture_task->upload_task = upload;
    }
}
// upload resource


// dstorage resource
void skr::io::VRAMServiceImpl::dstorageResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    tryDStorageBufferResource(task);
    tryDStorageTextureResource(task);
}

void skr::io::VRAMServiceImpl::tryDStorageBufferResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto ds_buffer_task = skr::get_if<skr::io::VRAMServiceImpl::DStorageBufferTask>(&task.resource_task))
    {
    #ifdef TRACY_ENABLE
        const auto BufferDStorage = "BufferDStorage-" + task.path;
        TracyMessage(BufferDStorage.c_str(), BufferDStorage.size());
    #endif
        SKR_ASSERT( task.step == kStepResourceCreated && "task.step must be kStepResourceCreated");

        const auto& buffer_io = ds_buffer_task->buffer_io;
        const auto& destination = ds_buffer_task->destination;

        CGPUDStorageBufferIODescriptor io_desc = {};
        io_desc.source_type = buffer_io.dstorage.source_type;
        if (io_desc.source_type == CGPU_DSTORAGE_SOURCE_FILE)
        {
            io_desc.source_file.file = ds_buffer_task->dstorage_task->ds_file;
            io_desc.source_file.offset = 0u;
            io_desc.source_file.size = ds_buffer_task->dstorage_task->file_size;
        }
        else
        {
            io_desc.source_memory.bytes = buffer_io.src_memory.bytes;
            io_desc.source_memory.bytes_size = buffer_io.src_memory.size;
        }
        io_desc.buffer = destination->buffer;
        io_desc.offset = buffer_io.vbuffer.offset;

        io_desc.compression = buffer_io.dstorage.compression;
        io_desc.uncompressed_size = buffer_io.dstorage.uncompressed_size;

        io_desc.name = buffer_io.vbuffer.buffer_name;
        cgpu_dstorage_enqueue_buffer_request(ds_buffer_task->dstorage_task->storage_queue, &io_desc);

        if (auto fence = task.task_batch->get_fence(ds_buffer_task->dstorage_task->storage_queue));
        else SKR_UNREACHABLE_CODE();
    }
}

void skr::io::VRAMServiceImpl::tryDStorageTextureResource(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto ds_texture_task = skr::get_if<skr::io::VRAMServiceImpl::DStorageTextureTask>(&task.resource_task))
    {
    #ifdef TRACY_ENABLE
        const auto TextureDStorage = "TextureDStorage-" + task.path;
        TracyMessage(TextureDStorage.c_str(), TextureDStorage.size());
    #endif
        SKR_ASSERT( task.step == kStepResourceCreated && "task.step must be kStepResourceCreated");

        const auto& texture_io = ds_texture_task->texture_io;
        const auto& destination = ds_texture_task->destination;

        CGPUDStorageTextureIODescriptor io_desc = {};
        io_desc.source_type = texture_io.dstorage.source_type;
        if (io_desc.source_type == CGPU_DSTORAGE_SOURCE_FILE)
        {
            io_desc.source_file.file = ds_texture_task->dstorage_task->ds_file;
            io_desc.source_file.offset = 0u;
            io_desc.source_file.size = ds_texture_task->dstorage_task->file_size;
            io_desc.width = texture_io.vtexture.width;
            io_desc.height = texture_io.vtexture.height;
            io_desc.depth = texture_io.vtexture.depth;
        }
        else
        {
            io_desc.source_memory.bytes = texture_io.src_memory.bytes;
            io_desc.source_memory.bytes_size = texture_io.src_memory.size;
            io_desc.width = texture_io.vtexture.width;
            io_desc.height = texture_io.vtexture.height;
            io_desc.depth = texture_io.vtexture.depth;
        }
        io_desc.name = texture_io.vtexture.texture_name;
        io_desc.texture = destination->texture;

        io_desc.compression = texture_io.dstorage.compression;
        io_desc.uncompressed_size = texture_io.dstorage.uncompressed_size;

        cgpu_dstorage_enqueue_texture_request(ds_texture_task->dstorage_task->storage_queue, &io_desc);

        if (auto fence = task.task_batch->get_fence(ds_texture_task->dstorage_task->storage_queue));
        else SKR_UNREACHABLE_CODE();
    }
}
// dstorage resource

// status check
bool skr::io::VRAMServiceImpl::vramIOFinished(skr::io::VRAMServiceImpl::Task& task) SKR_NOEXCEPT
{
    if (auto buffer_task = skr::get_if<skr::io::VRAMServiceImpl::BufferTask>(&task.resource_task))
    {
        SKR_ASSERT(buffer_task->upload_task != nullptr);
        auto status = cgpu_query_fence_status(task.task_batch->get_fence(buffer_task->buffer_io.transfer_queue));
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            buffer_task->upload_task->finished = true;
            return true;
        }
    }
    if (auto ds_buffer_task = skr::get_if<skr::io::VRAMServiceImpl::DStorageBufferTask>(&task.resource_task))
    {
        auto status = cgpu_query_fence_status(task.task_batch->get_fence(ds_buffer_task->dstorage_task->storage_queue));
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            ds_buffer_task->dstorage_task->finished = true;
            return true;
        }
    }
    if (auto texture_task = skr::get_if<skr::io::VRAMServiceImpl::TextureTask>(&task.resource_task))
    {
        SKR_ASSERT(texture_task->upload_task != nullptr);
        auto status = cgpu_query_fence_status(task.task_batch->get_fence(texture_task->texture_io.transfer_queue));
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            texture_task->upload_task->finished = true;
            return true;
        }
    }
    if (auto ds_texture_task = skr::get_if<skr::io::VRAMServiceImpl::DStorageTextureTask>(&task.resource_task))
    {
        auto status = cgpu_query_fence_status(task.task_batch->get_fence(ds_texture_task->dstorage_task->storage_queue));
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            ds_texture_task->dstorage_task->finished = true;
            return true;
        }
    }
    return false;
}
// status check

// cgpu helpers
skr::io::VRAMServiceImpl::CGPUUploadTask* skr::io::VRAMServiceImpl::allocateCGPUUploadTask(CGPUDeviceId device, CGPUQueueId queue, CGPUSemaphoreId semaphore) SKR_NOEXCEPT
{
    ZoneScopedN("AllocateCGPUUploadTask");

    auto upload = SkrNew<skr::io::VRAMServiceImpl::CGPUUploadTask>();
    upload->queue = queue;
    upload->semaphore = semaphore;
    resource_uploads.emplace_back(upload);
    return upload;
}

void skr::io::VRAMServiceImpl::freeCGPUUploadTask(skr::io::VRAMServiceImpl::CGPUUploadTask* upload) SKR_NOEXCEPT
{
    if (upload->upload_buffer) cgpu_free_buffer(upload->upload_buffer);
    SkrDelete(upload);
}

skr::io::VRAMServiceImpl::CGPUDStorageTask* skr::io::VRAMServiceImpl::allocateCGPUDStorageTask(CGPUDeviceId device, CGPUDStorageQueueId storage_queue, CGPUDStorageFileHandle file) SKR_NOEXCEPT
{
    ZoneScopedN("AllocateCGPUDStorageTask");

    auto dstorage = SkrNew<skr::io::VRAMServiceImpl::CGPUDStorageTask>();
    dstorage->storage_queue = storage_queue;
    if (file)
    {
        dstorage->ds_file = file;
        CGPUDStorageFileInfo finfo = {};
        cgpu_dstorage_query_file_info(dstorage->storage_queue, dstorage->ds_file, &finfo);
        dstorage->file_size = finfo.file_size;
    }
    dstorage_uploads.emplace_back(dstorage);
    return dstorage;
}

void skr::io::VRAMServiceImpl::freeCGPUDStorageTask(CGPUDStorageTask* task) SKR_NOEXCEPT
{
    if(task->ds_file) cgpu_dstorage_close_file(task->storage_queue, task->ds_file);
    SkrDelete(task);
}
// cgpu helpers

void __ioThreadTask_VRAM_execute(skr::io::VRAMServiceImpl* service)
{
    using namespace skr::io;
    // 0.update task 
    {
        service->tasks.update_(&service->threaded_service);
    }
    // 1.visit task
    auto foreach_task = [service](auto& task)
    {
        const auto vramIOStep = task.step;
        switch (vramIOStep)
        {
            case kStepNone: // start create resource
                {
                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_CREATING_RESOURCE);
                    service->createResource(task);
                    task.step = kStepResourceCreated;
                }
                break;
            case kStepResourceCreated: // start uploading
                if (task.isDStorage())
                {
                    ZoneScopedN("PrepareDStorage");

                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_VRAM_LOADING);
                    service->dstorageResource(task);
                    task.step = kStepDirectStorage;
                }
                else
                {
                    ZoneScopedN("PrepareCpyCmd");

                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_VRAM_LOADING);
                    service->uploadResource(task);
                    task.step = kStepUploading;
                }
                break;
            case kStepUploading:
                if (service->vramIOFinished(task))
                {
                    ZoneScopedN("CpyQueueSignalOK");

                    task.setTaskStatus(SKR_ASYNC_IO_STATUS_OK);
                    task.step = kStepFinished;
                }
                else task.step = kStepUploading; // continue uploading
                break;
            case kStepDirectStorage:
                if (service->vramIOFinished(task))
                {
                    ZoneScopedN("DStorageSignalOK");

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
    {
        ZoneScopedN("ioVRAMServiceWork");

        {
            static uint64_t upload_batch_id = 0;
            static uint64_t dstorage_batch_id = 0;
            auto upload_batch = SkrNew<skr::io::VRAMServiceImpl::TaskBatch>();
            auto dstorage_batch = SkrNew<skr::io::VRAMServiceImpl::TaskBatch>();
            upload_batch->id = upload_batch_id;
            dstorage_batch->id = dstorage_batch_id;

            while (auto iter = service->tasks.peek_())
            {
                if (!iter.has_value()) break;
                if (iter.value().isDStorage())
                    dstorage_batch->tasks.emplace_back(iter.value()).task_batch = dstorage_batch;
                else
                    upload_batch->tasks.emplace_back(iter.value()).task_batch = upload_batch;
            }
            
            if (!upload_batch->tasks.empty())
            {
                ZoneScopedN("LogCreateBatch(Upload)");

                SKR_LOG_TRACE("Created Upload Batch %d with %d Tasks", dstorage_batch->id, dstorage_batch->tasks.size());
                SKR_ASSERT(service->upload_batch_queue.find(upload_batch->id) == service->upload_batch_queue.end());
                service->upload_batch_queue[upload_batch->id] = upload_batch;
            }
            else
                SkrDelete(upload_batch);
                
            if (!dstorage_batch->tasks.empty())
            {
                ZoneScopedN("LogCreateBatch(DStorage)");

                SKR_LOG_TRACE("Created DirectStorage Batch %d with %d Tasks", dstorage_batch->id, dstorage_batch->tasks.size());
                SKR_ASSERT(service->dstorage_batch_queue.find(dstorage_batch->id) == service->dstorage_batch_queue.end());
                service->dstorage_batch_queue[dstorage_batch->id] = dstorage_batch;
            }
            else
                SkrDelete(dstorage_batch);

            upload_batch_id++;
            dstorage_batch_id++;
        }

        // upload cmds
        for (auto [batch_id, batch] : service->upload_batch_queue)
        {
            auto earliest_step = kStepResourceCreated;
            auto latest_step = kStepResourceCreated;
            for (auto& task : batch->tasks)
            {
                foreach_task(task);
                foreach_task(task); // call this twice to skip create_resource state to avoid idling
                latest_step = eastl::max(latest_step, task.step);
                earliest_step = eastl::min(latest_step, task.step);
            }
            if (latest_step == kStepUploading && earliest_step == kStepUploading && !batch->submitted)
            {
                for (auto&& [queue, cmd] : batch->cmds)
                {
                    auto barrier = make_zeroed<CGPUResourceBarrierDescriptor>();
                    barrier.buffer_barriers = batch->buffer_barriers.data();
                    barrier.buffer_barriers_count = (uint32_t)batch->buffer_barriers.size();
                    barrier.texture_barriers = batch->texture_barriers.data();
                    barrier.texture_barriers_count = (uint32_t)batch->texture_barriers.size();
                    cgpu_cmd_resource_barrier(cmd, &barrier);

                    cgpu_cmd_end(cmd);
                    CGPUQueueSubmitDescriptor submit_desc = {};
                    submit_desc.cmds = &cmd;
                    submit_desc.cmds_count = 1;
                    // TODO: Additional semaphores
                    submit_desc.signal_semaphore_count = 0;
                    submit_desc.signal_semaphores = nullptr;
                    submit_desc.signal_fence = batch->get_fence(queue);
                    cgpu_submit_queue(queue, &submit_desc);
                    batch->submitted = true;
                }

                if (batch->submitted)
                {
                    SKR_LOG_TRACE("Submit Upload Batch %d with %d Tasks", batch->id, batch->tasks.size());
                }
            }
        }
        // dstorage queue requests
        for (auto [batch_id, batch] : service->dstorage_batch_queue)
        {
            auto earliest_step = kStepResourceCreated;
            auto latest_step = kStepResourceCreated;
            for (auto& task : batch->tasks)
            {
                foreach_task(task);
                foreach_task(task); // call this twice to skip create_resource state to avoid idling
                latest_step = eastl::max(latest_step, task.step);
                earliest_step = eastl::min(latest_step, task.step);
            }
            if (latest_step == kStepDirectStorage && earliest_step == kStepDirectStorage && !batch->submitted)
            {
                ZoneScopedN("SubmitDStorageQueue");

                for (auto&& [queue, fence] : batch->ds_fences)
                {
                    ZoneScopedN("Submit");

                    auto _batch = batch;
                    cgpu_dstorage_queue_submit(queue, _batch->get_fence(queue));
                    _batch->submitted = true;

                }

                if (batch->submitted)
                {
                    ZoneScopedN("LogSubmit");

                    SKR_LOG_TRACE("Submit DirectStorage Batch %d with %d Tasks", batch->id, batch->tasks.size());
                }
            }
        }
    }
    // 2.sweep up
    {
        ZoneScopedN("ioVRAMServiceSweep");
    
        // 2.1 sweep task batches
        service->foreach_batch([foreach_task](auto& batch){
            bool ready = batch.second->submitted;
            for (auto [queue, batch_fence] : batch.second->fences)
            {
                if (cgpu_query_fence_status(batch_fence) != CGPU_FENCE_STATUS_COMPLETE) ready = false;
            }
            for (auto [ds_queue, batch_fence] : batch.second->ds_fences)
            {
                if (cgpu_query_fence_status(batch_fence) != CGPU_FENCE_STATUS_COMPLETE) ready = false;
            }
            if (ready)
            {
                for (auto& task : batch.second->tasks)
                {
                    foreach_task(task);
                    SKR_ASSERT(task.step == kStepFinished);
                }
                SKR_LOG_TRACE("Delete DirectStorage Batch %d with %d Tasks", batch.second->id, batch.second->tasks.size());
                SkrDelete(batch.second);
                batch.second = nullptr;
            }
        });
        service->upload_batch_queue.erase(
        eastl::remove_if(service->upload_batch_queue.begin(), service->upload_batch_queue.end(),
            [](auto& batch){
                if (batch.second == nullptr)
                    SKR_LOG_TRACE("Remove Upload Batch %d", batch.first);
                return batch.second == nullptr;
            }), service->upload_batch_queue.end());
        service->dstorage_batch_queue.erase(
        eastl::remove_if(service->dstorage_batch_queue.begin(), service->dstorage_batch_queue.end(),
            [](auto& batch){
                if (batch.second == nullptr)
                    SKR_LOG_TRACE("Remove DirectStorage Batch %d", batch.first);
                return batch.second == nullptr;
            }), service->dstorage_batch_queue.end());
        // 2.2 sweep upload tasks
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
        // 2.3 sweep dstorage tasks
        eastl::for_each(service->dstorage_uploads.begin(), service->dstorage_uploads.end(),
            [service](auto& dstorage){
                if (dstorage->finished)
                {
                    service->freeCGPUDStorageTask(dstorage);
                    dstorage = nullptr;
                }
            });
        service->dstorage_uploads.erase(
            eastl::remove_if(service->dstorage_uploads.begin(), service->dstorage_uploads.end(),
            [](auto& dstorage){
                return dstorage == nullptr;
            }), service->dstorage_uploads.end());
    }
}

void __ioThreadTask_VRAM(void* arg)
{
    using namespace skr::io;
#ifdef TRACY_ENABLE
    static uint32_t taskIndex = 0;
    skr::string name = "ioVRAMServiceThread-";
    name.append(skr::to_string(taskIndex++));
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
        __ioThreadTask_VRAM_execute(service);
    }
}

void skr::io::VRAMServiceImpl::request(const skr_vram_buffer_io_t* buffer_info, skr_async_request_t* async_request, skr_async_vbuffer_destination_t* destination) SKR_NOEXCEPT
{
    // try push back new request
    auto io_task = make_zeroed<Task>();
    io_task.priority = buffer_info->priority;
    io_task.sub_priority = buffer_info->sub_priority;
    io_task.request = async_request;
    for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
    {
        io_task.callbacks[i] = buffer_info->callbacks[i];
        io_task.callback_datas[i] = buffer_info->callback_datas[i];
    }
    io_task.path = buffer_info->dstorage.path ? buffer_info->dstorage.path : "";
    if (buffer_info->dstorage.queue)
    {
        io_task.resource_task = make_zeroed<DStorageBufferTask>();
        auto&& ds_buffer_task = skr::get<DStorageBufferTask>(io_task.resource_task);
        ds_buffer_task.buffer_io = *buffer_info;
        ds_buffer_task.destination = destination;
    }
    else
    {
        io_task.resource_task = make_zeroed<BufferTask>();
        auto&& buffer_task = skr::get<BufferTask>(io_task.resource_task);
        buffer_task.buffer_io = *buffer_info;
        buffer_task.destination = destination;
    }
    tasks.enqueue_(io_task, threaded_service.criticalTaskCount, name.c_str());
    threaded_service.request_();
}

void skr::io::VRAMServiceImpl::request(const skr_vram_texture_io_t* texture_info, skr_async_request_t* async_request, skr_async_vtexture_destination_t* destination) SKR_NOEXCEPT
{
    // try push back new request
    auto io_task = make_zeroed<Task>();
    io_task.priority = texture_info->priority;
    io_task.sub_priority = texture_info->sub_priority;
    io_task.request = async_request;
    for (uint32_t i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; i++)
    {
        io_task.callbacks[i] = texture_info->callbacks[i];
        io_task.callback_datas[i] = texture_info->callback_datas[i];
    }
    io_task.path = texture_info->dstorage.path ? texture_info->dstorage.path : "";
    if (texture_info->dstorage.queue)
    {
        io_task.resource_task = make_zeroed<DStorageTextureTask>();
        auto&& ds_buffer_task = skr::get<DStorageTextureTask>(io_task.resource_task);
        ds_buffer_task.texture_io = *texture_info;
        ds_buffer_task.destination = destination;
    }
    else
    {
        io_task.resource_task = make_zeroed<TextureTask>();
        auto&& buffer_task = skr::get<TextureTask>(io_task.resource_task);
        buffer_task.texture_io = *texture_info;
        buffer_task.destination = destination;
    }
    tasks.enqueue_(io_task, threaded_service.criticalTaskCount, name.c_str());
    threaded_service.request_();
}

skr_io_vram_service_t* skr_io_vram_service_t::create(const skr_vram_io_service_desc_t* desc) SKR_NOEXCEPT
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

void skr_io_vram_service_t::destroy(skr_io_vram_service_t* s) SKR_NOEXCEPT
{
    auto service = static_cast<skr::io::VRAMServiceImpl*>(s);
    s->drain();
    service->threaded_service.destroy_();
    SkrDelete(service);
}

bool skr::io::VRAMServiceImpl::try_cancel(skr_async_request_t* request) SKR_NOEXCEPT
{
    // TODO: Cancel on DStorage Queue
    return tasks.try_cancel_(request);
}

void skr::io::VRAMServiceImpl::defer_cancel(skr_async_request_t* request) SKR_NOEXCEPT
{
    // TODO: Cancel on DStorage Queue
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