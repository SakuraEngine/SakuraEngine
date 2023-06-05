#pragma once
#include "../common/io_runnner.hpp"
#include "../common/io_resolver.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"
#include "containers/hashmap.hpp"

namespace skr {
namespace io {

struct RAMService final : public IRAMService
{
    RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    
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
        Runner(RAMService* service, SObjectPtr<IIOReader> reader) SKR_NOEXCEPT 
            : RunnerBase({ service->name.u8_str(), SKR_THREAD_ABOVE_NORMAL }, reader),
            service(service)
        {

        }
        skr::AsyncResult serve() SKR_NOEXCEPT;
        RAMService* service = nullptr;
    };
    const skr::string name;
    const bool trace_log = false;
    const bool awake_at_request = false;
    Runner runner;
protected:
    static uint32_t global_idx;
    SAtomicU64 request_sequence = 0;
    SAtomicU64 batch_sequence = 0;
    SmartPool<RAMIORequest, IIORequest> request_pool;
};

} // namespace io
} // namespace skr