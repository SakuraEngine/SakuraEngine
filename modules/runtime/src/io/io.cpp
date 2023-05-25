#include "io/io.h"
#include <EASTL/fixed_vector.h>

namespace skr {
namespace io {
struct IORequestBoxed
{
    skr::string path;
    skr_io_file_handle file;
    eastl::fixed_vector<skr_io_block_t, 1> blocks;
    eastl::fixed_vector<skr_io_compressed_block_t, 1> compressed_blocks;
    float sub_priority;
    skr_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];
};
    
struct RUNTIME_API IORequestQueue
    : public moodycamel::ConcurrentQueue<IORequestBoxed>
{

};

struct RUNTIME_API IORequestArray
    // : public skr::vector<IORequestBoxed>
{

};

struct RAMService2Impl final : public RAMService2
{
    RAMService2Impl(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void request(skr_vfs_t*, const skr_io_request_t* request, skr_io_future_t* future, skr_async_ram_destination_t* dst) SKR_NOEXCEPT;
    void cancel(skr_io_future_t* future) SKR_NOEXCEPT 
    { 
        skr_atomicu32_store_relaxed(&future->request_cancel, 1); 
    }
    void stop(bool wait_drain = false) SKR_NOEXCEPT;
    void run() SKR_NOEXCEPT;
    void drain() SKR_NOEXCEPT;
    void set_sleep_time(uint32_t time) SKR_NOEXCEPT;
    SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT;

    // 7.2 finish callbacks are polled & executed by usr threads
    void poll_finish_callbacks() SKR_NOEXCEPT;

    IORequestArray requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    IORequestQueue request_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomicU64 queued_request_count = 0;
    struct Runner
    {
        // 2. sort raw requests
        void sort() SKR_NOEXCEPT;

        // 3. resolve requests to pending raw request array
        void resolve() SKR_NOEXCEPT;

        // 4. chunk pending raw requests to block slices
        void chunkSingleRequest(IORequestBoxed& request) SKR_NOEXCEPT;
        void chunk() SKR_NOEXCEPT;

        // 5. dispatch I/O blocks to drives (+allocate & cpy to raw)
        void dispatch() SKR_NOEXCEPT;

        // 6. do uncompress works (+allocate & cpy to uncompressed)
        void uncompress() SKR_NOEXCEPT;
    };
};

skr_io_ram_service2_t* RAMService2::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    return SkrNew<RAMService2Impl>(desc);
}

void RAMService2::destroy(skr_io_ram_service2_t* service) SKR_NOEXCEPT
{
    SkrDelete(service);
}

void RAMService2Impl::request(skr_vfs_t* vfs, const skr_io_request_t *request, 
    skr_io_future_t *future, skr_async_ram_destination_t *dst) SKR_NOEXCEPT
{
    IORequestBoxed rq = {};
    rq.path = request->path;
    rq.file = request->file;
    rq.blocks = { request->blocks, request->blocks + request->block_count };
    rq.compressed_blocks = { request->compressed_blocks, request->compressed_blocks + request->compressed_block_count };
    rq.sub_priority = request->sub_priority;
    for (int i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; ++i)
    {
        rq.callbacks[i] = request->callbacks[i];
        rq.callback_datas[i] = request->callback_datas[i];
    }
    for (int i = 0; i < SKR_IO_FINISH_POINT_COUNT; ++i)
    {
        rq.finish_callbacks[i] = request->finish_callbacks[i];
        rq.finish_callback_datas[i] = request->finish_callback_datas[i];
    }
    request_queues[request->priority].enqueue(rq);
    skr_atomicu32_store_relaxed(&queued_request_count, 1); 
}

void RAMService2Impl::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (wait_drain)
    {
        drain();
    }
}

void RAMService2Impl::run() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void RAMService2Impl::drain() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();

}

void RAMService2Impl::set_sleep_time(uint32_t time) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();

}

SkrAsyncServiceStatus RAMService2Impl::get_service_status() const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return SkrAsyncServiceStatus::SKR_ASYNC_SERVICE_STATUS_COUNT;
}

void RAMService2Impl::poll_finish_callbacks() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();

}

void RAMService2Impl::Runner::sort() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();

}

} // namespace io
} // namespace skr