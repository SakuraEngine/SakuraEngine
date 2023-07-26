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

template <typename Interface>
struct VRAMRequestMixin final : public IORequestMixin<Interface, 
    // components...
    IOStatusComponent, 
    FileSrcComponent, // Src
    VRAMIOStagingComponent, // Transfer
    VRAMBufferComponent, VRAMTextureComponent //Dst
>
{
    using Super = IORequestMixin<Interface, 
        // components...
        IOStatusComponent, 
        FileSrcComponent, // Src
        VRAMIOStagingComponent, // Transfer
        VRAMBufferComponent, VRAMTextureComponent //Dst
    >;

    void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMIOStagingComponent>()->set_transfer_queue(queue); 
    }

    void set_dstorage_queue(CGPUDStorageQueueId queue) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMIOStagingComponent>()->set_dstorage_queue(queue); 
    }

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMIOStagingComponent>()->set_memory_src(memory, bytes); 
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

    friend struct SmartPool<VRAMRequestMixin<Interface>, Interface>;
protected:
    VRAMRequestMixin(ISmartPoolPtr<Interface> pool, const uint64_t sequence) SKR_NOEXCEPT
        : Super(pool), sequence(sequence) 
    {

    }
    const uint64_t sequence;
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