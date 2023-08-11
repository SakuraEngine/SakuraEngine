
#include "ram_service.hpp"
#include "ram_batch.hpp"
#include "ram_buffer.hpp"

namespace skr {
namespace io {

const char* kIOBufferMemoryName = "io::buffer";

IRAMIOBuffer::~IRAMIOBuffer() SKR_NOEXCEPT
{

}

RAMIOBuffer::~RAMIOBuffer() SKR_NOEXCEPT
{
    free_buffer();
}

void RAMIOBuffer::allocate_buffer(uint64_t n) SKR_NOEXCEPT
{
    if (n)
    {
        bytes = (uint8_t*)sakura_mallocN(n, kIOBufferMemoryName);
    }
    size = n;
}

void RAMIOBuffer::free_buffer() SKR_NOEXCEPT
{
    if (bytes)
    {
        sakura_freeN(bytes, kIOBufferMemoryName);
        bytes = nullptr;
    }
    size = 0;
}

void RAMIOBatch::add_request(IORequestId request, RAMIOBufferId buffer, skr_io_future_t* future) SKR_NOEXCEPT
{
    auto rq = skr::static_pointer_cast<RAMRequestMixin>(request);
    if (auto pStatus = io_component<IOStatusComponent>(request.get()))
    {
        pStatus->owner_batch = this;
        pStatus->future = future;
    }
    rq->destination = buffer;
    if (auto pComp = io_component<BlocksComponent>(rq.get()))
    {
        SKR_ASSERT(!pComp->blocks.empty());
    }
    addRequest(request);
}

IOResultId RAMIOBatch::add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT
{
    auto srv = static_cast<RAMService*>(service);
    auto buffer = srv->ram_buffer_pool->allocate();
    add_request(request, buffer, future);
    return buffer;
}

} // namespace io
} // namespace skr