#include "../../pch.hpp"
#include "../common/io_request.hpp"
#include "io/vram/components.hpp"
#include "vram_resolvers.hpp"
#include "components.hpp"

namespace skr {
namespace io {

void AllocateVRAMResourceResolver::resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT
{
    ZoneScopedNC("VRAMResource::Allocate", tracy::Color::BlueViolet);
    auto buffer = io_component<VRAMBufferComponent>(request.get());
    if (buffer && (buffer->type == VRAMBufferComponent::Type::ServiceCreated))
    {
        buffer->buffer = cgpu_create_buffer(buffer->device, &buffer->desc);
        return;
    }
    
    auto texture = io_component<VRAMTextureComponent>(request.get());
    if (texture && (texture->type == VRAMTextureComponent::Type::ServiceCreated))
    {
        texture->texture = cgpu_create_texture(texture->device, &texture->desc);
        return;
    }
}

} // namespace io
} // namespace skr