#include "SkrRT/io/ram_io.hpp"
#include "ram_request.hpp"

namespace skr::io {

IBlocksRAMRequest::~IBlocksRAMRequest() SKR_NOEXCEPT
{

}

RAMIOStatusComponent::RAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
    : IOStatusComponent(request) 
{
        
}

RAMRequestMixin::RAMRequestMixin(ISmartPoolPtr<IBlocksRAMRequest> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
    : IORequestMixin(pool, service), sequence(sequence) 
{

}

RAMRequestMixin::~RAMRequestMixin() SKR_NOEXCEPT
{
    destination.reset();
}

void RAMIOStatusComponent::setStatus(ESkrIOStage status) SKR_NOEXCEPT
{
    auto rq = static_cast<RAMRequestMixin*>(request);
    if (status == SKR_IO_STAGE_CANCELLED)
    {
        if (auto dest = static_cast<RAMIOBuffer*>(rq->destination.get()))
        {
            dest->free_buffer();
        }
    }
    return IOStatusComponent::setStatus(status);
}

} // namespace skr::io