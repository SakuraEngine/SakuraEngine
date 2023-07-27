#pragma once
#include "SkrRT/io/io.h"
#include "SkrRT/platform/debug.h"
#include "SkrRT/platform/vfs.h"
#include "SkrRT/io/vram_io.hpp"
#include "../common/io_request.hpp"
#include "cgpu/api.h"
#include "components.hpp"
#include "io/vram/components.hpp"

#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>
#include <string.h> // ::strlen

namespace skr {
namespace io {

struct VRAMIOStatusComponent final : public IOStatusComponent
{
    VRAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT;
    void setStatus(ESkrIOStage status) SKR_NOEXCEPT override;
};

template <typename Interface, typename... Components>
struct VRAMRequestMixin : public IORequestMixin<Interface, Components...>
{
    using Super = IORequestMixin<Interface, Components...>;

    void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMUploadComponent>()->set_transfer_queue(queue); 
    }

    void set_dstorage_queue(CGPUDStorageQueueId queue) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMDStorageComponent>()->set_dstorage_queue(queue); 
    }

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        Super::template safe_comp<MemorySrcComponent>()->set_memory_src(memory, bytes); 
    }

#pragma region VRAMBufferComponent
    void set_buffer(CGPUBufferId buffer) SKR_NOEXCEPT
    {        
        Super::template safe_comp<VRAMBufferComponent>()->set_buffer(buffer); 
    }

    void set_buffer(CGPUDeviceId device, const CGPUBufferDescriptor* desc) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMBufferComponent>()->set_buffer(device, desc); 
    }
#pragma endregion

#pragma region VRAMTextureComponent
    void set_texture(CGPUTextureId texture) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMTextureComponent>()->set_texture(texture); 
    }
    
    void set_texture(CGPUDeviceId device, const CGPUTextureDescriptor* desc) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMTextureComponent>()->set_texture(device, desc); 
    }

    void set_slices(uint32_t first_slice, uint32_t slice_count) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMTextureComponent>()->set_slices(first_slice, slice_count); 
    }
#pragma endregion

protected:
    VRAMRequestMixin(ISmartPoolPtr<Interface> pool, const uint64_t sequence) SKR_NOEXCEPT
        : Super(pool), sequence(sequence) 
    {

    }
    const uint64_t sequence;
};

template <typename T>
struct VRAMRequest
{

};

template <>
struct VRAMRequest<ISlicesVRAMRequest> final : public VRAMRequestMixin<ISlicesVRAMRequest,
    IOStatusComponent, 
    PathSrcComponent, MemorySrcComponent, // Src
    VRAMUploadComponent, VRAMDStorageComponent, // Method
    VRAMTextureComponent // Dst
>
{
    friend struct SmartPool<VRAMRequest<ISlicesVRAMRequest>, ISlicesVRAMRequest>;
protected:
    VRAMRequest(ISmartPoolPtr<ISlicesVRAMRequest> pool, const uint64_t sequence) SKR_NOEXCEPT
        : VRAMRequestMixin(pool, sequence)
    {

    }
};

template <>
struct VRAMRequest<ITilesVRAMRequest> final : public VRAMRequestMixin<ITilesVRAMRequest,
    IOStatusComponent, 
    PathSrcComponent, MemorySrcComponent, // Src
    VRAMUploadComponent, VRAMDStorageComponent, // Method
    VRAMTextureComponent //Dst
>
{
    friend struct SmartPool<VRAMRequest<ITilesVRAMRequest>, ITilesVRAMRequest>;
protected:
    VRAMRequest(ISmartPoolPtr<ITilesVRAMRequest> pool, const uint64_t sequence) SKR_NOEXCEPT
        : VRAMRequestMixin(pool, sequence)
    {

    }
};

template <>
struct VRAMRequest<IBlocksVRAMRequest> final : public VRAMRequestMixin<IBlocksVRAMRequest,
    IOStatusComponent, 
    PathSrcComponent, MemorySrcComponent, // Src
    VRAMUploadComponent, VRAMDStorageComponent, // Method
    VRAMBufferComponent //Dst
>
{
    friend struct SmartPool<VRAMRequest<IBlocksVRAMRequest>, IBlocksVRAMRequest>;
protected:
    VRAMRequest(ISmartPoolPtr<IBlocksVRAMRequest> pool, const uint64_t sequence) SKR_NOEXCEPT
        : VRAMRequestMixin(pool, sequence)
    {

    }
};

inline void VRAMIOStatusComponent::setStatus(ESkrIOStage status) SKR_NOEXCEPT
{
    if (status == SKR_IO_STAGE_CANCELLED)
    {
        // if (auto dest = static_cast<RAMIOBuffer*>(rq->destination.get()))
            // dest->free_resource();
    }
    return IOStatusComponent::setStatus(status);
}

} // namespace io
} // namespace skr