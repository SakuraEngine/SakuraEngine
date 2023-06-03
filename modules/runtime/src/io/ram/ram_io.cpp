#include "../common/io_runnner.hpp"

#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"

#include "containers/hashmap.hpp"

namespace skr {
namespace io {

IRAMIOBuffer::~IRAMIOBuffer() SKR_NOEXCEPT
{

}

const char* kIOBufferMemoryName = "IOBuffer";
RAMIOBuffer::~RAMIOBuffer() SKR_NOEXCEPT
{
    free_buffer();
}

void RAMIOBuffer::allocate_buffer(uint64_t n) SKR_NOEXCEPT
{
    bytes = (uint8_t*)sakura_mallocN(n, kIOBufferMemoryName);
    size = n;
}

void RAMIOBuffer::free_buffer() SKR_NOEXCEPT
{
    if (bytes)
    {
        sakura_freeN(bytes, kIOBufferMemoryName);
        bytes = nullptr;
    }
    size = 0;
}
SmartPool<RAMIOBuffer, IRAMIOBuffer> buffer_pool;

IOResultId RAMIOBatch::add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT
{
    auto buffer = buffer_pool.allocate();
    auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);

    rq->future = future;
    rq->destination = buffer;
    SKR_ASSERT(!rq->blocks.empty());
    requests.emplace_back(request);

    return buffer;
}

RAMServiceImpl::RAMServiceImpl(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    : name(skr::format(u8"IRAMService-{}", global_idx++)), runner(this)
{
    reader = skr::SObjectPtr<VFSRAMReader>::Create(this);
}
uint32_t RAMServiceImpl::global_idx = 0;

skr_io_ram_service_t* IRAMService::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto srv = SkrNew<RAMServiceImpl>(desc);
    srv->run();
    return srv;
}

void IRAMService::destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT
{
    ZoneScopedN("destroy");

    auto S = static_cast<RAMServiceImpl*>(service);
    if (S->runner.get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        S->drain();
        S->runner.setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
        S->stop(false);
    }
    {
        ZoneScopedN("wait_stop");
        S->runner.wait_stop();
    }
    {
        ZoneScopedN("exit");
        S->runner.exit();
    }
    SkrDelete(service);
}

IOBatchId RAMServiceImpl::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(batch_pool.allocate(seq, n));
}

IORequestId RAMServiceImpl::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IIORequest>(request_pool.allocate(seq));
}

void RAMServiceImpl::request(IOBatchId batch) SKR_NOEXCEPT
{
    runner.enqueueBatch(batch);
    runner.tryAwake();
}

RAMIOBufferId RAMServiceImpl::request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch = open_batch(1);
    auto result = batch->add_request(request, future);
    auto&& buffer = skr::static_pointer_cast<RAMIOBuffer>(result);
    batch->set_priority(priority);
    this->request(batch);
    return buffer;
}

void RAMServiceImpl::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (wait_drain)
    {
        drain();
    }
    runner.stop();
}

void RAMServiceImpl::run() SKR_NOEXCEPT
{
    runner.run();
}

void RAMServiceImpl::drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    ZoneScopedN("drain");

    if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
    {
        while (runner.getQueuedBatchCount(priority) > 0)
        {
            // ...
        }
        while (skr_atomicu64_load_relaxed(&runner.ongoing_batch_counts[priority]) > 0)
        {
            // ...
        }
    }
    else
    {
        for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; i++)
        {
            const auto p = (SkrAsyncServicePriority)i;
            drain(p);
        }
    }
}

void RAMServiceImpl::set_sleep_time(uint32_t ms) SKR_NOEXCEPT
{
    runner.setSleepTime(ms);
}

SkrAsyncServiceStatus RAMServiceImpl::get_service_status() const SKR_NOEXCEPT
{
    return runner.getServiceStatus();
}

void RAMServiceImpl::poll_finish_callbacks() SKR_NOEXCEPT
{
    RQPtr rq = nullptr;
    while (runner.finish_queues->try_dequeue(rq))
    {
        rq->tryPollFinish();
    }
}

} // namespace io
} // namespace skr

namespace skr {
namespace io {

skr::AsyncResult RAMServiceImpl::Runner::serve() SKR_NOEXCEPT
{
    SKR_DEFER( { recycle(); } );

    resolve();
    fetch();
    
    uint64_t cnt = 0;
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        cnt += skr_atomicu64_load_relaxed(&ongoing_batch_counts[i]);
    }
    
    if (cnt)
    {
        ZoneScopedN("Serve");

        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
        sort();
        dispatch();
        uncompress();
        finish();
        return ASYNC_RESULT_OK;
    }
    else
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_SLEEPING);
        sleep();
        return ASYNC_RESULT_OK;
    }
    return ASYNC_RESULT_OK;
}

bool RAMServiceImpl::Runner::try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    const auto status = rq->getStatus();
    if (status >= SKR_IO_STAGE_LOADING) return false;

    if (bool cancel_requested = skr_atomicu32_load_acquire(&rq->future->request_cancel))
    {
        const auto d = skr_atomicu32_load_relaxed(&rq->done);
        if (d == 0)
        {
            rq->setStatus(SKR_IO_STAGE_CANCELLED);
            if (rq->needPollFinish())
            {
                finish_queues[priority].enqueue(rq);
                skr_atomicu32_store_relaxed(&rq->done, SKR_ASYNC_IO_DONE_STATUS_PENDING);
            }
            else
            {
                skr_atomicu32_store_relaxed(&rq->done, SKR_ASYNC_IO_DONE_STATUS_DONE);
            }
        }
        return true;
    }
    return false;
}

void RAMServiceImpl::Runner::recycle() SKR_NOEXCEPT
{
    ZoneScopedN("recycle");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        service->reader->recycle((SkrAsyncServicePriority)i);
        
        auto& arr = ongoing_batches[i];
        auto it = eastl::remove_if(arr.begin(), arr.end(), 
            [](const IOBatchId& batch) {
                for (auto request : batch->get_requests()) 
                {
                    auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
                    const auto done = skr_atomicu32_load_relaxed(&rq->done);
                    if (done != SKR_ASYNC_IO_DONE_STATUS_DONE)
                    {
                        return false;
                    }
                }
                return true;
            });
        const int64_t X = (int64_t)arr.size();
        arr.erase(it, arr.end());
        const int64_t Y = (int64_t)arr.size();
        skr_atomicu64_add_relaxed(&ongoing_batch_counts[i], Y - X);
    }
}

uint64_t RAMServiceImpl::Runner::fetch() SKR_NOEXCEPT
{
    SKR_ASSERT(service->reader);
    ZoneScopedN("fetch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        BatchPtr batch = nullptr;
        while (resolved_batch_queues[i].try_dequeue(batch))
        {
            service->reader->fetch((SkrAsyncServicePriority)i, batch);
            skr_atomicu64_add_relaxed(&queued_batch_counts[i], -1);
        }
    }
    return 0;
}

void RAMServiceImpl::Runner::sort() SKR_NOEXCEPT
{
    ZoneScopedN("sort");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        service->reader->sort((SkrAsyncServicePriority)i);
    }
}

void RAMServiceImpl::Runner::dispatch() SKR_NOEXCEPT
{
    ZoneScopedN("dispatch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        service->reader->dispatch((SkrAsyncServicePriority)i);
    }
}

void RAMServiceImpl::Runner::resolve() SKR_NOEXCEPT
{
    SKR_ASSERT(service->reader);
    ZoneScopedN("resolve");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        BatchPtr batch = nullptr;
        while (batch_queues[i].try_dequeue(batch))
        {
            ongoing_batches[i].emplace_back(batch);
            skr_atomicu64_add_relaxed(&ongoing_batch_counts[i], 1);

            for (auto&& request : batch->get_requests())
            {
                auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
                if (try_cancel((SkrAsyncServicePriority)i, rq))
                {
                    // ...
                }
                else
                {
                    rq->setStatus(SKR_IO_STAGE_RESOLVING);
                    for (auto&& resolver : resolver_chain->chain)
                        resolver->resolve(request);
                }
            }
            resolved_batch_queues[i].enqueue(batch);
        }
    }
}

void RAMServiceImpl::Runner::uncompress() SKR_NOEXCEPT
{
    // do nothing now
}

void RAMServiceImpl::Runner::finish() SKR_NOEXCEPT
{
    ZoneScopedN("finish");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        while (auto request = service->reader->poll_finish((SkrAsyncServicePriority)i))
        {
            auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
            rq->setStatus(SKR_IO_STAGE_COMPLETED);
            if (rq->needPollFinish())
            {
                finish_queues[i].enqueue(rq);
            }
            else
            {
                skr_atomicu32_store_relaxed(&rq->done, SKR_ASYNC_IO_DONE_STATUS_DONE);
            }
        }
    }
}

} // namespace io
} // namespace skr