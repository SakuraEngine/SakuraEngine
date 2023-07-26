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

IOResultId VRAMIOBatch::add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT
{
    auto srv = static_cast<VRAMService*>(service);
    if (auto pBufferComp = io_component<VRAMBufferComponent>(request.get()))
    {
        pBufferComp->artifact = srv->vram_buffer_pool->allocate();
        addRequest(request);
        return pBufferComp->artifact;
    }
    if (auto pTextureComp = io_component<VRAMTextureComponent>(request.get()))
    {
        pTextureComp->artifact = srv->vram_texture_pool->allocate();
        addRequest(request);
        return pTextureComp->artifact;
    }
    SKR_UNREACHABLE_CODE();
    return nullptr;
}

} // namespace io
} // namespace skr