#pragma once
#include "../common/io_runnner.hpp"
#include "../common/processors.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "vram_batch.hpp"
#include "vram_request.hpp"
#include "vram_resources.hpp"

namespace skr {
namespace io {

struct VRAMService final : public IVRAMService
{
    VRAMService(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT;
    
    [[nodiscard]] IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT;
    [[nodiscard]] virtual SlicesIORequestId open_texture_request() SKR_NOEXCEPT;
    [[nodiscard]] virtual BlocksVRAMRequestId open_buffer_request() SKR_NOEXCEPT;
    VRAMIOBufferId request(BlocksVRAMRequestId request, IOFuture* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT;
    VRAMIOTextureId request(SlicesIORequestId request, IOFuture* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT;
    void request(IOBatchId request) SKR_NOEXCEPT;
    IRAMService* get_ram_service() SKR_NOEXCEPT
    {
        return ram_service;
    }
    bool get_dstoage_available() const SKR_NOEXCEPT { return runner.ds_reader.get(); }

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
        Runner(VRAMService* service, skr::JobQueue* job_queue) SKR_NOEXCEPT;
        void enqueueBatch(const IOBatchId& batch) SKR_NOEXCEPT;
        void set_resolvers() SKR_NOEXCEPT;

        IOBatchBufferId batch_buffer = nullptr;
        IOReaderId<IIOBatchProcessor> ds_reader = nullptr;
        IOReaderId<IIOBatchProcessor> common_reader = nullptr;
        VRAMService* service = nullptr;
    };

    const skr::String name;    
    const bool awake_at_request = false;
    Runner runner;
    
    template <typename Interface>
    using VRAMRequestPool = SmartPoolPtr<VRAMRequest<Interface>, Interface>;
    VRAMRequestPool<ISlicesVRAMRequest> slices_pool = nullptr;
    VRAMRequestPool<ITilesVRAMRequest> tiles_pool = nullptr;
    VRAMRequestPool<IBlocksVRAMRequest> blocks_pool = nullptr;

    SmartPoolPtr<VRAMIOBatch, IIOBatch> vram_batch_pool = nullptr;

    SmartPoolPtr<VRAMBuffer, IVRAMIOBuffer> vram_buffer_pool = nullptr;
    SmartPoolPtr<VRAMTexture, IVRAMIOTexture> vram_texture_pool = nullptr;
private:
    IRAMService* ram_service = nullptr;
    static uint32_t global_idx;
    SAtomicU64 request_sequence = 0;
    SAtomicU64 batch_sequence = 0;
};

} // namespace io
} // namespace skr