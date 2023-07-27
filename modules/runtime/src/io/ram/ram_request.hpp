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
    RAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT;
    void setStatus(ESkrIOStage status) SKR_NOEXCEPT override;
};

struct RAMRequestMixin final : public IORequestMixin<IBlocksRAMRequest, 
    // components...
    RAMIOStatusComponent, 
    PathSrcComponent, FileComponent,
    BlocksComponent>
{
    friend struct SmartPool<RAMRequestMixin, IBlocksRAMRequest>;

    RAMIOBufferId destination = nullptr;
protected:
    RAMRequestMixin(ISmartPoolPtr<IBlocksRAMRequest> pool, const uint64_t sequence) SKR_NOEXCEPT;
    const uint64_t sequence = UINT64_MAX;
};

} // namespace io
} // namespace skr