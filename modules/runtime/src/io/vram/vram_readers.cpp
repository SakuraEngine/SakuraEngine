#include "../../pch.hpp"
#include "SkrRT/platform/debug.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "vram_readers.hpp"
#include "SkrRT/async/thread_job.hpp"
#include <EASTL/fixed_map.h>

// VFS READER IMPLEMENTATION

namespace skr {
namespace io {

CommonVRAMReader::CommonVRAMReader(VRAMService* service, IRAMService* ram_service) SKR_NOEXCEPT 
    : VRAMReaderBase(service), ram_service(ram_service) 
{

}

CommonVRAMReader::~CommonVRAMReader() SKR_NOEXCEPT
{
    
}

bool CommonVRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    fetched_batches[priority].enqueue(batch);
    inc_processing(priority);
    return true;
}

void CommonVRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    addRAMRequests(priority);
    ensureRAMRequests(priority);
    addUploadRequests(priority);
    ensureUploadRequests(priority);
}

void CommonVRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{

}

bool CommonVRAMReader::poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
{
    if (processed_batches[priority].try_dequeue(batch))
    {
        dec_processed(priority);
        return batch.get();
    }
    return false;
}

void CommonVRAMReader::addRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IOBatchId ram_batch = nullptr;
    IOBatchId vram_batch = nullptr;
    while (fetched_batches[priority].try_dequeue(vram_batch))
    {
    for (auto&& vram_request : vram_batch->get_requests())
    {
        if (service->runner.try_cancel(priority, vram_request))
        {
            // cancel...
        }
        else if (auto pStatus = io_component<IOStatusComponent>(vram_request.get()))
        {
            if (pStatus->getStatus() == SKR_IO_STAGE_RESOLVING)
            {
                ZoneScopedN("VRAMReader::RAMRequest");
                auto pPath = io_component<PathSrcComponent>(vram_request.get());
                auto pUpload = io_component<VRAMUploadComponent>(vram_request.get());
                if (pPath && !pPath->path.is_empty() && pUpload)
                {
                    pStatus->setStatus(SKR_IO_STAGE_LOADING);
                    auto ram_request = ram_service->open_request();
                    if (!ram_batch)
                    {
                        ram_batch = ram_service->open_batch(8);
                    }
                    if (pPath->vfs)
                        ram_request->set_vfs(pPath->vfs);
                    ram_request->set_path(pPath->path.u8_str());
                    // TODO: READ PARTIAL DATA ONLY NEEDED FROM FILE
                    ram_request->add_block({});
                    auto result = ram_batch->add_request(ram_request, &pUpload->ram_future);
                    pUpload->buffer = skr::static_pointer_cast<IRAMIOBuffer>(result);
                }
                auto pMemory = io_component<MemorySrcComponent>(vram_request.get());
                if (pMemory && pMemory->data && pMemory->size)
                {
                    pUpload->data = pMemory->data;
                    pUpload->size = pMemory->size;
                }
            }
            else
                SKR_UNREACHABLE_CODE()
        }
    }
    if(vram_batch != nullptr) 
        ramloading_batches[priority].emplace_back(vram_batch); 
    }
    if (ram_batch != nullptr)
        ram_service->request(ram_batch);        
}

void CommonVRAMReader::ensureRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto&& batches = ramloading_batches[priority];
    // erase empty batches
    batches.erase(
    eastl::remove_if(batches.begin(), batches.end(), [](auto& batch) {
        return batch->get_requests().empty();
    }), batches.end());

    for (auto&& batch : batches)
    {
        uint32_t finished_cnt = 0;
        auto requests = batch->get_requests();
        for (auto&& request : requests)
        {
            auto pUpload = io_component<VRAMUploadComponent>(request.get());
            if (pUpload->buffer && pUpload->ram_future.is_ready())
            {
                pUpload->data = pUpload->buffer->get_data();
                pUpload->size = pUpload->buffer->get_size();
            }
            if (pUpload->data && pUpload->size)
            {
                finished_cnt += 1;
            }
        }
        if (finished_cnt == requests.size()) // batch is all finished
        {
            to_upload_batches[priority].emplace_back(batch);
            batch.reset();
        }
    }

    // erase empty batches
    batches.erase(
    eastl::remove_if(batches.begin(), batches.end(), [](auto& batch) {
        return (batch == nullptr);
    }), batches.end());
}

void CommonVRAMReader::addUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto&& batches = to_upload_batches[priority];
    for (auto&& batch : batches)
    {
        eastl::fixed_map<CGPUQueueId, GPUUploadCmd, 1> cmds;
        auto requests = batch->get_requests();
        for (auto&& request : requests)
        {
            auto pUpload = io_component<VRAMUploadComponent>(request.get());
#ifdef TRACY_ENABLE
            auto pPath = io_component<PathSrcComponent>(request.get());
#endif
            if (cmds.find(pUpload->transfer_queue) == cmds.end())
            {
                [[maybe_unused]] auto&& [cmd_kv, sucess] = cmds.emplace(
                    pUpload->transfer_queue, GPUUploadCmd{
                        /*.queue = */pUpload->transfer_queue,
                        /*.batch = */batch,
                        /*.okay = */false,
                        /*.pool = */nullptr,
                        /*.cmdbuf = */nullptr,
                        /*.fence = */nullptr
                    });
            }
            auto&& cmd = cmds[pUpload->transfer_queue];
            if (cmd.pool == nullptr)
            {
                CGPUCommandPoolDescriptor pdesc = { /*.name = */u8"VRAMUploadCmdPool" };
                CGPUCommandBufferDescriptor bdesc = { /*.is_secondary = */false };
                cmd.pool = cgpu_create_command_pool(cmd.queue, &pdesc);
                cmd.cmdbuf = cgpu_create_command_buffer(cmd.pool, &bdesc);
                cgpu_cmd_begin(cmd.cmdbuf);
                cmd.fence = cgpu_create_fence(cmd.queue->device);
            }
            // record copy command
            if (auto pBuffer = io_component<VRAMBufferComponent>(request.get()))
            {
                CGPUBufferId upload_buffer = nullptr;
                {
                    // prepare upload buffer
                    ZoneScopedN("PrepareUploadBuffer");
#ifdef TRACY_ENABLE
                    skr::string Name = u8"BufferUpload-";
                    Name += pPath->path;
                    TracyMessage(Name.c_str(), Name.size());
#endif
                    skr::string name = /*pBuffer->name ? buffer_io.vbuffer.buffer_name :*/ u8"";
                    name += u8"-upload";
                    upload_buffer = cgpux_create_mapped_upload_buffer(cmd.queue->device, pUpload->size, name.u8_str());
                    cmd.upload_buffers.emplace_back(upload_buffer);

                    memcpy(upload_buffer->info->cpu_mapped_address, pUpload->data, pUpload->size);
                }
                if (upload_buffer)
                {
                    CGPUBufferToBufferTransfer buf_cpy = {};
                    buf_cpy.dst = pBuffer->buffer;
                    buf_cpy.dst_offset = pBuffer->offset;
                    buf_cpy.src = upload_buffer;
                    buf_cpy.src_offset = 0;
                    buf_cpy.size = pUpload->size;
                    cgpu_cmd_transfer_buffer_to_buffer(cmd.cmdbuf, &buf_cpy);
                }
                auto&& Artifact = skr::static_pointer_cast<VRAMBuffer>(pBuffer->artifact);
                Artifact->buffer = pBuffer->buffer;
                // TODO: RELEASE BARRIER
                auto buffer_barrier = make_zeroed<CGPUBufferBarrier>();
                buffer_barrier.buffer = pBuffer->buffer;
                buffer_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                buffer_barrier.dst_state = CGPU_RESOURCE_STATE_COMMON;
                // release
                if (cmd.queue->type == CGPU_QUEUE_TYPE_TRANSFER)
                {
                    buffer_barrier.queue_release = true;
                    buffer_barrier.queue_type = cmd.queue->type;
                }
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.buffer_barriers = &buffer_barrier;
                barrier_desc.buffer_barriers_count = 1;
                cgpu_cmd_resource_barrier(cmd.cmdbuf, &barrier_desc);
            }
            if (auto pTexture = io_component<VRAMTextureComponent>(request.get()))
            {
                CGPUBufferId upload_buffer = nullptr;
                {
                    // prepare upload buffer
                    ZoneScopedN("PrepareUploadBuffer");
#ifdef TRACY_ENABLE
                    skr::string Name = u8"TextureUpload-";
                    Name += pPath->path;
                    TracyMessage(Name.c_str(), Name.size());
#endif
                    skr::string name = /*pBuffer->name ? buffer_io.vbuffer.buffer_name :*/ u8"";
                    name += u8"-upload";
                    upload_buffer = cgpux_create_mapped_upload_buffer(cmd.queue->device, pUpload->size, name.u8_str());
                    cmd.upload_buffers.emplace_back(upload_buffer);

                    memcpy(upload_buffer->info->cpu_mapped_address, pUpload->data, pUpload->size);
                }
                if (upload_buffer)
                {
                    CGPUBufferToTextureTransfer tex_cpy = {};
                    tex_cpy.dst = pTexture->texture;
                    tex_cpy.dst_subresource.aspects = CGPU_TVA_COLOR;
                    // TODO: texture array & mips
                    tex_cpy.dst_subresource.base_array_layer = 0;
                    tex_cpy.dst_subresource.layer_count = 1;
                    tex_cpy.dst_subresource.mip_level = 0;
                    tex_cpy.src = upload_buffer;
                    tex_cpy.src_offset = 0;
                    cgpu_cmd_transfer_buffer_to_texture(cmd.cmdbuf, &tex_cpy);
                }
                auto&& Artifact = skr::static_pointer_cast<VRAMTexture>(pTexture->artifact);
                Artifact->texture = pTexture->texture;
                // TODO: RELEASE BARRIER
                auto texture_barrier = make_zeroed<CGPUTextureBarrier>();
                texture_barrier.texture = pTexture->texture;
                texture_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                texture_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
                // release
                if (cmd.queue->type == CGPU_QUEUE_TYPE_TRANSFER)
                {
                    texture_barrier.queue_release = true;
                    texture_barrier.queue_type = cmd.queue->type;
                }
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.texture_barriers = &texture_barrier;
                barrier_desc.texture_barriers_count = 1;
                cgpu_cmd_resource_barrier(cmd.cmdbuf, &barrier_desc);
            }
        }
        // submit all cmds
        for (auto&& [queue, cmd] : cmds)
        {
            gpu_uploads[priority].emplace_back(cmd);

            CGPUQueueSubmitDescriptor submit = {};
            submit.cmds = &cmd.cmdbuf;
            submit.cmds_count = 1;
            submit.signal_fence = cmd.fence;
            cgpu_cmd_end(cmd.cmdbuf);
            cgpu_submit_queue(queue, &submit);
        }
    }
    batches.clear();
}

void CommonVRAMReader::ensureUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    for (auto&& upload : gpu_uploads[priority])
    {
        auto status = cgpu_query_fence_status(upload.fence);
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            ZoneScopedN("EnsureFence");

            for (auto&& request : upload.batch->get_requests())
            {
                auto pStatus = io_component<IOStatusComponent>(request.get());
                pStatus->setStatus(SKR_IO_STAGE_LOADED);
            }
            upload.okay = true;
            dec_processing(priority);
            inc_processed(priority);
            processed_batches[priority].enqueue(upload.batch);
            {
                for (auto upload_buffer : upload.upload_buffers)
                    cgpu_free_buffer(upload_buffer);
                cgpu_free_command_buffer(upload.cmdbuf);
                cgpu_free_command_pool(upload.pool);
                cgpu_free_fence(upload.fence);
            }
        }
    }
    // erase all finished uploads
    gpu_uploads[priority].erase(std::remove_if(
        gpu_uploads[priority].begin(), gpu_uploads[priority].end(), [](auto&& upload) {
            return upload.okay;
        }), gpu_uploads[priority].end());
}

} // namespace io
} // namespace skr