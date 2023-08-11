#include "SkrRT/platform/debug.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/misc/defer.hpp"
#include "vram_readers.hpp"
#include <EASTL/fixed_map.h>
#include <tuple>

// VFS READER IMPLEMENTATION

namespace skr {
namespace io {

SwapableCmdPool::RC::RC() SKR_NOEXCEPT
{

}

SwapableCmdPool::RC::RC(CGPUCommandPoolId v, SAtomic32* pRC) SKR_NOEXCEPT
    : v(v), pRC(pRC)
{
    if (pRC)
        skr_atomic32_add_relaxed(pRC, 1);
}

SwapableCmdPool::RC::~RC() SKR_NOEXCEPT
{
    if (pRC)
    {
        auto prev = skr_atomic32_add_relaxed(pRC, -1);
        if (prev == 1)
        {
            cgpu_reset_command_pool(v);
        }
    }
}

SwapableCmdPool::SwapableCmdPool() SKR_NOEXCEPT
{

}

SwapableCmdPool::~SwapableCmdPool() SKR_NOEXCEPT
{
    SKR_ASSERT(pools[0] == nullptr);
    SKR_ASSERT(pools[1] == nullptr);
}

void SwapableCmdPool::initialize(CGPUQueueId queue) SKR_NOEXCEPT
{
    CGPUCommandPoolDescriptor pdesc = { u8"VRAMIOService-CmdPool" };
    pools[0] = cgpu_create_command_pool(queue, &pdesc);
    pools[1] = cgpu_create_command_pool(queue, &pdesc);
}

void SwapableCmdPool::finalize() SKR_NOEXCEPT
{
    cgpu_free_command_pool(pools[0]);
    cgpu_free_command_pool(pools[1]);
    pools[0] = nullptr;
    pools[1] = nullptr;
}

FORCEINLINE SwapableCmdPool::RC SwapableCmdPool::get() SKR_NOEXCEPT
{
    return RC(pools[index], &rcs[index]);
}

void SwapableCmdPool::swap() SKR_NOEXCEPT
{
    index = (index == 0) ? 1 : 0;
}

GPUUploadCmd::GPUUploadCmd() SKR_NOEXCEPT
{

}

GPUUploadCmd::GPUUploadCmd(CGPUQueueId queue, IOBatchId batch) SKR_NOEXCEPT
    : batch(batch), queue(queue)
{

}

void GPUUploadCmd::start(SwapableCmdPool& swap_pool) SKR_NOEXCEPT
{
    pool = swap_pool.get();
    CGPUCommandBufferDescriptor bdesc = { /*.is_secondary = */false };
    cmdbuf = cgpu_create_command_buffer(pool, &bdesc);
    cgpu_cmd_begin(cmdbuf);
    fence = cgpu_create_fence(queue->device);
}

void GPUUploadCmd::finish() SKR_NOEXCEPT
{
    for (auto upload_buffer : upload_buffers)
        cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cmdbuf);
    cgpu_free_fence(fence);
    okay = true;
}

CommonVRAMReader::CommonVRAMReader(VRAMService* service, IRAMService* ram_service) SKR_NOEXCEPT 
    : VRAMReaderBase(service), ram_service(ram_service) 
{

}

CommonVRAMReader::~CommonVRAMReader() SKR_NOEXCEPT
{
    for (auto&& [queue, pool] : cmdpools)
        pool.finalize();   
}

bool CommonVRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    fetched_batches[priority].enqueue(batch);
    inc_processing(priority);
    return true;
}

void CommonVRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SkrZoneScopedN("CommonVRAMReader::dispatch");

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

bool CommonVRAMReader::shouldUseUpload(IIORequest* request) const SKR_NOEXCEPT
{
    if (auto pDS = io_component<VRAMDStorageComponent>(request))
    {
        return !pDS->should_use_dstorage();
    }
    return true;
}

void CommonVRAMReader::addRAMRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    RAMBatchPtr ram_batch = nullptr;
    IOBatchId vram_batch = nullptr;
    while (fetched_batches[priority].try_dequeue(vram_batch))
    {
    for (auto&& vram_request : vram_batch->get_requests())
    {
        if (!shouldUseUpload(vram_request.get())) 
            continue;
        if (service->runner.try_cancel(priority, vram_request))
        {
            // cancel...
        }
        else
        {
            auto pStatus = io_component<IOStatusComponent>(vram_request.get());
            if (pStatus->getStatus() == SKR_IO_STAGE_RESOLVING)
            {
                SkrZoneScopedN("VRAMReader::RAMRequest");
                auto pPath = io_component<PathSrcComponent>(vram_request.get());
                auto pUpload = io_component<VRAMUploadComponent>(vram_request.get());
                if (pPath && pPath->get_path() && pUpload)
                {
                    pStatus->setStatus(SKR_IO_STAGE_LOADING);
                    auto ram_request = ram_service->open_request();
                    if (!ram_batch)
                    {
                        ram_batch = skr::static_pointer_cast<RAMIOBatch>(ram_service->open_batch(8));
                    }
                    if (auto vfs = pPath->get_vfs())
                        ram_request->set_vfs(vfs);
                    ram_request->set_path(pPath->get_path());
                    // TODO: READ PARTIAL DATA ONLY NEEDED FROM FILE
                    ram_request->add_block({});
                    if (auto pinnedBuffer = pUpload->ram_buffer) // pinned result
                    {
                        ram_batch->add_request(ram_request, pinnedBuffer, &pUpload->ram_future);
                    }
                    else if (auto result = ram_batch->add_request(ram_request, &pUpload->ram_future))
                    {
                        pUpload->ram_buffer = skr::static_pointer_cast<IRAMIOBuffer>(result);
                    }
                }
                auto pMemory = io_component<MemorySrcComponent>(vram_request.get());
                if (pMemory && pMemory->data && pMemory->size)
                {
                    pUpload->src_data = pMemory->data;
                    pUpload->src_size = pMemory->size;
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
    // erase null batches
    batches.erase(
    eastl::remove_if(batches.begin(), batches.end(), [](auto& batch) {
        return (batch == nullptr);
    }), batches.end());

    // erase empty batches
    batches.erase(
    eastl::remove_if(batches.begin(), batches.end(), [](auto& batch) {
        return batch->get_requests().empty();
    }), batches.end());

    for (auto&& batch : batches)
    {
        uint32_t finished_cnt = 0;
        uint32_t skip_cnt = 0;
        auto requests = batch->get_requests();
        for (auto&& vram_request : requests)
        {
            if (!shouldUseUpload(vram_request.get()))
            {
                finished_cnt += 1;
                skip_cnt += 1;
            }
            else if (auto pUpload = io_component<VRAMUploadComponent>(vram_request.get()))
            {
                if (pUpload->ram_buffer && pUpload->ram_future.is_ready())
                {
                    pUpload->src_data = pUpload->ram_buffer->get_data();
                    pUpload->src_size = pUpload->ram_buffer->get_size();
                }
                if (pUpload->src_data && pUpload->src_size)
                {
                    finished_cnt += 1;
                }
            }
        }
        if (skip_cnt == requests.size()) // batch is all dstorage
        {
            finishBatch(priority, batch);
            batch.reset();
        }
        else if (finished_cnt == requests.size()) // batch is all finished
        {
            to_upload_batches[priority].emplace_back(batch);
            batch.reset();
        }
    }
}

struct StackCmdMapKey
{
    CGPUQueueId queue;
    IIOBatch* batch;
    bool operator<(const StackCmdMapKey& rhs) const
    {
        return std::tie(queue, batch) < std::tie(rhs.queue, batch);
    }
};
template <size_t N = 1>
struct StackCmdAllocator : public eastl::fixed_map<StackCmdMapKey, GPUUploadCmd, N>
{
    auto& allocate(IOBatchId& batch, SwapableCmdPoolMap& cmdpools, VRAMUploadComponent* pUpload)
    {
        auto& cmds = *this;
        auto transfer_queue = pUpload->get_transfer_queue();
        auto key = StackCmdMapKey{ transfer_queue, batch.get() };
        if (cmds.find(key) == cmds.end())
        {
            cmds.emplace(key, GPUUploadCmd(transfer_queue, batch));
        }
        auto& cmd = cmds[key];
        auto cmdqueue = cmd.get_queue();
        if (cmdpools.find(cmdqueue) == cmdpools.end())
        {
            auto&& [iter, sucess] = cmdpools.emplace(cmdqueue, SwapableCmdPool());
            iter->second.initialize(cmdqueue);
        }
        auto&& cmdpool = cmdpools[cmdqueue];
        if (cmd.get_cmdbuf() == nullptr)
        {
            SkrZoneScopedN("PrepareCmd");
            cmd.start(cmdpool);
        }
        return cmd;
    }
};

void CommonVRAMReader::addUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SkrZoneScopedN("VRAMReader::UploadRequests");

    for (auto&& batch : to_upload_batches[priority])
    {
        StackCmdAllocator<1> cmds;
        auto requests = batch->get_requests();
        for (auto&& vram_request : requests)
        {
            auto pUpload = io_component<VRAMUploadComponent>(vram_request.get());
#ifdef SKR_PROFILE_ENABLE
            auto pPath = io_component<PathSrcComponent>(vram_request.get());
#endif
            auto& cmd = cmds.allocate(batch, cmdpools, pUpload);
            auto cmdqueue = cmd.get_queue();
            auto cmdbuf = cmd.get_cmdbuf();
            // record copy command
            if (auto pBuffer = io_component<VRAMBufferComponent>(vram_request.get()))
            {
                CGPUBufferId upload_buffer = nullptr;
                {
                    // prepare upload buffer
                    SkrZoneScopedN("PrepareUploadBuffer");
#ifdef SKR_PROFILE_ENABLE
                    skr::string Name = u8"BufferUpload-";
                    Name += pPath->get_path();
                    SkrMessage(Name.c_str(), Name.size());
#endif
                    skr::string name = /*pBuffer->name ? buffer_io.vbuffer.buffer_name :*/ u8"";
                    name += u8"-upload";
                    upload_buffer = cgpux_create_mapped_upload_buffer(cmdqueue->device, pUpload->src_size, name.u8_str());
                    cmd.upload_buffers.emplace_back(upload_buffer);

                    memcpy(upload_buffer->info->cpu_mapped_address, pUpload->src_data, pUpload->src_size);
                }
                if (upload_buffer)
                {
                    // TODO: SUPPORT BLOCKS
                    CGPUBufferToBufferTransfer buf_cpy = {};
                    buf_cpy.dst = pBuffer->buffer;
                    buf_cpy.dst_offset = pBuffer->offset;
                    buf_cpy.src = upload_buffer;
                    buf_cpy.src_offset = 0;
                    buf_cpy.size = pUpload->src_size;
                    cgpu_cmd_transfer_buffer_to_buffer(cmdbuf, &buf_cpy);
                }
                auto&& Artifact = skr::static_pointer_cast<VRAMBuffer>(pBuffer->artifact);
                Artifact->buffer = pBuffer->buffer;
                // TODO: RELEASE BARRIER
                auto buffer_barrier = make_zeroed<CGPUBufferBarrier>();
                buffer_barrier.buffer = pBuffer->buffer;
                buffer_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                buffer_barrier.dst_state = CGPU_RESOURCE_STATE_COMMON;
                // release
                if (cmdqueue->type == CGPU_QUEUE_TYPE_TRANSFER)
                {
                    buffer_barrier.queue_release = true;
                    buffer_barrier.queue_type = cmdqueue->type;
                }
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.buffer_barriers = &buffer_barrier;
                barrier_desc.buffer_barriers_count = 1;
                cgpu_cmd_resource_barrier(cmdbuf, &barrier_desc);
            }
            else if (auto pTexture = io_component<VRAMTextureComponent>(vram_request.get()))
            {
                CGPUBufferId upload_buffer = nullptr;
                {
                    // prepare upload buffer
                    SkrZoneScopedN("PrepareUploadBuffer");
#ifdef SKR_PROFILE_ENABLE
                    skr::string Name = u8"TextureUpload-";
                    Name += pPath->get_path();
                    SkrMessage(Name.c_str(), Name.size());
#endif
                    skr::string name = /*pBuffer->name ? buffer_io.vbuffer.buffer_name :*/ u8"";
                    name += u8"-upload";
                    upload_buffer = cgpux_create_mapped_upload_buffer(cmdqueue->device, pUpload->src_size, name.u8_str());
                    cmd.upload_buffers.emplace_back(upload_buffer);

                    memcpy(upload_buffer->info->cpu_mapped_address, pUpload->src_data, pUpload->src_size);
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
                    cgpu_cmd_transfer_buffer_to_texture(cmdbuf, &tex_cpy);
                }
                auto&& Artifact = skr::static_pointer_cast<VRAMTexture>(pTexture->artifact);
                Artifact->texture = pTexture->texture;
                // TODO: RELEASE BARRIER
                auto texture_barrier = make_zeroed<CGPUTextureBarrier>();
                texture_barrier.texture = pTexture->texture;
                texture_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                texture_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
                // release
                if (cmdqueue->type == CGPU_QUEUE_TYPE_TRANSFER)
                {
                    texture_barrier.queue_release = true;
                    texture_barrier.queue_type = cmdqueue->type;
                }
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.texture_barriers = &texture_barrier;
                barrier_desc.texture_barriers_count = 1;
                cgpu_cmd_resource_barrier(cmdbuf, &barrier_desc);
            }
        }
        // submit all cmds
        {
            SkrZoneScopedN("SubmitCmds");
            for (auto&& [key, cmd] : cmds)
            {
                gpu_uploads[priority].emplace_back(cmd);

                auto cmdbuf = cmd.get_cmdbuf();
                auto fence = cmd.get_fence();
                CGPUQueueSubmitDescriptor submit = {};
                submit.cmds = &cmdbuf;
                submit.cmds_count = 1;
                submit.signal_fence = fence;
                cgpu_cmd_end(cmdbuf);
                cgpu_submit_queue(key.queue, &submit);
            }
        }
    }
    // clear batches & swap all pools
    to_upload_batches[priority].clear();
    for (auto&& [queue, pool] : cmdpools)
    {
        pool.swap();
    }
}

void CommonVRAMReader::finishBatch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    dec_processing(priority);
    inc_processed(priority);
    processed_batches[priority].enqueue(batch);
}

void CommonVRAMReader::ensureUploadRequests(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    for (auto&& upload : gpu_uploads[priority])
    {
        auto batch = upload.get_batch();
        auto fence = upload.get_fence();
        auto status = cgpu_query_fence_status(fence);
        if (status == CGPU_FENCE_STATUS_COMPLETE)
        {
            SkrZoneScopedN("EnsureFence");
            for (auto&& request : batch->get_requests())
            {
                if (auto upload =  shouldUseUpload(request.get()))
                {
                    auto pStatus = io_component<IOStatusComponent>(request.get());
                    pStatus->setStatus(SKR_IO_STAGE_LOADED);
                }
            }
            finishBatch(priority, batch);
            upload.finish();
        }
    }
    // erase all finished uploads
    gpu_uploads[priority].erase(std::remove_if(
        gpu_uploads[priority].begin(), gpu_uploads[priority].end(), [](auto&& upload) {
            return upload.is_finished();
        }), gpu_uploads[priority].end());
}

} // namespace io
} // namespace skr

#include "../vram/vram_readers.hpp"

namespace skr {
namespace io {

static const ESkrDStoragePriority DStoragePriorityLUT_VRAM[] = 
{
    SKR_DSTORAGE_PRIORITY_LOW,
    SKR_DSTORAGE_PRIORITY_NORMAL,
    SKR_DSTORAGE_PRIORITY_HIGH
};
static_assert(sizeof(DStoragePriorityLUT_VRAM) / sizeof(DStoragePriorityLUT_VRAM[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

static const char8_t* DStorageNames_VRAM[] = { u8"F2V-Low", u8"F2V-Normal", u8"F2V-High" };
static_assert(sizeof(DStorageNames_VRAM) / sizeof(DStorageNames_VRAM[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

DStorageVRAMReader::DStorageVRAMReader(VRAMService* service, CGPUDeviceId device) SKR_NOEXCEPT 
    : VRAMReaderBase(service)
{
    SkrDStorageQueueDescriptor desc = {};
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        events[i] = SmartPoolPtr<DStorageEvent>::Create(kIOPoolObjectsMemoryName);
    }
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const auto dpriority = DStoragePriorityLUT_VRAM[i];
        desc.source = SKR_DSTORAGE_SOURCE_FILE;
        desc.capacity = SKR_DSTORAGE_MAX_QUEUE_CAPACITY;
        desc.priority = dpriority;
        desc.name = DStorageNames_VRAM[i];
        desc.gpu_device = device;
        f2v_queues[i] = skr_create_dstorage_queue(&desc);
    }
    desc.source = SKR_DSTORAGE_SOURCE_MEMORY;
    desc.priority = SKR_DSTORAGE_PRIORITY_REALTIME;
    desc.name = u8"M2V";
    m2v_queue = skr_create_dstorage_queue(&desc);
};

DStorageVRAMReader::~DStorageVRAMReader() SKR_NOEXCEPT
{
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        skr_free_dstorage_queue(f2v_queues[i]);
    }
    skr_free_dstorage_queue(m2v_queue);
}

bool DStorageVRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
{
    auto B = static_cast<IOBatchBase*>(batch.get());
    if (B->can_use_dstorage)
    {
        fetched_batches[priority].enqueue(batch);
        inc_processing(priority);
    }
    else
    {
        processed_batches[priority].enqueue(batch);
        inc_processed(priority);
    }
    return true;
}

bool DStorageVRAMReader::shouldUseDStorage(IIORequest* request) const SKR_NOEXCEPT
{
    if (auto pDS = io_component<VRAMDStorageComponent>(request))
    {
        return pDS->should_use_dstorage();
    }
    return false;
}

void DStorageVRAMReader::enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto instance = skr_get_dstorage_instnace();
    IOBatchId batch;
    eastl::fixed_map<SkrDStorageQueueId, skr::SObjectPtr<DStorageEvent>, 2> _events;
#ifdef SKR_PROFILE_ENABLE
    SkrCZoneCtx Zone;
    bool bZoneSet = false;
    SKR_DEFER({ if (bZoneSet) SkrCZoneEnd(Zone); });
#endif
    while (fetched_batches[priority].try_dequeue(batch))
    {
        auto addOrGetEvent = [&](SkrDStorageQueueId queue) {
            auto&& iter = _events.find(queue);
            if (iter == _events.end())
            {
                _events.emplace(queue, events[priority]->allocate(queue));
            }
            return _events[queue];
        };
        SkrDStorageQueueId queue = nullptr;
        for (auto&& vram_request : batch->get_requests())
        {
            if (!shouldUseDStorage(vram_request.get())) 
                continue;

#ifdef SKR_PROFILE_ENABLE
            if (!bZoneSet)
            {
                SkrCZoneN(z, "DStorage::EnqueueAndSubmit", 1);
                Zone = z;
                bZoneSet = true;
            }
#endif
            auto pStatus = io_component<IOStatusComponent>(vram_request.get());
            auto pDS = io_component<VRAMDStorageComponent>(vram_request.get());
            auto pMemory = io_component<MemorySrcComponent>(vram_request.get());
            if (service->runner.try_cancel(priority, vram_request))
            {
                skr_dstorage_close_file(instance, pDS->dfile);
                pDS->dfile = nullptr;
            }
            else if (auto pBuffer = io_component<VRAMBufferComponent>(vram_request.get()))
            {
                SkrZoneScopedN("DStorage::ReadBufferCmd");
                pStatus->setStatus(SKR_IO_STAGE_LOADING);
                CGPUDStorageBufferIODescriptor io = {};
                // io.name = rq->get_path();
                io.fence = nullptr;
                io.compression = SKR_DSTORAGE_COMPRESSION_NONE; // TODO: DECOMPRESS
                if (const bool memory_src = !pDS->dfile) // memory
                {
                    io.source_type = SKR_DSTORAGE_SOURCE_MEMORY;
                    io.source_memory.bytes = pMemory->data;
                    io.source_memory.bytes_size = pMemory->size;
                    io.uncompressed_size = pMemory->size;
                    queue = m2v_queue;
                }
                else // file
                {
                    SKR_ASSERT(pDS->dfile);
                    SkrDStorageFileInfo fInfo = {};
                    skr_dstorage_query_file_info(instance, pDS->dfile, &fInfo);
                    io.source_type = SKR_DSTORAGE_SOURCE_FILE;
                    io.source_file.file = pDS->dfile;
                    io.source_file.offset = 0;
                    io.source_file.size = fInfo.file_size;
                    io.uncompressed_size = fInfo.file_size;
                    queue = f2v_queues[priority];
                }
                pDS->get_dstorage_compression(io.compression, io.uncompressed_size);
                io.buffer = pBuffer->buffer;
                io.offset = pBuffer->offset;
                addOrGetEvent(queue);
                cgpu_dstorage_enqueue_buffer_request(queue, &io);

                auto&& Artifact = skr::static_pointer_cast<VRAMBuffer>(pBuffer->artifact);
                Artifact->buffer = pBuffer->buffer;
            }
            else if (auto pTexture = io_component<VRAMTextureComponent>(vram_request.get()))
            {
                SkrZoneScopedN("DStorage::ReadTextureCmd");
                pStatus->setStatus(SKR_IO_STAGE_LOADING);
                CGPUDStorageTextureIODescriptor io = {};
                // io.name = rq->get_path();
                io.fence = nullptr;
                io.compression = SKR_DSTORAGE_COMPRESSION_NONE; // TODO: DECOMPRESS
                if (const bool memory_src = !pDS->dfile) // memory
                {
                    io.source_type = SKR_DSTORAGE_SOURCE_MEMORY;
                    io.source_memory.bytes = pMemory->data;
                    io.source_memory.bytes_size = pMemory->size;
                    io.uncompressed_size = pMemory->size;
                    queue = m2v_queue;
                }
                else // file
                {
                    SKR_ASSERT(pDS->dfile);
                    SkrDStorageFileInfo fInfo = {};
                    skr_dstorage_query_file_info(instance, pDS->dfile, &fInfo);
                    io.source_type = SKR_DSTORAGE_SOURCE_FILE;
                    io.source_file.file = pDS->dfile;
                    io.source_file.offset = 0;
                    io.source_file.size = fInfo.file_size;
                    io.uncompressed_size = fInfo.file_size;
                    queue = f2v_queues[priority];
                }
                pDS->get_dstorage_compression(io.compression, io.uncompressed_size);
                io.texture = pTexture->texture;
                const auto pInfo = io.texture->info;
                io.width = (uint32_t)pInfo->width;
                io.height = (uint32_t)pInfo->height;
                io.depth = (uint32_t)pInfo->depth;
                addOrGetEvent(queue);
                cgpu_dstorage_enqueue_texture_request(queue, &io);

                auto&& Artifact = skr::static_pointer_cast<VRAMTexture>(pTexture->artifact);
                Artifact->texture = pTexture->texture;
            }
        }
        // TODO: hybrid m2v/f2v batches support
        if (queue)
        {
            addOrGetEvent(queue)->batches.emplace_back(batch);
        }
        else
        {
            processed_batches[priority].enqueue(batch);
            dec_processing(priority);
            inc_processed(priority);
        }
    }
    for (auto&& [k, event] : _events)
    {
        if (const auto enqueued = event->batches.size())
        {
            skr_dstorage_queue_submit(event->queue, event->event);
            submitted[priority].emplace_back(event);
        }
    }
}

void DStorageVRAMReader::pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SkrZoneScopedN("DStorage::PollSubmitted");

    auto instance = skr_get_dstorage_instnace();
    for (auto& e : submitted[priority])
    {
        if (e->okay() || e->batches.empty())
        {
            for (auto batch : e->batches)
            {
                for (auto vram_request : batch->get_requests())
                {
                    if (!shouldUseDStorage(vram_request.get())) 
                        continue;
                    auto pDS = io_component<VRAMDStorageComponent>(vram_request.get());
                    auto pStatus = io_component<IOStatusComponent>(vram_request.get());
                    pStatus->setStatus(SKR_IO_STAGE_LOADED);
                    if (pDS->dfile)
                    {
                        skr_dstorage_close_file(instance, pDS->dfile);
                        pDS->dfile = nullptr;
                    }
                }
                processed_batches[priority].enqueue(batch);
                dec_processing(priority);
                inc_processed(priority);
            }
            e.reset();
        }
    }

    // remove empty events
    auto cleaner = eastl::remove_if(submitted[priority].begin(), submitted[priority].end(), [](const auto& e) { return !e; });
    submitted[priority].erase(cleaner, submitted[priority].end());
}

void DStorageVRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SkrZoneScopedN("DStorageVRAMReader::dispatch");

    enqueueAndSubmit(priority);
    pollSubmitted(priority);
}

void DStorageVRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{

}

bool DStorageVRAMReader::poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
{
    if (processed_batches[priority].try_dequeue(batch))
    {
        dec_processed(priority);
        return batch.get();
    }
    return false;
}
} // namespace io
} // namespace skr