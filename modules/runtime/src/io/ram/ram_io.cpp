#include "../common/io_runnner.hpp"

#include "misc/defer.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"

#include "containers/hashmap.hpp"

namespace skr {
namespace io {

const char* kIOBufferMemoryName = "IOBuffer";
SmartPool<RAMIOBuffer, IRAMIOBuffer> ram_buffer_pool;
SmartPool<RAMIOBatch, IIOBatch> ram_batch_pool;
uint32_t RAMService::global_idx = 0;

IRAMIOBuffer::~IRAMIOBuffer() SKR_NOEXCEPT {}

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

IOResultId RAMIOBatch::add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT
{
    auto buffer = ram_buffer_pool.allocate();
    auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
    rq->future = future;
    rq->destination = buffer;
    SKR_ASSERT(!rq->blocks.empty());
    requests.emplace_back(request);
    return buffer;
}

inline static IOReaderId CreateReader(RAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto reader = skr::SObjectPtr<VFSRAMReader>::Create(service);
    return std::move(reader);
}

RAMService::RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    : name(skr::format(u8"IRAMService-{}", global_idx++)), 
      runner(this, CreateReader(this, desc))
{
    
}

skr_io_ram_service_t* IRAMService::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto srv = SkrNew<RAMService>(desc);
    srv->run();
    return srv;
}

void IRAMService::destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT
{
    ZoneScopedN("destroy");

    auto S = static_cast<RAMService*>(service);
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

IOBatchId RAMService::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(ram_batch_pool.allocate(seq, n));
}

IORequestId RAMService::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IIORequest>(request_pool.allocate(seq));
}

void RAMService::request(IOBatchId batch) SKR_NOEXCEPT
{
    runner.enqueueBatch(batch);
    runner.tryAwake();
}

RAMIOBufferId RAMService::request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch = open_batch(1);
    auto result = batch->add_request(request, future);
    auto&& buffer = skr::static_pointer_cast<RAMIOBuffer>(result);
    batch->set_priority(priority);
    this->request(batch);
    return buffer;
}

void RAMService::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (wait_drain)
    {
        drain();
    }
    runner.stop();
}

void RAMService::run() SKR_NOEXCEPT
{
    runner.run();
}

void RAMService::drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT
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

void RAMService::set_sleep_time(uint32_t ms) SKR_NOEXCEPT
{
    runner.setSleepTime(ms);
}

SkrAsyncServiceStatus RAMService::get_service_status() const SKR_NOEXCEPT
{
    return runner.getServiceStatus();
}

void RAMService::poll_finish_callbacks() SKR_NOEXCEPT
{
    runner.poll_finish_callbacks();
}

} // namespace io
} // namespace skr

namespace skr {
namespace io {

skr::AsyncResult RAMService::Runner::serve() SKR_NOEXCEPT
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

bool RAMService::Runner::try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    const auto status = rq->getStatus();
    if (status >= SKR_IO_STAGE_LOADING) return false;

    if (bool cancel_requested = skr_atomicu32_load_acquire(&rq->future->request_cancel))
    {
        if (rq->getFinishStep() == SKR_ASYNC_IO_FINISH_STEP_NONE)
        {
            rq->setStatus(SKR_IO_STAGE_CANCELLED);
            if (rq->needPollFinish())
            {
                finish_queues[priority].enqueue(rq);
                rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_PENDING);
            }
            else
            {
                rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
            }
        }
        return true;
    }
    return false;
}

void RAMService::Runner::recycle() SKR_NOEXCEPT
{
    ZoneScopedN("recycle");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->recycle((SkrAsyncServicePriority)i);
        
        auto& arr = ongoing_batches[i];
        auto it = eastl::remove_if(arr.begin(), arr.end(), 
            [](const IOBatchId& batch) {
                for (auto request : batch->get_requests()) 
                {
                    auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
                    if (rq->getFinishStep() != SKR_ASYNC_IO_FINISH_STEP_DONE)
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

uint64_t RAMService::Runner::fetch() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
    ZoneScopedN("fetch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        BatchPtr batch = nullptr;
        while (resolved_batch_queues[i].try_dequeue(batch))
        {
            reader->fetch((SkrAsyncServicePriority)i, batch);
            skr_atomicu64_add_relaxed(&queued_batch_counts[i], -1);
        }
    }
    return 0;
}

void RAMService::Runner::sort() SKR_NOEXCEPT
{
    ZoneScopedN("sort");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->sort((SkrAsyncServicePriority)i);
    }
}

void RAMService::Runner::dispatch() SKR_NOEXCEPT
{
    ZoneScopedN("dispatch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        reader->dispatch((SkrAsyncServicePriority)i);
    }
}

void RAMService::Runner::resolve() SKR_NOEXCEPT
{
    SKR_ASSERT(reader);
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

void RAMService::Runner::uncompress() SKR_NOEXCEPT
{
    // do nothing now
}

void RAMService::Runner::finish() SKR_NOEXCEPT
{
    ZoneScopedN("finish");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        while (auto request = reader->poll_finish((SkrAsyncServicePriority)i))
        {
            auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
            rq->setStatus(SKR_IO_STAGE_COMPLETED);
            if (rq->needPollFinish())
            {
                finish_queues[i].enqueue(rq);
            }
            else
            {
                rq->setFinishStep(SKR_ASYNC_IO_FINISH_STEP_DONE);
            }
        }
    }
}

} // namespace io
} // namespace skr