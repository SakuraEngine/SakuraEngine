#include "SkrRT/async/wait_timeout.hpp"
#include "../dstorage/dstorage_resolvers.hpp"

#include "ram_service.hpp"
#include "ram_resolvers.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"

namespace skr::io {

namespace RAMUtils
{
inline static IOReaderId<IIORequestProcessor> CreateReader(RAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto reader = skr::SObjectPtr<VFSRAMReader>::Create(service, desc->io_job_queue);
    return std::move(reader);
}

inline static IOReaderId<IIOBatchProcessor> CreateBatchReader(RAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
#ifdef _WIN32
    if (skr_query_dstorage_availability() == SKR_DSTORAGE_AVAILABILITY_HARDWARE)
    {
        auto reader = skr::SObjectPtr<DStorageRAMReader>::Create(service);
        return std::move(reader);
    }
#endif
    return nullptr;
}
} // namespace RAMUtils

uint32_t RAMService::global_idx = 0;
RAMService::RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    : name(desc->name ? skr::string(desc->name) : skr::format(u8"RAMService-{}", global_idx++)), 
      awake_at_request(desc->awake_at_request),
      runner(this, desc->callback_job_queue)
{
    request_pool = SmartPoolPtr<RAMRequestMixin, IBlocksRAMRequest>::Create(kIOPoolObjectsMemoryName);
    ram_buffer_pool = SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer>::Create(kIOPoolObjectsMemoryName);
    ram_batch_pool = SmartPoolPtr<RAMIOBatch, IIOBatch>::Create(kIOPoolObjectsMemoryName);

    if (desc->use_dstorage)
        runner.ds_reader = RAMUtils::CreateBatchReader(this, desc);
    runner.vfs_reader = RAMUtils::CreateReader(this, desc);

    runner.set_resolvers();

    if (!desc->awake_at_request)
    {
        if (desc->sleep_time > 2000)
        {
            SKR_ASSERT(desc->sleep_time <= 2000);
            SKR_LOG_FATAL(u8"RAMService: too long sleep_time causes 'deadlock' when awake_at_request is false");
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
    SkrZoneScopedN("RAMService::destroy");

    auto S = static_cast<RAMService*>(service);
    S->runner.destroy();
    SkrDelete(S);
}

IOBatchId RAMService::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(ram_batch_pool->allocate(this, seq, n));
}

BlocksRAMRequestId RAMService::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IBlocksRAMRequest>(request_pool->allocate(this, seq));
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
        SkrZoneScopedN("RAMService::drain");
        auto predicate = [this, priority] {
            return !runner.processing_count(priority);
        };
        bool fatal = !wait_timeout(predicate, 5);
        if (fatal)
        {
            SKR_LOG_FATAL(u8"RAMService: drain timeout, %llu requests are still processing", 
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

RAMService::Runner::Runner(RAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT
    : RunnerBase({ service->name.u8_str(), SKR_THREAD_ABOVE_NORMAL }, job_queue),
    service(service)
{

}

void RAMService::Runner::enqueueBatch(const IOBatchId& batch) SKR_NOEXCEPT
{
    const auto priority = batch->get_priority();
    for (auto&& request : batch->get_requests())
    {
        if (auto pStatus = io_component<IOStatusComponent>(request.get()))
        {
            auto status = pStatus->getStatus();
            SKR_ASSERT(status == SKR_IO_STAGE_NONE);
            pStatus->setStatus(SKR_IO_STAGE_ENQUEUED);
        }
    }
    batch_buffer->fetch(priority, batch);
    skr_atomic64_add_relaxed(&processing_request_counts[priority], 1);
}

void RAMService::Runner::set_resolvers() SKR_NOEXCEPT
{
    auto alloc_buffer = SObjectPtr<AllocateIOBufferResolver>::Create();
    auto chain = skr::static_pointer_cast<IORequestResolverChain>(IIORequestResolverChain::Create());
    chain->runner = this;

    IORequestResolverId open_file = nullptr;
    open_file = SObjectPtr<VFSFileResolver>::Create();

    IORequestResolverId open_dfile = nullptr;
    const bool dstorage = ds_reader.get();
    if (dstorage) 
    {
        open_dfile = SObjectPtr<DStorageFileResolver>::Create();
        chain->then(open_dfile);
    }

    chain->then(open_file)
        ->then(alloc_buffer);
        
    batch_buffer = SObjectPtr<IOBatchBuffer>::Create(); // hold batches

    batch_processors = { batch_buffer, chain };
    if (dstorage)
        batch_processors.push_back(ds_reader);
    request_processors = { vfs_reader };
}

} // namespace skr::io