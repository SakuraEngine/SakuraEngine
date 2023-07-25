#include "../../pch.hpp"
#include "vram_request.hpp"

namespace skr::io {

VRAMRequestMixin::VRAMRequestMixin(ISmartPool<IIORequest>* pool, const uint64_t sequence) SKR_NOEXCEPT
    : IORequestCRTP(pool), sequence(sequence) 
{

}

IVRAMIORequest::~IVRAMIORequest() SKR_NOEXCEPT
{

}

IBlocksVRAMRequest::~IBlocksVRAMRequest() SKR_NOEXCEPT
{

}

ISlicesVRAMRequest::~ISlicesVRAMRequest() SKR_NOEXCEPT
{

}

ITilesVRAMRequest::~ITilesVRAMRequest() SKR_NOEXCEPT
{

}

} // namespace skr::io