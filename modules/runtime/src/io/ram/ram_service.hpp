#pragma once
#include "../common/io_runnner.hpp"
#include "../common/batch_processors.hpp"
#include "ram_readers.hpp"
#include "ram_batch.hpp"
#include "ram_request.hpp"
#include "ram_buffer.hpp"

namespace skr {
namespace io {

struct RAMService final : public IRAMService
{
    RAMService(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    
    [[nodiscard]] IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT;
    [[nodiscard]] IORequestId open_request() SKR_NOEXCEPT;

    void set_resolvers(IORequestResolverChainId chain) SKR_NOEXCEPT
    {
        runner.set_resolvers(chain);
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
        Runner(RAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT 
            : RunnerBase({ service->name.u8_str(), SKR_THREAD_ABOVE_NORMAL }, job_queue),
            service(service)
        {
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                skr_atomic64_store_relaxed(&processing_request_counts[i], 0);
            }
        }

        void enqueueBatch(const IOBatchId& batch)
        {
            const auto priority = batch->get_priority();
            batch_buffer->fetch(priority, batch);
            for (auto&& request : batch->get_requests())
            {
                auto&& rq = skr::static_pointer_cast<IORequestBase>(request);
                rq->setStatus(SKR_IO_STAGE_ENQUEUED);
            }
            skr_atomic64_add_relaxed(&processing_request_counts[priority], 1);
        }

        uint64_t getQueuedBatchCount(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
        {
            return batch_buffer->processing_count();
        }

        uint64_t getExecutingBatchCount(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
        {
            return reader->processing_count();
        }

        void set_resolvers(IORequestResolverChainId _chain) SKR_NOEXCEPT
        {
            auto chain = skr::static_pointer_cast<IORequestResolverChain>(_chain);
            chain->runner = this;

            resolver_chain = chain;
            batch_buffer = SObjectPtr<IOBatchBuffer>::Create();
            batch_processors = { batch_buffer, resolver_chain };
            request_processors = { reader };
        }

        // TODO: REMOVE THESE
        uint64_t getProcessingRequestCount(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
        {
            if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
            {
                return skr_atomic64_load_relaxed(&processing_request_counts[priority]);
            }
            else
            {
                uint64_t count = 0;
                for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
                {
                    count += skr_atomic64_load_relaxed(&processing_request_counts[i]);
                }
                return count;
            }
        }
        bool cancelFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT override
        {
            auto ret = RunnerBase::cancelFunction(rq, priority);
            skr_atomic64_add_relaxed(&processing_request_counts[priority], -1);
            return ret;
        }
        bool completeFunction(skr::SObjectPtr<IORequestBase> rq, SkrAsyncServicePriority priority) SKR_NOEXCEPT override
        {
            auto ret = RunnerBase::completeFunction(rq, priority);
            skr_atomic64_add_relaxed(&processing_request_counts[priority], -1);
            return ret;
        }
        SAtomic64 processing_request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        // END TODO

        IOBatchBufferId batch_buffer = nullptr;
        IORequestResolverChainId resolver_chain = nullptr;
        IOReaderId<IIORequestProcessor> reader = nullptr;

        RAMService* service = nullptr;
    };
    const skr::string name;
    const bool awake_at_request = false;
    Runner runner;
    
    SmartPoolPtr<RAMIORequest, IIORequest> request_pool = nullptr;
    SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer> ram_buffer_pool = nullptr;
    SmartPoolPtr<RAMIOBatch, IIOBatch> ram_batch_pool = nullptr;
protected:
    static uint32_t global_idx;
    SAtomicU64 request_sequence = 0;
    SAtomicU64 batch_sequence = 0;
};

} // namespace io
} // namespace skr