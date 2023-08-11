#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "cgpu/api.h"

SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service)
SKR_DECLARE_TYPE_ID_FWD(skr::io, IVRAMService, skr_io_vram_service)

typedef struct skr_vram_io_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    skr_io_ram_service_id ram_service SKR_IF_CPP(= nullptr);
    skr_job_queue_id callback_job_queue SKR_IF_CPP(= nullptr);
    CGPUDeviceId gpu_device SKR_IF_CPP(= nullptr);
    bool awake_at_request SKR_IF_CPP(= true);
    bool use_dstorage SKR_IF_CPP(= true);
} skr_vram_io_service_desc_t;

#ifdef __cplusplus
namespace skr {
namespace io {

struct IVRAMService;
using VRAMServiceDescriptor = skr_vram_io_service_desc_t;

struct SKR_RUNTIME_API IVRAMIOResource : public skr::SInterface
{
    virtual ~IVRAMIOResource() SKR_NOEXCEPT;
};

struct SKR_RUNTIME_API IVRAMIOBuffer : public IVRAMIOResource
{
    virtual ~IVRAMIOBuffer() SKR_NOEXCEPT;

    // TODO: REMOVE THIS
    virtual CGPUBufferId get_buffer() const SKR_NOEXCEPT = 0;
    // virtual CGPUBufferId acquire_buffer(CGPUQueueId owner, CGPUCommandBufferId cmd) const SKR_NOEXCEPT = 0;
};

struct SKR_RUNTIME_API IVRAMIOTexture : public IVRAMIOResource
{
    virtual ~IVRAMIOTexture() SKR_NOEXCEPT;

    virtual CGPUTextureId get_texture() const SKR_NOEXCEPT = 0;
    // virtual CGPUTextureId acquire_texture(CGPUQueueId owner, CGPUCommandBufferId cmd) const SKR_NOEXCEPT = 0;
};

struct SKR_RUNTIME_API IVRAMIORequest : public IIORequest
{
    virtual ~IVRAMIORequest() SKR_NOEXCEPT;

#pragma region Transfer
    virtual void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT = 0;
    virtual void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT = 0;
    virtual RAMIOBufferId pin_staging_buffer() SKR_NOEXCEPT = 0;

    virtual void set_enable_dstorage(bool enable) SKR_NOEXCEPT = 0;
    virtual void set_dstorage_compression(SkrDStorageCompression compression, uint64_t uncompressed_size) SKR_NOEXCEPT = 0;
#pragma endregion
};

struct SKR_RUNTIME_API IBlocksVRAMRequest : public IVRAMIORequest
{
    virtual ~IBlocksVRAMRequest() SKR_NOEXCEPT;

#pragma region BlocksComponent
    virtual skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT = 0;
    virtual void add_block(const skr_io_block_t& block) SKR_NOEXCEPT = 0;
    virtual void reset_blocks() SKR_NOEXCEPT = 0;
#pragma endregion

#pragma region CompressedBlocksComponent
    virtual skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT = 0;
    virtual void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT = 0;
    virtual void reset_compressed_blocks() SKR_NOEXCEPT = 0;
#pragma endregion

#pragma region IOVRAMResourceComponent
    virtual void set_buffer(CGPUBufferId buffer, uint64_t offset) SKR_NOEXCEPT = 0;
    virtual void set_buffer(CGPUDeviceId device, const CGPUBufferDescriptor* desc) SKR_NOEXCEPT = 0;
#pragma endregion
};

struct SKR_RUNTIME_API ISlicesVRAMRequest : public IVRAMIORequest
{
    virtual ~ISlicesVRAMRequest() SKR_NOEXCEPT;
#pragma region IOVRAMResourceComponent
    virtual void set_texture(CGPUTextureId texture) SKR_NOEXCEPT = 0;
    virtual void set_texture(CGPUDeviceId device, const CGPUTextureDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual void set_slices(uint32_t first_slice, uint32_t slice_count) SKR_NOEXCEPT = 0;
#pragma endregion
};

struct SKR_RUNTIME_API ITilesVRAMRequest : public IVRAMIORequest
{
    virtual ~ITilesVRAMRequest() SKR_NOEXCEPT;
};

using VRAMIOResourceId = SObjectPtr<IVRAMIOResource>;
using VRAMIOBufferId = SObjectPtr<IVRAMIOBuffer>;
using VRAMIOTextureId = SObjectPtr<IVRAMIOTexture>;
using SlicesIORequestId = SObjectPtr<ISlicesVRAMRequest>;
using TilesIORequestId = SObjectPtr<ITilesVRAMRequest>;
using BlocksVRAMRequestId = SObjectPtr<IBlocksVRAMRequest>;

struct SKR_RUNTIME_API IVRAMService : public IIOService
{
    [[nodiscard]] static IVRAMService* create(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT;
    static void destroy(IVRAMService* service) SKR_NOEXCEPT;

    // open a texture request for filling
    [[nodiscard]] virtual SlicesIORequestId open_texture_request() SKR_NOEXCEPT = 0;

    // open a buffer request for filling
    [[nodiscard]] virtual BlocksVRAMRequestId open_buffer_request() SKR_NOEXCEPT = 0;

    // open a tile request for filling
    // [[nodiscard]] virtual TilesIORequestId open_tile_request() SKR_NOEXCEPT = 0;

    // start a request batch
    [[nodiscard]] virtual IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT = 0;

    // submit a buffer request
    [[nodiscard]] virtual VRAMIOBufferId request(BlocksVRAMRequestId request, IOFuture* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT = 0;
    
    // submit a texture request
    [[nodiscard]] virtual VRAMIOTextureId request(SlicesIORequestId request, IOFuture* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT = 0;
    
    // submit a batch
    virtual void request(IOBatchId request) SKR_NOEXCEPT = 0;

    // get underlying ram service
    virtual IRAMService* get_ram_service() SKR_NOEXCEPT = 0;

    // get avability of dstorage
    [[nodiscard]] virtual bool get_dstoage_available() const SKR_NOEXCEPT = 0;

    virtual ~IVRAMService() SKR_NOEXCEPT = default;
    IVRAMService() SKR_NOEXCEPT = default;
};

} // namespace io
} // namespace skr
#endif