#include "io/io.h"
#include "platform/vfs.h"
#include "misc/log.h"
#include "async/service_thread.hpp"
#include "containers/hashmap.hpp"
#include "containers/sptr.hpp"
#include <EASTL/fixed_vector.h>

namespace skr {
namespace io {
struct IORequestBoxed : public skr::SInterface
{
    skr_vfs_t* vfs = nullptr;
    skr::string path;
    skr_io_file_handle file;
    eastl::fixed_vector<skr_io_block_t, 1> blocks;
    eastl::fixed_vector<skr_io_compressed_block_t, 1> compressed_blocks;
    float sub_priority;

    SAtomic32 done = 0;
    skr_io_future_t* future = nullptr;
    skr_async_ram_destination_t* destination = nullptr;

    skr_io_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];

    eastl::fixed_vector<uint64_t, 1> cids;

    void setStatus(SkrAsyncIOStatus status)
    {
        skr_atomicu32_store_relaxed(&future->status, status);
        if (const auto callback = callbacks[status])
        {
            callback(future, nullptr, callback_datas[status]);
        }
    }

    SkrAsyncIOStatus getStatus() const
    {
        return static_cast<SkrAsyncIOStatus>(skr_atomicu32_load_relaxed(&future->status));
    }

    uint32_t add_refcount() 
    { 
        return 1 + skr_atomicu32_add_relaxed(&rc, 1); 
    }

    uint32_t release() 
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }
private:
    SAtomicU32 rc = 0;
};

using RQPtr = skr::SObjectPtr<IORequestBoxed>;
using IORequestQueue = moodycamel::ConcurrentQueue<RQPtr>;  
using IORequestArray = skr::vector<RQPtr>;

using CHKPtr = skr::SPtr<IORequstChunk>;
using IOChunkArray = skr::vector<CHKPtr>;
using IOChunkMap = skr::parallel_flat_hash_map<uint64_t, IORequstChunk*>;

IOChunkMap chunkmap;
IORequstChunk::IORequstChunk() SKR_NOEXCEPT
    : uid(skr_atomicu64_add_relaxed(&s_uid, 1))
{
    chunkmap.insert( {uid, this} );
}

IORequstChunk::~IORequstChunk() SKR_NOEXCEPT
{
    chunkmap.erase(uid);
}

SAtomicU64 IORequstChunk::s_uid = 0;

struct RAMService2Impl final : public RAMService2
{
    RAMService2Impl(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    {
        
    }

    void request(skr_vfs_t*, const skr_io_request_t* request, skr_io_future_t* future, skr_async_ram_destination_t* dst) SKR_NOEXCEPT;
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
        static uint32_t global_idx;
        Runner() SKR_NOEXCEPT 
            : skr::ServiceThread({ skr::format(u8"RAMService-{}", global_idx++).u8_str() }) 
        {
            for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
            {
                skr_atomicu64_store_relaxed(&request_counts[i], 0);
                skr_atomicu64_store_relaxed(&queued_request_counts[i], 0);
            }
        }

        skr::AsyncResult serve() SKR_NOEXCEPT;
        void sleep() SKR_NOEXCEPT
        {
            // TODO: sleep with condvar
            const auto ms = skr_atomicu64_load_relaxed(&sleep_time);
            skr_thread_sleep(ms);
        }

        // 0. recycle
        void recycle() SKR_NOEXCEPT;

        // 1. fetch requests from queue
        uint64_t fetch() SKR_NOEXCEPT;

        // 2. sort raw requests
        void sort() SKR_NOEXCEPT;

        // 3. resolve requests to pending raw request array
        void resolve() SKR_NOEXCEPT;

        // 4. chunk pending raw requests to block slices
        void chunk() SKR_NOEXCEPT;
        void chunkSingleRequest(SkrAsyncServicePriority priority, RQPtr request) SKR_NOEXCEPT;

        // 5. dispatch I/O blocks to drives (+allocate & cpy to raw)
        void dispatch() SKR_NOEXCEPT;
        void dispatch_open() SKR_NOEXCEPT;
        void dispatch_read() SKR_NOEXCEPT;
        void dispatch_close() SKR_NOEXCEPT;

        // 6. do uncompress works (+allocate & cpy to uncompressed)
        void uncompress() SKR_NOEXCEPT;

        // 7. finish
        void finish() SKR_NOEXCEPT;

        IORequestQueue request_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        SAtomicU64 queued_request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

        IORequestArray requests[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        SAtomicU64 request_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        IOChunkArray chunks[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        
        IORequestQueue finish_queues[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
        
        SAtomicU32 sleep_time = 0u;
        SAtomicU32 service_status = SKR_ASYNC_SERVICE_STATUS_SLEEPING;
    };
    Runner runner;
};
uint32_t RAMService2Impl::Runner::global_idx = 0;

skr_io_ram_service2_t* RAMService2::create(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    return SkrNew<RAMService2Impl>(desc);
}

void RAMService2::destroy(skr_io_ram_service2_t* service) SKR_NOEXCEPT
{
    auto S = static_cast<RAMService2Impl*>(service);
    if (S->runner.get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        S->drain();
        skr_atomicu32_store_relaxed(&S->runner.service_status, SKR_ASYNC_SERVICE_STATUS_SLEEPING);
        S->stop(false);
    }
    S->runner.wait_stop();
    S->runner.exit();
    SkrDelete(service);
}

void RAMService2Impl::request(skr_vfs_t* vfs, const skr_io_request_t *request, 
    skr_io_future_t *future, skr_async_ram_destination_t *dst) SKR_NOEXCEPT
{
    auto rq = RQPtr::CreateZeroed();
    rq->vfs = vfs;
    rq->future = future;
    rq->destination = dst;

    rq->path = request->path;
    rq->file = request->file;
    rq->blocks = { request->blocks, request->blocks + request->block_count };
    rq->compressed_blocks = { request->compressed_blocks, request->compressed_blocks + request->compressed_block_count };
    rq->sub_priority = request->sub_priority;
    for (int i = 0; i < SKR_ASYNC_IO_STATUS_COUNT; ++i)
    {
        rq->callbacks[i] = request->callbacks[i];
        rq->callback_datas[i] = request->callback_datas[i];
    }
    for (int i = 0; i < SKR_IO_FINISH_POINT_COUNT; ++i)
    {
        rq->finish_callbacks[i] = request->finish_callbacks[i];
        rq->finish_callback_datas[i] = request->finish_callback_datas[i];
    }

    runner.request_queues[request->priority].enqueue(rq);
    rq->setStatus(SKR_ASYNC_IO_STATUS_ENQUEUED);
    skr_atomicu32_add_relaxed(&runner.queued_request_counts, 1); 
}

void RAMService2Impl::stop(bool wait_drain) SKR_NOEXCEPT
{
    if (wait_drain)
    {
        drain();
    }
    runner.stop();
}

void RAMService2Impl::run() SKR_NOEXCEPT
{
    runner.run();
}

void RAMService2Impl::drain(SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
    {
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

void RAMService2Impl::set_sleep_time(uint32_t ms) SKR_NOEXCEPT
{
    skr_atomicu32_store_relaxed(&runner.sleep_time, ms);
}

SkrAsyncServiceStatus RAMService2Impl::get_service_status() const SKR_NOEXCEPT
{
    const auto s = skr_atomicu32_load_relaxed(&runner.service_status);
    return (SkrAsyncServiceStatus)s;
}

void RAMService2Impl::poll_finish_callbacks() SKR_NOEXCEPT
{
    RQPtr rq = nullptr;
    while (runner.finish_queues->try_dequeue(rq))
    {
        if (rq->getStatus() == SKR_ASYNC_IO_STATUS_OK)
        {
            rq->finish_callbacks[SKR_IO_FINISH_POINT_COMPLETE](
                rq->future, nullptr, rq->finish_callback_datas[SKR_IO_FINISH_POINT_COMPLETE]);
        }
        else
        {
            rq->finish_callbacks[SKR_IO_FINISH_POINT_CANCEL](
                rq->future, nullptr, rq->finish_callback_datas[SKR_IO_FINISH_POINT_CANCEL]);
        }
        skr_atomicu32_store_relaxed(&rq->done, 1);
    }
}

} // namespace io
} // namespace skr

namespace skr {
namespace io {

skr::AsyncResult RAMService2Impl::Runner::serve() SKR_NOEXCEPT
{
    recycle();

    fetch();
    
    uint64_t cnt = 0;
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        cnt += skr_atomicu64_load_relaxed(&request_counts[i]);
    }
    
    if (cnt)
    {
        skr_atomicu32_store_relaxed(&service_status, SKR_ASYNC_SERVICE_STATUS_RUNNING);
        sort();
        resolve();
        chunk();
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

void RAMService2Impl::Runner::recycle() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        auto it = eastl::remove_if(arr.begin(), arr.end(), 
            [](const RQPtr& rq) { 
                return skr_atomicu32_load_relaxed(&rq->done); 
            });
        const int64_t X = (int64_t)arr.size();
        arr.erase(it, arr.end());
        const int64_t Y = (int64_t)arr.size();
        skr_atomicu64_add_relaxed(&request_counts[i], Y - X);
    }
}

uint64_t RAMService2Impl::Runner::fetch() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        RQPtr request = nullptr;
        auto& queue = request_queues[i];
        while (queue.try_dequeue(request))
        {
            auto& arr = requests[i];
            arr.emplace_back(request);

            skr_atomicu64_add_relaxed(&request_counts[i], 1);
            skr_atomicu64_add_relaxed(&queued_request_counts[i], -1);
        }
    }
    return 0;
}

void RAMService2Impl::Runner::sort() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        std::sort(arr.begin(), arr.end(), 
        [](const RQPtr& a, const RQPtr& b) {
            return a->sub_priority > b->sub_priority;
        });
    }
}

void RAMService2Impl::Runner::chunk() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            chunkSingleRequest((SkrAsyncServicePriority)i, rq);
        }
    }
}

void RAMService2Impl::Runner::chunkSingleRequest(SkrAsyncServicePriority priority, RQPtr rq) SKR_NOEXCEPT
{
    if (rq->cids.empty())
    {
        auto chk = CHKPtr::CreateZeroed();

        chk->blocks = rq->blocks;
        chk->compressed_blocks = rq->compressed_blocks;

        chunks[priority].emplace_back(chk);
        rq->cids.emplace_back(chk->uid);
    }
}

void RAMService2Impl::Runner::dispatch() SKR_NOEXCEPT
{
    dispatch_read();
    dispatch_close();
}

void RAMService2Impl::Runner::resolve() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (rq->getStatus() == SKR_ASYNC_IO_STATUS_ENQUEUED)
            {
                SKR_LOG_DEBUG("dispatch open request: %s", rq->path.c_str());
                rq->setStatus(SKR_ASYNC_IO_STATUS_CREATING_RESOURCE);
                
                if (!rq->file)
                {
                    rq->file = skr_vfs_fopen(rq->vfs, 
                        rq->path.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
                }

                for (auto& block : rq->blocks)
                {
                    if (!block.size)
                    {
                        block.size = skr_vfs_fsize(rq->file) - block.offset;
                    }
                    rq->destination->size += block.size;
                }

                if (!rq->destination->bytes)
                {
                    if (!rq->destination->size)
                    {
                        SKR_ASSERT(0 && "invalid destination size");
                    }
                    rq->destination->bytes = (uint8_t*)sakura_malloc(rq->destination->size);
                }
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch open request: %s, skip", rq->path.c_str());
            }
        }
    }
}

void RAMService2Impl::Runner::dispatch_read() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (rq->getStatus() == SKR_ASYNC_IO_STATUS_CREATING_RESOURCE)
            {
                rq->setStatus(SKR_ASYNC_IO_STATUS_RAM_LOADING);

                SKR_LOG_DEBUG("dispatch read request: %s", rq->path.c_str());
                for (const auto cid : rq->cids)
                {
                    const auto chk = chunkmap[cid];
                    for (auto block : chk->blocks)
                    {
                        skr_vfs_fread(rq->file, rq->destination->bytes, block.offset, block.size);
                    }

                    auto it = eastl::remove_if(chunks[i].begin(), chunks[i].end(),
                        [cid](const CHKPtr& p) { return p->uid == cid; });
                    chunks[i].erase(it, chunks[i].end());
                }

                rq->setStatus(SKR_ASYNC_IO_STATUS_OK);
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch read request: %s, skip", rq->path.c_str());
            }
        }
    }
}

void RAMService2Impl::Runner::dispatch_close() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (rq->getStatus() == SKR_ASYNC_IO_STATUS_OK)
            {
                SKR_LOG_DEBUG("dispatch close request: %s", rq->path.c_str());
                skr_vfs_fclose(rq->file);
            }
            else
            {
                // SKR_LOG_DEBUG("dispatch close request: %s, skip", rq->path.c_str());
            }
        }
    }
}

void RAMService2Impl::Runner::uncompress() SKR_NOEXCEPT
{
    // do nothing now
}

void RAMService2Impl::Runner::finish() SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
    {
        auto& arr = requests[i];
        for (auto& rq : arr)
        {
            if (rq->getStatus() == SKR_ASYNC_IO_STATUS_OK)
            {
                bool need_finish = false;
                for (auto f : rq->finish_callbacks)
                {
                    if (f)
                        need_finish = true;
                }
                if (need_finish)
                {
                    finish_queues[i].enqueue(rq);
                }
                else
                {
                    skr_atomicu32_store_relaxed(&rq->done, 1);
                }
            }
        }
    }
}

} // namespace io
} // namespace skr