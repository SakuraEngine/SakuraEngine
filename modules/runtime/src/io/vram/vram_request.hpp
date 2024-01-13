#pragma once
#include "SkrGraphics/api.h"
#include "SkrRT/io/io.h"
#include "SkrRT/io/vram_io.hpp"
#include "./../vram/components.hpp"
#include "./../common/io_request.hpp"

#include <string.h> // ::strlen

namespace skr {
namespace io {

template <>
struct CID<struct VRAMIOStatusComponent> 
{
    static constexpr skr_guid_t Get() { return CID<IOStatusComponent>::Get(); } 
};
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

    void set_enable_dstorage(bool enable) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMDStorageComponent>()->set_enable_dstorage(enable); 
    }
    
    void set_dstorage_compression(SkrDStorageCompression compression, uint64_t uncompressed_size) SKR_NOEXCEPT
    {
        Super::template safe_comp<VRAMDStorageComponent>()->set_dstorage_compression(compression, uncompressed_size); 
    }

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        Super::template safe_comp<MemorySrcComponent>()->set_memory_src(memory, bytes); 
    }

    RAMIOBufferId pin_staging_buffer() SKR_NOEXCEPT
    {
        auto vram_service = static_cast<IVRAMService*>(this->service);
        auto ram_service = static_cast<RAMService*>(vram_service->get_ram_service());
        Super::template safe_comp<VRAMDStorageComponent>()->set_force_enable_dstorage(false); 
        return Super::template safe_comp<VRAMUploadComponent>()->pin_staging_buffer(ram_service); 
    }

#pragma region VRAMBufferComponent
    void set_buffer(CGPUBufferId buffer, uint64_t offset) SKR_NOEXCEPT
    {        
        Super::template safe_comp<VRAMBufferComponent>()->set_buffer(buffer, offset); 
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
    VRAMRequestMixin(ISmartPoolPtr<Interface> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
        : Super(pool, service), sequence(sequence) 
    {

    }
    const uint64_t sequence;
};

template <typename T>
struct VRAMRequest {};

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
    VRAMRequest(ISmartPoolPtr<ISlicesVRAMRequest> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
        : VRAMRequestMixin(pool, service, sequence)
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
    VRAMRequest(ISmartPoolPtr<ITilesVRAMRequest> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
        : VRAMRequestMixin(pool, service, sequence)
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
    VRAMRequest(ISmartPoolPtr<IBlocksVRAMRequest> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
        : VRAMRequestMixin(pool, service, sequence)
    {

    }
};

} // namespace io
} // namespace skr