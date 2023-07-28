#include "components.hpp"
#include "vram_request.hpp"

namespace skr {
namespace io {

VRAMIOStatusComponent::VRAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
    
}

void VRAMIOStatusComponent::setStatus(ESkrIOStage status) SKR_NOEXCEPT
{
    if (status == SKR_IO_STAGE_CANCELLED)
    {
        // if (auto dest = static_cast<RAMIOBuffer*>(rq->destination.get()))
            // dest->free_resource();
    }
    return IOStatusComponent::setStatus(status);
}

VRAMUploadComponent::VRAMUploadComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

VRAMDStorageComponent::VRAMDStorageComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

VRAMBufferComponent::VRAMBufferComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

VRAMBufferComponent::~VRAMBufferComponent() SKR_NOEXCEPT
{

}

void VRAMBufferComponent::set_buffer(CGPUBufferId buffer, uint64_t offset) SKR_NOEXCEPT
{
    this->offset = offset;
    this->buffer = buffer;
    this->device = buffer->device;
    this->type = Type::Imported;
}

void VRAMBufferComponent::set_buffer(CGPUDeviceId device, const CGPUBufferDescriptor* desc) SKR_NOEXCEPT
{
    this->device = device;
    this->desc = *desc;
    this->type = Type::ServiceCreated;
}

VRAMTextureComponent::VRAMTextureComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

VRAMTextureComponent::~VRAMTextureComponent() SKR_NOEXCEPT
{
    
}

void VRAMTextureComponent::set_texture(CGPUTextureId texture) SKR_NOEXCEPT
{
    this->texture = texture;
    this->device = texture->device;
    this->type = Type::Imported;
}

void VRAMTextureComponent::set_texture(CGPUDeviceId device, const CGPUTextureDescriptor* desc) SKR_NOEXCEPT
{
    this->device = device;
    this->desc = *desc;
    this->type = Type::ServiceCreated;
}

void VRAMTextureComponent::set_slices(uint32_t first_slice, uint32_t slice_count) SKR_NOEXCEPT
{
    this->first_slice = first_slice;
    this->slice_count = slice_count;
}

} // namespace io
} // namespace skr