#include "../../pch.hpp"
#include "status_component.hpp"
#include "blocks_component.hpp"
#include "src_components.hpp"

namespace skr {
namespace io {

IORequestComponent::IORequestComponent(IIORequest* const request) SKR_NOEXCEPT 
    : request(request) 
{

}

IORequestComponent::~IORequestComponent() SKR_NOEXCEPT
{

}

IOStatusComponent::IOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

IOStatusComponent::~IOStatusComponent() SKR_NOEXCEPT
{

}

skr_guid_t IOStatusComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<IOStatusComponent>::Get(); 
}

PathSrcComponent::PathSrcComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t PathSrcComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<PathSrcComponent>::Get(); 
}

FileComponent::FileComponent(IIORequest* const request) SKR_NOEXCEPT
    : IORequestComponent(request)
{

}

skr_guid_t FileComponent::get_tid() const SKR_NOEXCEPT
{
    return IORequestComponentTID<FileComponent>::Get(); 
}

uint64_t FileComponent::get_fsize() const SKR_NOEXCEPT
{
    if (file)
    {
        SKR_ASSERT(!dfile);
        return skr_vfs_fsize(file);
    }
    else
    {
        SKR_ASSERT(dfile);
        SKR_ASSERT(!file);
        auto instance = skr_get_dstorage_instnace();
        SkrDStorageFileInfo info;
        skr_dstorage_query_file_info(instance, dfile, &info);
        return info.file_size;
    }
    return 0;
}

MemorySrcComponent::MemorySrcComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

skr_guid_t MemorySrcComponent::get_tid() const SKR_NOEXCEPT 
{ 
    return IORequestComponentTID<MemorySrcComponent>::Get(); 
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