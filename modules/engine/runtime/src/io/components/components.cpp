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

PathSrcComponent::PathSrcComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

FileComponent::FileComponent(IIORequest* const request) SKR_NOEXCEPT
    : IORequestComponent(request)
{

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

BlocksComponent::BlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

CompressedBlocksComponent::CompressedBlocksComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IORequestComponent(request) 
{
    
}

} // namespace io
} // namespace skr