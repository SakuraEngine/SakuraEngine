#include "SkrRT/platform/dstorage.h"
#include "../common/io_request.hpp"
#include "../vram/components.hpp"
#include "../vram/vram_resolvers.hpp"
#include "components.hpp"

namespace skr {
namespace io {

void AllocateVRAMResourceResolver::resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT
{
    ZoneScopedNC("VRAMResource::Allocate", tracy::Color::BlueViolet);
    // try open dstorage files first
    if (auto pMem = io_component<MemorySrcComponent>(request.get()); !pMem->data)
    {
        if (auto pDS = io_component<VRAMDStorageComponent>(request.get()); pDS && pDS->dstorage_queue)
        {
            auto pPath = io_component<PathSrcComponent>(request.get());
            auto instance = skr_get_dstorage_instnace();
            pDS->dfile = skr_dstorage_open_file(instance, pPath->get_path());
            SKR_ASSERT(pDS->dfile);
        }
    }

    // allocate vram buffers
    auto buffer = io_component<VRAMBufferComponent>(request.get());
    if (buffer && (buffer->type == VRAMBufferComponent::Type::ServiceCreated))
    {
        buffer->buffer = cgpu_create_buffer(buffer->device, &buffer->desc);
        return;
    }
    
    // allocate vram textures
    auto texture = io_component<VRAMTextureComponent>(request.get());
    if (texture && (texture->type == VRAMTextureComponent::Type::ServiceCreated))
    {
        texture->texture = cgpu_create_texture(texture->device, &texture->desc);
        return;
    }
}

} // namespace io
} // namespace skr