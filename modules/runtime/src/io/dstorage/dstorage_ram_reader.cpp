#include "../ram/ram_request.hpp"
#include "../ram/ram_buffer.hpp"

#include "../ram/ram_readers.hpp"

namespace skr {
namespace io {

static const ESkrDStoragePriority DStoragePriorityLUT[] = 
{
    SKR_DSTORAGE_PRIORITY_LOW,
    SKR_DSTORAGE_PRIORITY_NORMAL,
    SKR_DSTORAGE_PRIORITY_HIGH
};
static_assert(sizeof(DStoragePriorityLUT) / sizeof(DStoragePriorityLUT[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

static const char8_t* DStorageNames[] = { u8"F2M-Low", u8"F2M-Normal", u8"F2M-High" };
static_assert(sizeof(DStorageNames) / sizeof(DStorageNames[0]) == SKR_ASYNC_SERVICE_PRIORITY_COUNT);

DStorageRAMReader::DStorageRAMReader(RAMService* service) SKR_NOEXCEPT 
    : RAMReaderBase(service)
{
    SkrDStorageQueueDescriptor desc = {};
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        events[i] = SmartPoolPtr<DStorageEvent>::Create();
    }
    for (auto i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        const auto dpriority = DStoragePriorityLUT[i];
        desc.source = SKR_DSTORAGE_SOURCE_FILE;
        desc.capacity = SKR_DSTORAGE_MAX_QUEUE_CAPACITY;
        desc.priority = dpriority;
        desc.name = DStorageNames[i];
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
    fetched_batches[priority].enqueue(batch);
    inc_processing(priority);
    return true;
}

void DStorageRAMReader::enqueueAndSubmit(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto queue = f2m_queues[priority];
    auto instance = skr_get_dstorage_instnace();
    auto event = skr::static_pointer_cast<DStorageEvent>(events[priority]->allocate(queue));
    IOBatchId batch;
    while (fetched_batches[priority].try_dequeue(batch))
    {
        for (auto request : batch->get_requests())
        {
            auto rq = skr::static_pointer_cast<RAMIORequest>(request);
            auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
            if (service->runner.try_cancel(priority, rq))
            {
                skr_dstorage_close_file(instance, rq->dfile);
                rq->dfile = nullptr;
            }
            else if ( rq->getStatus() == SKR_IO_STAGE_RESOLVING)
            {
                ZoneScopedN("read_request");
                SKR_ASSERT(rq->dfile);
                rq->setStatus(SKR_IO_STAGE_LOADING);
                uint64_t dst_offset = 0u;
                for (const auto& block : rq->blocks)
                {
                    const auto address = buf->bytes + dst_offset;
                    SkrDStorageIODescriptor io = {};
                    io.name = rq->get_path();
                    io.event = nullptr;
                    io.source_type = SKR_DSTORAGE_SOURCE_FILE;
                    io.compression = SKR_DSTORAGE_COMPRESSION_NONE; // TODO: DECOMPRESS

                    io.source_file.file = rq->dfile;
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
        event->batches.emplace_back(batch);
    }
    if (const auto enqueued = event->batches.size())
    {
        skr_dstorage_queue_submit(queue, event->event);
        submitted[priority].emplace_back(event);
    }
}

void DStorageRAMReader::pollSubmitted(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    for (auto& e : submitted[priority])
    {
        if (e->okay() || e->batches.empty())
        {
            for (auto batch : e->batches)
            {
                for (auto request : batch->get_requests())
                {
                    auto rq = skr::static_pointer_cast<RAMIORequest>(request);
                    rq->setStatus(SKR_IO_STAGE_LOADED);
                }
                loaded_batches[priority].enqueue(batch);
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
    if (loaded_batches[priority].try_dequeue(batch))
    {
        dec_processed(priority);
        return batch.get();
    }
    return false;
}

} // namespace io
} // namespace skr