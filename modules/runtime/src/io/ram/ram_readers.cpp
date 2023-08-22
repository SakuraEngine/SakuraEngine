#include "SkrRT/io/io.h"
#include "ram_readers.hpp"
#include "SkrRT/async/thread_job.hpp"

// VFS READER IMPLEMENTATION

namespace skr {
namespace io {

using VFSReaderFutureLauncher = skr::FutureLauncher<bool>;

bool VFSRAMReader::fetch(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
{
    auto pStatus = io_component<IOStatusComponent>(request.get());
    const auto loaded = (pStatus->getStatus() == SKR_IO_STAGE_LOADED);
    const auto cancelled = (pStatus->getStatus() == SKR_IO_STAGE_CANCELLED);
    if (loaded || cancelled) // already processed by other readers
    {
        loaded_requests[priority].enqueue(request);
        inc_processed(priority);
    }
    else if (auto pComp = io_component<IOStatusComponent>(request.get()))
    {
        SKR_ASSERT(pComp->getStatus() == SKR_IO_STAGE_RESOLVING);
        fetched_requests[priority].enqueue(request);
        inc_processing(priority);
    }
    return true;
}

uint64_t VFSRAMReader::get_prefer_batch_size() const SKR_NOEXCEPT
{
    return 0; // fread is blocking, so we don't need to batch
}

void VFSRAMReader::dispatchFunction(SkrAsyncServicePriority priority, const IORequestId& request) SKR_NOEXCEPT
{
    auto rq = skr::static_pointer_cast<RAMRequestMixin>(request);
    auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
    auto pBlocks = io_component<BlocksComponent>(request.get());
    if (auto pFile = io_component<FileComponent>(request.get()))
    {
        {
            SkrZoneScopedN("dispatch_read");
            if (service->runner.try_cancel(priority, rq))
            {
                // cancel...
                if (pFile->file) 
                {
                    skr_vfs_fclose(pFile->file);
                    pFile->file = nullptr;
                }
                if (buf)
                {
                    buf->free_buffer();
                }
            }
            else if (auto pStatus = io_component<IOStatusComponent>(request.get()))
            {
                if (pStatus->getStatus() == SKR_IO_STAGE_RESOLVING)
                {
                    SkrZoneScopedN("read_request");

                    pStatus->setStatus(SKR_IO_STAGE_LOADING);
                    // SKR_LOG_DEBUG(u8"dispatch read request: %s", rq->path.c_str());
                    uint64_t dst_offset = 0u;
                    for (const auto& block : pBlocks->blocks)
                    {
                        const auto address = buf->get_data() + dst_offset;
                        skr_vfs_fread(pFile->file, address, block.offset, block.size);
                        dst_offset += block.size;
                    }
                    pStatus->setStatus(SKR_IO_STAGE_LOADED);
                }
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
        {
            SkrZoneScopedN("dispatch_close");
            if (pFile->file)
            {
                // SKR_LOG_DEBUG(u8"dispatch close request: %s", rq->path.c_str());
                skr_vfs_fclose(pFile->file);
                pFile->file = nullptr;
                loaded_requests[priority].enqueue(rq);
                inc_processed(priority);
            }
        }
    }
    dec_processing(priority);

    awakeService();
}

void VFSRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    IORequestId rq;
    if (fetched_requests[priority].try_dequeue(rq))
    {
        auto launcher = VFSReaderFutureLauncher(job_queue);
        loaded_futures[priority].emplace_back(
            launcher.async([this, rq, priority](){
                SkrZoneScopedN("VFSReadTask");
                dispatchFunction(priority, rq);
                return true;
            })
        );
    }
}

bool VFSRAMReader::poll_processed_request(SkrAsyncServicePriority priority, IORequestId& request) SKR_NOEXCEPT
{
    if (loaded_requests[priority].try_dequeue(request))
    {
        dec_processed(priority);
        return request.get();
    }
    return false;
}

void VFSRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SkrZoneScopedN("VFSRAMReader::recycle");

    auto& arr = loaded_futures[priority];
    for (auto& future : arr)
    {
        auto status = future->wait_for(0);
        if (status == skr::FutureStatus::Ready)
        {
            SkrDelete(future);
            future = nullptr;
        }
    }
    auto it = eastl::remove_if(arr.begin(), arr.end(), 
        [](skr::IFuture<bool>* future) {
            return (future == nullptr);
        });
    arr.erase(it, arr.end());
}

} // namespace io
} // namespace skr

// DSTORAGE READER IMPLEMENTATION

#include "../ram/ram_request.hpp"
#include "../ram/ram_buffer.hpp"
#include "../ram/ram_readers.hpp"

namespace skr {
namespace io {

static const ESkrDStoragePriority DStoragePriorityLUT_RAM[] = 
{
    SKR_DSTORAGE_PRIORITY_LOW,
    SKR_DSTORAGE_PRIORITY_NORMAL,
    SKR_DSTORAGE_PRIORITY_HIGH
};
static_assert(sizeof(DStoragePriorityLUT_RAM) / sizeof(DStoragePriorityLUT_RAM[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

static const char8_t* DStorageNames_RAM[] = { u8"F2M-Low", u8"F2M-Normal", u8"F2M-High" };
static_assert(sizeof(DStorageNames_RAM) / sizeof(DStorageNames_RAM[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

DStorageRAMReader::DStorageRAMReader(RAMService* service) SKR_NOEXCEPT 
    : RAMReaderBase(service)
{
    SkrDStorageQueueDescriptor desc = {};
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        events[i] = SmartPoolPtr<DStorageEvent>::Create(kIOPoolObjectsMemoryName);
    }
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const auto dpriority = DStoragePriorityLUT_RAM[i];
        desc.source = SKR_DSTORAGE_SOURCE_FILE;
        desc.capacity = SKR_DSTORAGE_MAX_QUEUE_CAPACITY;
        desc.priority = dpriority;
        desc.name = DStorageNames_RAM[i];
        f2m_queues[i] = skr_create_dstorage_queue(&desc);
    }
    desc.source = SKR_DSTORAGE_SOURCE_MEMORY;
    desc.priority = SKR_DSTORAGE_PRIORITY_REALTIME;
    desc.name = u8"M2M";
    m2m_queue = skr_create_dstorage_queue(&desc);
};

DStorageRAMReader::~DStorageRAMReader() SKR_NOEXCEPT
{
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        skr_free_dstorage_queue(f2m_queues[i]);
    }
    skr_free_dstorage_queue(m2m_queue);
}

bool DStorageRAMReader::fetch(SkrAsyncServicePriority priority, IOBatchId batch) SKR_NOEXCEPT
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

void DStorageRAMReader::enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto queue = f2m_queues[priority];
    auto instance = skr_get_dstorage_instnace();
    IOBatchId batch;
    skr::SObjectPtr<DStorageEvent> event;
#ifdef SKR_PROFILE_ENABLE
    SkrCZoneCtx Zone;
    bool bZoneSet = false;
#endif
    while (fetched_batches[priority].try_dequeue(batch))
    {
        auto& eref = event;
        if (!eref)
        {
#ifdef SKR_PROFILE_ENABLE
            SkrCZoneN(z, "DStorage::EnqueueAndSubmit", 1);
            Zone = z;
            bZoneSet = true;
#endif
            eref = skr::static_pointer_cast<DStorageEvent>(events[priority]->allocate(queue));
        }
        for (auto&& request : batch->get_requests())
        {
            auto rq = skr::static_pointer_cast<RAMRequestMixin>(request);
            auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
            auto pBlocks = io_component<BlocksComponent>(request.get());
            if (auto pFile = io_component<FileComponent>(request.get()))
            {
                if (service->runner.try_cancel(priority, rq))
                {
                    skr_dstorage_close_file(instance, pFile->dfile);
                    pFile->dfile = nullptr;
                }
                else if (auto pStatus = io_component<IOStatusComponent>(request.get()))
                {
                    if (pStatus->getStatus() == SKR_IO_STAGE_RESOLVING)
                    {
                        SkrZoneScopedN("DStorage::ReadRequest");
                        SKR_ASSERT(pFile->dfile);
                        pStatus->setStatus(SKR_IO_STAGE_LOADING);
                        uint64_t dst_offset = 0u;
                        for (const auto& block : pBlocks->blocks)
                        {
                            const auto address = buf->get_data() + dst_offset;
                            SkrDStorageIODescriptor io = {};
                            io.name = rq->get_path();
                            io.event = nullptr;
                            io.source_type = SKR_DSTORAGE_SOURCE_FILE;
                            io.compression = SKR_DSTORAGE_COMPRESSION_NONE; // TODO: DECOMPRESS

                            io.source_file.file = pFile->dfile;
                            io.source_file.offset = block.offset;
                            io.source_file.size = block.size;

                            io.destination = address;
                            io.uncompressed_size = block.size;
                            skr_dstorage_enqueue_request(queue, &io);

                            dst_offset += block.size;
                        }
                    }
                    else
                        SKR_UNREACHABLE_CODE();
                }
            }

        }
        event->batches.emplace_back(batch);
    }
    if (event)
    {
        if (const auto enqueued = event->batches.size())
        {
            skr_dstorage_queue_submit(queue, event->event);
            submitted[priority].emplace_back(event);
        }
    }
#ifdef SKR_PROFILE_ENABLE
    if (bZoneSet)
        SkrCZoneEnd(Zone);
#endif
}

void DStorageRAMReader::pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    SkrZoneScopedN("DStorage::PollSubmitted");

    auto instance = skr_get_dstorage_instnace();
    for (auto& e : submitted[priority])
    {
        if (e->okay() || e->batches.empty())
        {
            for (auto batch : e->batches)
            {
                for (auto request : batch->get_requests())
                {
                    auto pFile = io_component<FileComponent>(request.get());
                    auto pStatus = io_component<IOStatusComponent>(request.get());
                    pStatus->setStatus(SKR_IO_STAGE_LOADED);
                    skr_dstorage_close_file(instance, pFile->dfile);
                    pFile->dfile = nullptr;
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

void DStorageRAMReader::dispatch(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    enqueueAndSubmit(priority);
    pollSubmitted(priority);
}

void DStorageRAMReader::recycle(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{

}

bool DStorageRAMReader::poll_processed_batch(SkrAsyncServicePriority priority, IOBatchId& batch) SKR_NOEXCEPT
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