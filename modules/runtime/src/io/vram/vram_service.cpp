#include "SkrRT/async/wait_timeout.hpp"
#include "../dstorage/dstorage_resolvers.hpp"

#include "SkrRT/io/vram_io.hpp"
#include "vram_service.hpp"
#include "vram_resolvers.hpp"
#include "vram_readers.hpp"
#include "vram_batch.hpp"

namespace skr::io {

namespace VRAMUtils
{
inline static IOReaderId<IIOBatchProcessor> CreateCommonReader(VRAMService* service, const VRAMServiceDescriptor* desc) SKR_NOEXCEPT
{
    auto reader = skr::SObjectPtr<CommonVRAMReader>::Create(service, desc->ram_service);
    return std::move(reader);
}

inline static IOReaderId<IIOBatchProcessor> CreateDSReader(VRAMService* service, const VRAMServiceDescriptor* desc) SKR_NOEXCEPT
{
#ifdef _WIN32
    if (desc->gpu_device->adapter->instance->backend != CGPU_BACKEND_D3D12)
        return nullptr;
    if (skr_query_dstorage_availability() == SKR_DSTORAGE_AVAILABILITY_HARDWARE)
    {
        auto reader = skr::SObjectPtr<DStorageVRAMReader>::Create(service, desc->gpu_device);
        return std::move(reader);
    }
#endif
    return nullptr;
}
} // namespace RAMUtils

uint32_t VRAMService::global_idx = 0;
VRAMService::VRAMService(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT
    : name(desc->name ? skr::string(desc->name) : skr::format(u8"VRAMService-{}", global_idx++)), 
      awake_at_request(desc->awake_at_request),
      runner(this, desc->callback_job_queue),
      ram_service(desc->ram_service)
{
    slices_pool = VRAMRequestPool<ISlicesVRAMRequest>::Create(kIOPoolObjectsMemoryName);
    tiles_pool = VRAMRequestPool<ITilesVRAMRequest>::Create(kIOPoolObjectsMemoryName);
    blocks_pool = VRAMRequestPool<IBlocksVRAMRequest>::Create(kIOPoolObjectsMemoryName);

    vram_batch_pool = SmartPoolPtr<VRAMIOBatch, IIOBatch>::Create(kIOPoolObjectsMemoryName);
    
    vram_buffer_pool = SmartPoolPtr<VRAMBuffer, IVRAMIOBuffer>::Create(kIOPoolObjectsMemoryName);
    vram_texture_pool = SmartPoolPtr<VRAMTexture, IVRAMIOTexture>::Create(kIOPoolObjectsMemoryName);

    if (desc->use_dstorage)
    {
        runner.ds_reader = VRAMUtils::CreateDSReader(this, desc);
    }
    runner.common_reader = VRAMUtils::CreateCommonReader(this, desc);
    runner.set_resolvers();

    if ((!desc->awake_at_request) && (desc->sleep_time > 2000))
    {
        SKR_ASSERT(desc->sleep_time <= 2000);
        SKR_LOG_FATAL(u8"RAMService: too long sleep_time causes 'deadlock' when awake_at_request is false");
    }
    runner.set_sleep_time(desc->sleep_time);
}

IVRAMService* IVRAMService::create(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT
{
    return SkrNew<VRAMService>(desc);
}

void IVRAMService::destroy(IVRAMService* service) SKR_NOEXCEPT
{
    SkrZoneScopedN("VRAMService::destroy");

    auto S = static_cast<VRAMService*>(service);
    S->runner.destroy();
    SkrDelete(S);
}

IOBatchId VRAMService::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(vram_batch_pool->allocate(this, seq, n));
}

SlicesIORequestId VRAMService::open_texture_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<ISlicesVRAMRequest>(slices_pool->allocate(this, seq));
}

BlocksVRAMRequestId VRAMService::open_buffer_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IBlocksVRAMRequest>(blocks_pool->allocate(this, seq));
}

void VRAMService::request(IOBatchId batch) SKR_NOEXCEPT
{
    runner.enqueueBatch(batch);
    if (awake_at_request)
    {
        runner.awake();
    }
}

VRAMIOBufferId VRAMService::request(BlocksVRAMRequestId request, IOFuture* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch = open_batch(1);
    auto result = batch->add_request(request, future);
    auto buffer = skr::static_pointer_cast<IVRAMIOBuffer>(result);
    batch->set_priority(priority);
    this->request(batch);
    return buffer;
}
    
VRAMIOTextureId VRAMService::request(SlicesIORequestId request, IOFuture* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch = open_batch(1);
    auto result = batch->add_request(request, future);
    auto texture = skr::static_pointer_cast<IVRAMIOTexture>(result);
    batch->set_priority(priority);
    this->request(batch);
    return texture;
}
    
void VRAMService::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (wait_drain)
    {
        drain();
    }
    runner.stop();
}

void VRAMService::run() SKR_NOEXCEPT
{
    runner.run();
}

void VRAMService::drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    runner.drain(priority);    
    {
        SkrZoneScopedN("VRAMService::server_drain");
        auto predicate = [this, priority] {
            return !runner.processing_count(priority);
        };
        bool fatal = !::wait_timeout(predicate, 5);
        if (fatal)
        {
            SKR_LOG_FATAL(u8"VRAMService: drain timeout, %llu requests are still processing", 
                runner.processing_count(priority));
        }
    }
}

void VRAMService::set_sleep_time(uint32_t ms) SKR_NOEXCEPT
{
    runner.set_sleep_time(ms);
}

SkrAsyncServiceStatus VRAMService::get_service_status() const SKR_NOEXCEPT
{
    return runner.getServiceStatus();
}

void VRAMService::poll_finish_callbacks() SKR_NOEXCEPT
{
    runner.poll_finish_callbacks();
}

VRAMService::Runner::Runner(VRAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT
    : RunnerBase({ service->name.u8_str(), SKR_THREAD_ABOVE_NORMAL }, job_queue),
    service(service)
{

}

void VRAMService::Runner::enqueueBatch(const IOBatchId& batch) SKR_NOEXCEPT
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

void VRAMService::Runner::set_resolvers() SKR_NOEXCEPT
{
    auto chain = skr::static_pointer_cast<IORequestResolverChain>(IIORequestResolverChain::Create());
    chain->runner = this;

    auto alloc_resource = SObjectPtr<AllocateVRAMResourceResolver>::Create();
    chain->then(alloc_resource); // VRAMService: we open dstorage files in this resolver
    batch_buffer = SObjectPtr<IOBatchBuffer>::Create(); // hold batches
    batch_processors = { batch_buffer, chain, common_reader };
    if (const bool ds_available = ds_reader.get())
    {
        batch_processors.push_back(ds_reader);
    }
}
} // namespace skr::io