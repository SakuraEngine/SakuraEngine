#include "../../pch.hpp"
#include "vram_request.hpp"

namespace skr::io {

VRAMRequestMixin::VRAMRequestMixin(ISmartPool<IIORequest>* pool, const uint64_t sequence) SKR_NOEXCEPT
    : IORequestCRTP(pool), sequence(sequence) 
{

}

ISlicesIORequest::~ISlicesIORequest() SKR_NOEXCEPT
{

}

ITilesIORequest::~ITilesIORequest() SKR_NOEXCEPT
{

}

} // namespace skr::io