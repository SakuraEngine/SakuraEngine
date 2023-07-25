#include "../../pch.hpp"
#include "components.hpp"

namespace skr {
namespace io {

VRAMIOStatusComponent::VRAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
    
}

VRAMIOStagingComponent::VRAMIOStagingComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
    
}

skr_guid_t VRAMIOStagingComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<VRAMIOStagingComponent>::Get(); 
}

VRAMIOResourceComponent::VRAMIOResourceComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
    
}

skr_guid_t VRAMIOResourceComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<VRAMIOResourceComponent>::Get(); 
}


} // namespace io
} // namespace skr