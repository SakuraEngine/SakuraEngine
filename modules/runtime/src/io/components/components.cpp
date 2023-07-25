#include "../../pch.hpp"
#include "status_component.hpp"
#include "blocks_component.hpp"
#include "file_component.hpp"

namespace skr {
namespace io {

IOStatusComponent::IOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t IOStatusComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<IOStatusComponent>::Get(); 
}

IOFileComponent::IOFileComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t IOFileComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<IOFileComponent>::Get(); 
}

IOBlocksComponent::IOBlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t IOBlocksComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<IOBlocksComponent>::Get(); 
}

IOCompressedBlocksComponent::IOCompressedBlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t IOCompressedBlocksComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<IOCompressedBlocksComponent>::Get(); 
}

} // namespace io
} // namespace skr