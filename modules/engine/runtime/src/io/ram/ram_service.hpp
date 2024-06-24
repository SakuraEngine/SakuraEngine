#pragma once
#include "../common/io_runnner.hpp"
#include "../common/processors.hpp"
#include "ram_batch.hpp"
#include "ram_request.hpp"
#include "ram_buffer.hpp"

namespace skr {
namespace io {

struct RAMService final : public IRAMService
{
    RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    
    [[nodiscard]] IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT;
    [[nodiscard]] BlocksRAMRequestId open_request() SKR_NOEXCEPT;
    RAMIOBufferId request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void request(IOBatchId request) SKR_NOEXCEPT;
    
    void cancel(skr_io_future_t* future) SKR_NOEXCEPT 
    { 
        skr_atomic_store_relaxed(&future->request_cancel, 1); 
    }
    void stop(bool wait_drain = false) SKR_NOEXCEPT;
    void run() SKR_NOEXCEPT;
    void drain(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) SKR_NOEXCEPT;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT;
    SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT;
    void poll_finish_callbacks() SKR_NOEXCEPT;

    struct Runner final : public RunnerBase
    {
        Runner(RAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT; 

        void enqueueBatch(const IOBatchId& batch) SKR_NOEXCEPT;
        void set_resolvers() SKR_NOEXCEPT;

        IOBatchBufferId batch_buffer = nullptr;
        IOReaderId<IIORequestProcessor> vfs_reader = nullptr;
        IOReaderId<IIOBatchProcessor> ds_reader = nullptr;
        RAMService* service = nullptr;
    };
    const skr::String name;
    const bool awake_at_request = false;
    Runner runner;
    
    SmartPoolPtr<RAMRequestMixin, IBlocksRAMRequest> request_pool = nullptr;
    SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer> ram_buffer_pool = nullptr;
    SmartPoolPtr<RAMIOBatch, IIOBatch> ram_batch_pool = nullptr;
    
private:
    RAMIOBuffer allocateBuffer(uint64_t n) SKR_NOEXCEPT;
    void freeBuffer(RAMIOBuffer* buffer) SKR_NOEXCEPT;

    static uint32_t global_idx;
    SAtomicU64 request_sequence = 0;
    SAtomicU64 batch_sequence = 0;
};

} // namespace io
} // namespace skr