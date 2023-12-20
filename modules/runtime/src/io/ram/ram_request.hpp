#pragma once
#include "../common/io_request.hpp"
#include "ram_buffer.hpp" // IWYU pragma: keep

#include <string.h> // ::strlen

namespace skr {
namespace io {

template <>
struct CID<struct RAMIOStatusComponent> 
{
    static constexpr skr_guid_t Get() { return CID<IOStatusComponent>::Get(); } 
};
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
    ~RAMRequestMixin() SKR_NOEXCEPT;

    RAMIOBufferId destination = nullptr;
protected:
    RAMRequestMixin(ISmartPoolPtr<IBlocksRAMRequest> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT;
    const uint64_t sequence = UINT64_MAX;
};

} // namespace io
} // namespace skr