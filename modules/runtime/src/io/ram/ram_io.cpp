#include "../common/io_runnner.hpp"

#include "async/wait_timeout.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"

#include <stdlib.h> // abort

namespace skr {
namespace io {

const char* kIOBufferMemoryName = "IOBuffer";
uint32_t RAMService::global_idx = 0;

IRAMIOBuffer::~IRAMIOBuffer() SKR_NOEXCEPT {}

RAMIOBuffer::~RAMIOBuffer() SKR_NOEXCEPT
{
    free_buffer();
}

void RAMIOBuffer::allocate_buffer(uint64_t n) SKR_NOEXCEPT
{
    if (n)
    {
        bytes = (uint8_t*)sakura_mallocN(n, kIOBufferMemoryName);
    }
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
    auto rq = skr::static_pointer_cast<RAMIORequest>(request);
    rq->future = future;
    rq->destination = buffer;
    SKR_ASSERT(!rq->blocks.empty());
    requests.emplace_back(request);
    return buffer;
}

inline static IOReaderId<IIORequestProcessor> CreateReader(RAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto reader = skr::SObjectPtr<VFSRAMReader>::Create(service, desc->io_job_queue);
    return std::move(reader);
}

RAMService::RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    : name(desc->name ? skr::string(desc->name) : skr::format(u8"IRAMService-{}", global_idx++)), 
      awake_at_request(desc->awake_at_request),
      runner(this, desc->callback_job_queue)
{
    request_pool = SmartPoolPtr<RAMIORequest, IIORequest>::Create();
    ram_buffer_pool = SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer>::Create();
    ram_batch_pool = SmartPoolPtr<RAMIOBatch, IIOBatch>::Create();

    runner.reader = CreateReader(this, desc);
    runner.set_resolvers();

    if (!desc->awake_at_request)
    {
        if (desc->sleep_time > 2000)
        {
            SKR_ASSERT(desc->sleep_time <= 2000);
            SKR_LOG_FATAL("RAMService: too long sleep_time causes 'deadlock' when awake_at_request is false");
        }
    }
    runner.set_sleep_time(desc->sleep_time);
}

skr_io_ram_service_t* IRAMService::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    return SkrNew<RAMService>(desc);
}

void IRAMService::destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT
{
    ZoneScopedN("destroy");

    auto S = static_cast<RAMService*>(service);
    S->runner.destroy();
    SkrDelete(S);
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
        runner.awake();
    }
}

RAMIOBufferId RAMService::request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch = open_batch(1);
    auto result = batch->add_request(request, future);
    auto buffer = skr::static_pointer_cast<RAMIOBuffer>(result);
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
    runner.drain(priority);    
    {
        ZoneScopedN("server_drain");
        auto predicate = [this, priority] {
            return !runner.processing_count(priority);
        };
        bool fatal = !wait_timeout(predicate, 5);
        if (fatal)
        {
            SKR_LOG_FATAL("RAMService: drain timeout, %llu requests are still processing", 
                runner.processing_count(priority));
        }
    }
}

void RAMService::set_sleep_time(uint32_t ms) SKR_NOEXCEPT
{
    runner.set_sleep_time(ms);
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