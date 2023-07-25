#include "../../pch.hpp"
#include "SkrRT/async/wait_timeout.hpp"
#include "../common/io_request.hpp"
#include "../dstorage/dstorage_resolvers.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "vram_service.hpp"
#include "vram_resolvers.hpp"
#include "vram_readers.hpp"

namespace skr {
namespace io {

IVRAMIOResource::~IVRAMIOResource() SKR_NOEXCEPT
{

}

IVRAMIOBuffer::~IVRAMIOBuffer() SKR_NOEXCEPT
{

}

IVRAMIOTexture::~IVRAMIOTexture() SKR_NOEXCEPT
{

}

IOResultId VRAMIOBatch::add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT
{
    /*
    auto srv = static_cast<VRAMService*>(service);
    auto buffer = srv->vram_buffer_pool->allocate();
    auto rq = skr::static_pointer_cast<VRAMRequestMixin>(request);
    rq->future = future;
    rq->destination = buffer;
    rq->owner_batch = this;
    SKR_ASSERT(!rq->blocks.empty());
    addRequest(request);
    return buffer;
    */
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

} // namespace io
} // namespace skr