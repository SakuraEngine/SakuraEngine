#pragma once
#include "../common/io_runnner.hpp"
#include "../common/io_resolver.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"
#include "containers/hashmap.hpp"

namespace skr {
namespace io {

struct RAMServiceImpl final : public IRAMService
{
    RAMServiceImpl(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    
    [[nodiscard]] IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT;
    [[nodiscard]] IORequestId open_request() SKR_NOEXCEPT;

    void set_resolvers(IOBatchResolverChainId chain) SKR_NOEXCEPT
    {
        runner.resolver_chain = skr::static_pointer_cast<IOBatchResolverChain>(chain);
    }

    RAMIOBufferId request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void request(IOBatchId request) SKR_NOEXCEPT;

    RAMIOBuffer allocate_buffer(uint64_t n) SKR_NOEXCEPT;
    void free_buffer(RAMIOBuffer* buffer) SKR_NOEXCEPT;
    
    void cancel(skr_io_future_t* future) SKR_NOEXCEPT 
    { 
        skr_atomicu32_store_relaxed(&future->request_cancel, 1); 
    }
    void stop(bool wait_drain = false) SKR_NOEXCEPT;
    void run() SKR_NOEXCEPT;
    void drain(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) SKR_NOEXCEPT;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT;
    SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT;

    // 7.2 finish callbacks are polled & executed by usr threads
    void poll_finish_callbacks() SKR_NOEXCEPT;

    struct Runner final : public RunnerBase
    {
        Runner(RAMServiceImpl* service) SKR_NOEXCEPT 
            : RunnerBase({ service->name.u8_str(), SKR_THREAD_ABOVE_NORMAL }),
            service(service)
        {
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                skr_atomicu64_store_relaxed(&ongoing_batch_counts[i], 0);
                skr_atomicu64_store_relaxed(&queued_batch_counts[i], 0);
            }
        }

        skr::AsyncResult serve() SKR_NOEXCEPT;

        // cancel request marked as request_cancel
        bool try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT;
        // 0. recycletry_cancel
        void recycle() SKR_NOEXCEPT;
        // 1. fetch requests from queue
        uint64_t fetch() SKR_NOEXCEPT;
        // 2. sort raw requests
        void sort() SKR_NOEXCEPT;
        // 3. resolve requests to pending raw request array
        void resolve() SKR_NOEXCEPT;
        // 5. dispatch I/O blocks to drives (+allocate & cpy to raw)
        void dispatch() SKR_NOEXCEPT;
        // 6. do uncompress works (+allocate & cpy to uncompressed)
        void uncompress() SKR_NOEXCEPT;
        // 7. finish
        void finish() SKR_NOEXCEPT;

        RAMServiceImpl* service = nullptr;
        SObjectPtr<IOBatchResolverChain> resolver_chain = nullptr;

        IOBatchArray ongoing_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        SAtomicU64 ongoing_batch_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    };
    const skr::string name;
    IOReaderId reader = nullptr;
    Runner runner;
protected:
    static uint32_t global_idx;

    SAtomicU64 request_sequence = 0;
    SAtomicU64 batch_sequence = 0;
    SmartPool<RAMIORequest, IIORequest> request_pool;
    SmartPool<RAMIOBatch, IIOBatch> batch_pool;
};

} // namespace io
} // namespace skr