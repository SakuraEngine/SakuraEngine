#pragma once
#include "SkrRT/platform/debug.h"
#include "SkrRT/platform/vfs.h"
#include "SkrRT/io/vram_io.hpp"
#include "../common/io_request.hpp"
#include "components.hpp"

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
    FileSrcComponent, IOStatusComponent, 
    VRAMIOStagingComponent, VRAMIOResourceComponent>
{
    using Super = IORequestMixin<Interface, 
        // components...
        FileSrcComponent, IOStatusComponent, 
        VRAMIOStagingComponent, VRAMIOResourceComponent>;

    void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void set_dstorage_queue(CGPUDStorageQueueId queue) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void set_buffer(CGPUTextureId texture) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void set_buffer(CGPUDeviceId device, const CGPUBufferDescriptor* desc) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

#pragma region IOVRAMResourceComponent
    void set_texture(CGPUTextureId texture) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
    
    void set_texture(CGPUDeviceId device, const CGPUTextureDescriptor* desc) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void set_slices(uint32_t first_slice, uint32_t slice_count) SKR_NOEXCEPT
    {
        SKR_UNIMPLEMENTED_FUNCTION();
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