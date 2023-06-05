#include "../common/io_runnner.hpp"

#include "misc/defer.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"

#include <stdlib.h> // abort

namespace skr {
namespace io {

const char* kIOBufferMemoryName = "IOBuffer";
uint32_t RAMService::global_idx = 0;

template<typename F>
bool wait_timeout(F f, uint32_t seconds_timeout = 3)
{
    ZoneScopedN("WaitTimeOut");
    uint32_t milliseconds = 0;
    while (!f())
    {
        if (milliseconds > seconds_timeout * 100)
        {
            SKR_LOG_ERROR("drain timeout, force quit");
            return false;
        }
        skr_thread_sleep(1);
        milliseconds++;
    }
    return true;
}

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
    auto srv = static_cast<RAMService*>(service);
    auto buffer = srv->ram_buffer_pool->allocate();
    auto&& rq = skr::static_pointer_cast<RAMIORequest>(request);
    rq->future = future;
    rq->destination = buffer;
    SKR_ASSERT(!rq->blocks.empty());
    requests.emplace_back(request);
    return buffer;
}

inline static IOReaderId CreateReader(RAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto reader = skr::SObjectPtr<VFSRAMReader>::Create(service, desc->io_job_queue);
    return std::move(reader);
}

RAMService::RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    : name(desc->name ? skr::string(desc->name) : skr::format(u8"IRAMService-{}", global_idx++)), 
      trace_log(desc->trace_log), awake_at_request(desc->awake_at_request),
      runner(this, CreateReader(this, desc), desc->callback_job_queue)
{
    request_pool = SmartPoolPtr<RAMIORequest, IIORequest>::Create();
    ram_buffer_pool = SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer>::Create();
    ram_batch_pool = SmartPoolPtr<RAMIOBatch, IIOBatch>::Create();

    if (!desc->awake_at_request)
    {
        if (desc->sleep_time > 2000)
        {
            SKR_ASSERT(desc->sleep_time <= 2000);
            SKR_LOG_FATAL("RAMService: too long sleep_time causes 'deadlock' when awake_at_request is false");
        }
    }
    runner.setSleepTime(desc->sleep_time);
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
    S->drain();
    if (S->runner.get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        S->runner.setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
        S->stop(false);
    }
    {
        ZoneScopedN("wait_stop");
        if (S->trace_log)
            SKR_LOG_TRACE("IRAMService::destroy: wait runner thread to stop...");
        S->runner.wait_stop();
    }
    {
        ZoneScopedN("exit");
        if (S->trace_log)
            SKR_LOG_TRACE("IRAMService::destroy: wait runner thread to exit...");
        S->runner.exit();
    }
    SkrDelete(S);
    SKR_LOG_TRACE("IRAMService::destroy: !");
}

IOBatchId RAMService::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(ram_batch_pool->allocate(this, seq, n));
}

IORequestId RAMService::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IIORequest>(request_pool->allocate(seq));
}

void RAMService::request(IOBatchId batch) SKR_NOEXCEPT
{
    runner.enqueueBatch(batch);
    if (awake_at_request)
    {
        runner.tryAwake();
    }
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
        const bool sucess = wait_timeout([&](){
            return (runner.getQueuedBatchCount(priority) == 0) &&
            (runner.getExecutingBatchCount(priority) == 0) &&
            (runner.getProcessingRequestCount(priority) == 0);
        }, 8);
        if (!sucess)
        {
            SKR_LOG_FATAL("RAMService::drain: timeout!");
            abort();
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
    SKR_DEFER( { ZoneScopedNC("Finish", tracy::Color::Tan1); finish(); recycle(); } );

    uint64_t cnt;
    {
        ZoneScopedNC("Resolve", tracy::Color::Orchid1);

        resolve();
        cnt = fetch();
    }
    if (cnt)
    {
        ZoneScopedNC("Dispatch", tracy::Color::Maroon1);

        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
        dispatch();
        uncompress();
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

} // namespace io
} // namespace skr