#pragma once
#include "../common/io_request.hpp"
#include "ram_buffer.hpp"

#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>
#include <string.h> // ::strlen

namespace skr {
namespace io {

struct RAMIOStatusComponent final : public IOStatusComponent
{
    RAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
        : IOStatusComponent(request) 
    {
        
    }
    void setStatus(ESkrIOStage status) SKR_NOEXCEPT override;
};

struct RAMRequestMixin final : public IORequestCRTP<IBlocksRAMRequest, 
    IOFileComponent, RAMIOStatusComponent, IOBlocksComponent>
{
    friend struct SmartPool<RAMRequestMixin, IBlocksRAMRequest>;

    RAMIOBufferId destination = nullptr;
protected:
    RAMRequestMixin(ISmartPool<IBlocksRAMRequest>* pool, const uint64_t sequence) 
        : IORequestCRTP(pool), sequence(sequence) 
    {

    }
    const uint64_t sequence;
};

inline void RAMIOStatusComponent::setStatus(ESkrIOStage status) SKR_NOEXCEPT
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

} // namespace io
} // namespace skr