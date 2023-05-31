#include "io_runnner.hpp"

namespace skr {
namespace io {

struct RAMServiceImpl final : public RAMService
{
    RAMServiceImpl(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    {
        
    }
    [[nodiscard]] IORequest open_request() SKR_NOEXCEPT;

    uint64_t add_resolver(const char8_t* name, RequestResolver resolver) SKR_NOEXCEPT
    {
        skr_rw_mutex_acuire_w(&runner.resolvers_mutex);
        const auto id = runner.resolvers.size();
        runner.resolvers.emplace_back(name, resolver);
        skr_rw_mutex_release(&runner.resolvers_mutex);
        return id;
    }

    void request(IORequest request, skr_io_future_t* future, skr_async_ram_destination_t* dst) SKR_NOEXCEPT;
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

    struct Runner final : public skr::ServiceThread
    {
        const bool condsleep = false;
        static uint32_t global_idx;
        Runner() SKR_NOEXCEPT 
            : skr::ServiceThread({ skr::format(u8"RAMService-{}", global_idx++).u8_str(), SKR_THREAD_ABOVE_NORMAL }) 
        {
            skr_init_rw_mutex(&resolvers_mutex);
            condlock.initialize(skr::format(u8"RAMServiceCondLock-{}", global_idx++).u8_str());
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                skr_atomicu64_store_relaxed(&request_counts[i], 0);
                skr_atomicu64_store_relaxed(&queued_request_counts[i], 0);
            }
        }

        ~Runner() SKR_NOEXCEPT
        {
            skr_destroy_rw_mutex(&resolvers_mutex);
        }

        skr::AsyncResult serve() SKR_NOEXCEPT;
        void sleep() SKR_NOEXCEPT
        {
            const auto ms = skr_atomicu64_load_relaxed(&sleep_time);
            if (!condsleep)
            {
                ZoneScopedNC("ioServiceSleep(Sleep)", tracy::Color::Gray55);
                skr_thread_sleep(ms);
            }
            else
            {
                ZoneScopedNC("ioServiceSleep(Cond)", tracy::Color::Gray55);
                condlock.lock();
                condlock.wait(ms);
                condlock.unlock();
            }
        }

        // cancel request marked as request_cancel
        bool try_cancel(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT;
        // 0. recycle
        void recycle() SKR_NOEXCEPT;
        // 1. fetch requests from queue
        uint64_t fetch() SKR_NOEXCEPT;
        // 2. sort raw requests
        void sort() SKR_NOEXCEPT;
        // 3. resolve requests to pending raw request array
        void resolve() SKR_NOEXCEPT;
        // 5. dispatch I/O blocks to drives (+allocate & cpy to raw)
        void dispatch() SKR_NOEXCEPT;
        void dispatch_read() SKR_NOEXCEPT;
        void dispatch_close() SKR_NOEXCEPT;
        // 6. do uncompress works (+allocate & cpy to uncompressed)
        void uncompress() SKR_NOEXCEPT;
        // 7. finish
        void finish() SKR_NOEXCEPT;

        SRWMutex resolvers_mutex;
        eastl::vector<eastl::pair<skr::string, RequestResolver>> resolvers;

        IORequestQueue request_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        SAtomicU64 queued_request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

        IORequestArray requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        SAtomicU64 request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        
        IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        
        SAtomicU32 sleep_time = 16u;
        SAtomicU32 service_status = SKR_ASYNC_SERVICE_STATUS_SLEEPING;
        CondLock condlock;
    };
    Runner runner;
    SmartPool<RAMIORequest, IIORequest> request_pool;
    SAtomicU64 sequence_number = 0;

    SmartPool<IOBatchBase, IIOBatch> batch_pool;
};
uint32_t RAMServiceImpl::Runner::global_idx = 0;

skr_io_ram_service_t* RAMService::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto srv = SkrNew<RAMServiceImpl>(desc);
    srv->run();
    return srv;
}

void RAMService::destroy(skr_io_ram_service_t* service) SKR_NOEXCEPT
{
    ZoneScopedN("destroy");

    auto S = static_cast<RAMServiceImpl*>(service);
    if (S->runner.get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        S->drain();
        skr_atomicu32_store_relaxed(&S->runner.service_status, SKR_ASYNC_SERVICE_STATUS_QUITING);
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

IORequest RAMServiceImpl::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&sequence_number, 1);
    return skr::static_pointer_cast<IIORequest>(request_pool.allocate(seq));
}

void RAMServiceImpl::request(IORequest request, skr_io_future_t *future, skr_async_ram_destination_t *dst) SKR_NOEXCEPT
{
    auto rq = skr::static_pointer_cast<RAMIORequest>(request);

    SKR_ASSERT(!rq->blocks.empty());

    rq->future = future;
    rq->destination = dst;

    const auto pri = rq->get_priority();
    runner.request_queues[pri].enqueue(rq);
    rq->setStatus(SKR_IO_STAGE_ENQUEUED);
    skr_atomicu32_add_relaxed(&runner.queued_request_counts[pri], 1);

    if (runner.condsleep)
    {
        runner.condlock.signal();
    } 
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
        while (skr_atomicu64_load_relaxed(&runner.queued_request_counts[priority]) > 0)
        {
            // ...
        }
        while (skr_atomicu64_load_relaxed(&runner.request_counts[priority]) > 0)
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
    skr_atomicu32_store_relaxed(&runner.sleep_time, ms);
}

SkrAsyncServiceStatus RAMServiceImpl::get_service_status() const SKR_NOEXCEPT
{
    const auto s = skr_atomicu32_load_relaxed(&runner.service_status);
    return (SkrAsyncServiceStatus)s;
}

void RAMServiceImpl::poll_finish_callbacks() SKR_NOEXCEPT
{
    RQPtr rq = nullptr;
    while (runner.finish_queues->try_dequeue(rq))
    {
        if (rq->getStatus() == SKR_IO_STAGE_COMPLETED)
        {
            rq->finish_callbacks[SKR_IO_FINISH_POINT_COMPLETE](
                rq->future, nullptr, rq->finish_callback_datas[SKR_IO_FINISH_POINT_COMPLETE]);
        }
        else
        {
            rq->finish_callbacks[SKR_IO_FINISH_POINT_CANCEL](
                rq->future, nullptr, rq->finish_callback_datas[SKR_IO_FINISH_POINT_CANCEL]);
        }
        skr_atomicu32_store_relaxed(&rq->done, SKR_ASYNC_IO_DONE_STATUS_DONE);
    }
}

} // namespace io
} // namespace skr

namespace skr {
namespace io {

skr::AsyncResult RAMServiceImpl::Runner::serve() SKR_NOEXCEPT
{
    SKR_DEFER( { recycle(); } );

    fetch();
    
    uint64_t cnt = 0;
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        cnt += skr_atomicu64_load_relaxed(&request_counts[i]);
    }
    
    if (cnt)
    {
        ZoneScopedN("Serve");

        skr_atomicu32_store_relaxed(&service_status, SKR_ASYNC_SERVICE_STATUS_RUNNING);
        sort();
        resolve();
        dispatch();
        uncompress();
        finish();
        return ASYNC_RESULT_OK;
    }
    else
    {
        skr_atomicu32_store_relaxed(&service_status, SKR_ASYNC_SERVICE_STATUS_SLEEPING);
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
            bool need_finish = false;
            for (auto f : rq->finish_callbacks)
            {
                if (f)
                {
                    need_finish = true;
                }
            }
            if (need_finish)
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
        auto& arr = requests[i];
        auto it = eastl::remove_if(arr.begin(), arr.end(), 
            [](const RQPtr& rq) { 
                return (skr_atomicu32_load_relaxed(&rq->done) == SKR_ASYNC_IO_DONE_STATUS_DONE); 
            });
        const int64_t X = (int64_t)arr.size();
        arr.erase(it, arr.end());
        const int64_t Y = (int64_t)arr.size();
        skr_atomicu64_add_relaxed(&request_counts[i], Y - X);
    }
}

uint64_t RAMServiceImpl::Runner::fetch() SKR_NOEXCEPT
{
    ZoneScopedN("fetch");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        RQPtr rq = nullptr;
        auto& queue = request_queues[i];
        while (queue.try_dequeue(rq))
        {
            auto& arr = requests[i];
            arr.emplace_back(rq);

            skr_atomicu64_add_relaxed(&request_counts[i], 1);
            skr_atomicu64_add_relaxed(&queued_request_counts[i], -1);
        }
    }
    return 0;
}

void RAMServiceImpl::Runner::sort() SKR_NOEXCEPT
{
    ZoneScopedN("sort");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        std::sort(arr.begin(), arr.end(), 
        [](const RQPtr& a, const RQPtr& b) {
            return a->sub_priority > b->sub_priority;
        });
    }
}

void RAMServiceImpl::Runner::dispatch() SKR_NOEXCEPT
{
    dispatch_read();
    dispatch_close();
}

void RAMServiceImpl::Runner::resolve() SKR_NOEXCEPT
{
    ZoneScopedN("resolve");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (try_cancel((SkrAsyncServicePriority)i, rq))
            {
                // cancel...
            }
            else if (rq->getStatus() == SKR_IO_STAGE_ENQUEUED)
            {
                // SKR_LOG_DEBUG("dispatch open request: %s", rq->path.c_str());
                rq->setStatus(SKR_IO_STAGE_RESOLVING);
                
                // TODO: resolver ordering

                // TODO: resolver jobs
                skr_rw_mutex_acuire_r(&resolvers_mutex);
                for (auto&& [id, resolver] : resolvers)
                {
                    resolver(rq);
                }
                skr_rw_mutex_release(&resolvers_mutex);
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch open request: %s, skip", rq->path.c_str());
            }
        }
    }
}

uint64_t RAMService::add_file_resolver() SKR_NOEXCEPT
{ 
    return add_resolver(u8"file", [](IORequest request) { request->open_file(); }); 
}

uint64_t RAMService::add_iobuffer_resolver() SKR_NOEXCEPT
{
    const auto id = add_resolver(u8"iobuffer", [](IORequest request) {
        auto rq = skr::static_pointer_cast<RAMIORequest>(request);
        // deal with 0 block size
        for (auto& block : rq->blocks)
        {
            if (!block.size)
            {
                block.size = rq->get_fsize() - block.offset;
            }
            if (!rq->destination->size)
            {
                rq->destination->size += block.size;
            }
        }
        // allocate
        if (!rq->destination->bytes)
        {
            if (!rq->destination->size)
            {
                SKR_ASSERT(0 && "invalid destination size");
            }
            ZoneScopedNC("IOBufferAllocate", tracy::Color::BlueViolet);
            rq->destination->bytes = (uint8_t*)sakura_malloc(rq->destination->size);
        }
    });
    // this->arrange_after(id, "file");
    return id;
}

uint64_t RAMService::add_chunking_resolver(uint64_t chunk_size) SKR_NOEXCEPT
{
    const auto id = add_resolver(u8"chunking", [chunk_size](IORequest request) {
        auto rq = skr::static_pointer_cast<RAMIORequest>(request);
        uint64_t total = 0;
        for (auto& block : rq->get_blocks())
            total += block.size;
        uint64_t chunk_count = total / chunk_size;
        if (chunk_count > 2)
        {
            auto bks = rq->blocks;
            rq->reset_blocks();
            rq->blocks.reserve(chunk_count);
            for (auto& block : bks)
            {
                uint64_t acc_size = block.size;
                uint64_t acc_offset = block.offset;
                while (acc_size > chunk_size)
                {
                    rq->add_block({acc_offset, chunk_size});
                    acc_offset += chunk_size;
                    acc_size -= chunk_size;
                }
                rq->get_blocks().back().size += acc_size;
            }
        }
    });
    // this->arrange_after(id, "iobuffer");
    return id;
}

void RAMServiceImpl::Runner::dispatch_read() SKR_NOEXCEPT
{
    ZoneScopedN("dispatch_read");
    
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (try_cancel((SkrAsyncServicePriority)i, rq))
            {
                // cancel...
                if (rq->file) skr_vfs_fclose(rq->file);
                if (rq->destination->bytes) sakura_free(rq->destination->bytes);
            }
            else if (rq->getStatus() == SKR_IO_STAGE_RESOLVING)
            {
                ZoneScopedN("read_request");

                rq->setStatus(SKR_IO_STAGE_LOADING);
                // SKR_LOG_DEBUG("dispatch read request: %s", rq->path.c_str());
                uint64_t dst_offset = 0u;
                for (const auto& block : rq->blocks)
                {
                    const auto address = rq->destination->bytes + dst_offset;
                    skr_vfs_fread(rq->file, address, block.offset, block.size);
                    dst_offset += block.size;
                }
                rq->setStatus(SKR_IO_STAGE_LOADED);
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch read request: %s, skip", rq->path.c_str());
            }
        }
    }
}

void RAMService::add_default_resolvers() SKR_NOEXCEPT
{
    add_file_resolver();
    add_iobuffer_resolver();
    // add_chunking_resolver(); // need to chunk, sort and order-by-offset on HDD platforms
}

void RAMServiceImpl::Runner::dispatch_close() SKR_NOEXCEPT
{
    ZoneScopedN("dispatch_close");

    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (rq->file)
            {
                // SKR_LOG_DEBUG("dispatch close request: %s", rq->path.c_str());
                skr_vfs_fclose(rq->file);
                rq->file = nullptr;
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch close request: %s, skip", rq->path.c_str());
            }
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
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            const auto d = skr_atomicu32_load_relaxed(&rq->done);
            if (d == 0)
            {
                rq->setStatus(SKR_IO_STAGE_COMPLETED);
                bool need_finish = false;
                for (auto f : rq->finish_callbacks)
                {
                    if (f)
                        need_finish = true;
                }
                if (need_finish)
                {
                    finish_queues[i].enqueue(rq);
                    skr_atomicu32_store_relaxed(&rq->done, SKR_ASYNC_IO_DONE_STATUS_PENDING);
                }
                else
                {
                    skr_atomicu32_store_relaxed(&rq->done, SKR_ASYNC_IO_DONE_STATUS_DONE);
                }
            }
        }
    }
}

} // namespace io
} // namespace skr