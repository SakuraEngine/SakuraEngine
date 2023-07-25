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

FileSrcComponent::FileSrcComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t FileSrcComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<FileSrcComponent>::Get(); 
}

BlocksComponent::BlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t BlocksComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<BlocksComponent>::Get(); 
}

CompressedBlocksComponent::CompressedBlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t CompressedBlocksComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<CompressedBlocksComponent>::Get(); 
}

} // namespace io
} // namespace skr