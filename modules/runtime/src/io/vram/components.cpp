#include "../../pch.hpp"
#include "components.hpp"
#include "vram_request.hpp"

namespace skr {
namespace io {

VRAMIOStatusComponent::VRAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
    
}

VRAMIOStagingComponent::VRAMIOStagingComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t VRAMIOStagingComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<VRAMIOStagingComponent>::Get(); 
}

VRAMBufferComponent::VRAMBufferComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t VRAMBufferComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<VRAMBufferComponent>::Get(); 
}

TextureComponent::TextureComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t TextureComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<TextureComponent>::Get(); 
}


} // namespace io
} // namespace skr