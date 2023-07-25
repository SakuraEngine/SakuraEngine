#pragma once
#include "SkrRT/io/io.h"

SKR_DECLARE_TYPE_ID_FWD(skr::io, IVRAMService, skr_io_vram_service)

typedef struct skr_vram_io_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    skr_job_queue_id io_job_queue SKR_IF_CPP(= nullptr);
    skr_job_queue_id callback_job_queue SKR_IF_CPP(= nullptr);
    bool awake_at_request SKR_IF_CPP(= true);
    bool use_dstorage SKR_IF_CPP(= true);
} skr_vram_io_service_desc_t;

namespace skr {
namespace io {

struct IVRAMService;
using VRAMServiceDescriptor = skr_vram_io_service_desc_t;

struct RUNTIME_API IVRAMIOResource : public skr::SInterface
{
    virtual ~IVRAMIOResource() SKR_NOEXCEPT;
};

struct RUNTIME_API IVRAMIOBuffer : public IVRAMIOResource
{
    virtual ~IVRAMIOBuffer() SKR_NOEXCEPT;
};

struct RUNTIME_API IVRAMIOTexture : public IVRAMIOResource
{
    virtual ~IVRAMIOTexture() SKR_NOEXCEPT;
};

struct RUNTIME_API ISlicesIORequest : public IIORequest
{
    virtual ~ISlicesIORequest() SKR_NOEXCEPT;
};

struct RUNTIME_API ITilesIORequest : public IIORequest
{
    virtual ~ITilesIORequest() SKR_NOEXCEPT;
};

using VRAMIOResourceId = SObjectPtr<IVRAMIOResource>;
using VRAMIOBufferId = SObjectPtr<IVRAMIOBuffer>;
using VRAMIOTextureId = SObjectPtr<IVRAMIOTexture>;
using SlicesIORequestId = SObjectPtr<ISlicesIORequest>;
using TilesIORequestId = SObjectPtr<ITilesIORequest>;

struct RUNTIME_API IVRAMService : public IIOService
{
    [[nodiscard]] static IVRAMService* create(const VRAMServiceDescriptor* desc) SKR_NOEXCEPT;
    static void destroy(IVRAMService* service) SKR_NOEXCEPT;

    // open a texture request for filling
    // [[nodiscard]] virtual SlicesIORequestId open_texture_request() SKR_NOEXCEPT = 0;

    // open a buffer request for filling
    // [[nodiscard]] virtual BlocksIORequestId open_buffer_request() SKR_NOEXCEPT = 0;

    // open a tile request for filling
    // [[nodiscard]] virtual TilesIORequestId open_tile_request() SKR_NOEXCEPT = 0;

    // start a request batch
    // [[nodiscard]] virtual IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT = 0;

    // submit a request
    // virtual VRAMIOBufferId request(IORequestId request, IOFuture* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT = 0;
    
    // submit a batch
    virtual void request(IOBatchId request) SKR_NOEXCEPT = 0;

    virtual ~IVRAMService() SKR_NOEXCEPT = default;
    IVRAMService() SKR_NOEXCEPT = default;
};

} // namespace io
} // namespace skr