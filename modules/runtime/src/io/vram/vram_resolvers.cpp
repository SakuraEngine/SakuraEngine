#include "../../pch.hpp" // IWYU pragma: keep
#include "vram_resolvers.hpp"

namespace skr {
namespace io {

void AllocateVRAMResourceResolver::resolve(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
{
    ZoneScopedNC("VRAMResource::Allocate", tracy::Color::BlueViolet);
    SKR_UNIMPLEMENTED_FUNCTION();
    /*
    auto rq = skr::static_pointer_cast<VRAMIORequest>(request);
    auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
    // deal with 0 block size
    for (auto& block : rq->blocks)
    {
        if (block.size == 0)
        {
            block.size = rq->get_fsize() - block.offset;
        }
        if (buf->size == 0)
        {
            buf->size += block.size;
        }
    }
    // allocate
    if (buf->bytes == nullptr)
    {
        if (buf->size == 0)
        {
            SKR_ASSERT(0 && "invalid destination size");
        }
        buf->allocate_buffer(buf->size);
    }
    */
}

} // namespace io
} // namespace skr