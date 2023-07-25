#pragma once
#include "SkrRT/platform/vfs.h"
#include "SkrRT/io/vram_io.hpp"
#include "../common/io_request.hpp"
#include "components.hpp"

#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>
#include <string.h> // ::strlen

namespace skr {
namespace io {

struct VRAMRequestMixin final : public IORequestCRTP<
    IIORequest, IOFileComponent, IOStatusComponent, VRAMIOStagingComponent>
{
    friend struct SmartPool<VRAMRequestMixin, IIORequest>;

    eastl::fixed_vector<skr_io_block_t, 1> blocks;
    
protected:
    VRAMRequestMixin(ISmartPool<IIORequest>* pool, const uint64_t sequence) 
        : IORequestCRTP(pool), sequence(sequence) 
    {

    }

    const uint64_t sequence;
};

inline void VRAMIOStatusComponent::setStatus(ESkrIOStage status) SKR_NOEXCEPT
{
    [[maybe_unused]] auto rq = static_cast<VRAMRequestMixin*>(request);
    if (status == SKR_IO_STAGE_CANCELLED)
    {
        // if (auto dest = static_cast<RAMIOBuffer*>(rq->destination.get()))
            // dest->free_resource();
    }
    return IOStatusComponent::setStatus(status);
}

} // namespace io
} // namespace skr