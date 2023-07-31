#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "../components/status_component.hpp"
#include "SkrRT/platform/debug.h"

struct CGPUBuffer;
struct CGPUTexture;

namespace skr {
namespace io {

template <>
struct CID<struct VRAMUploadComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMUploadComponent final : public IORequestComponent
{
    VRAMUploadComponent(IIORequest* const request) SKR_NOEXCEPT;

    void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT
    {
        transfer_queue = queue;
    }

    CGPUQueueId transfer_queue = nullptr;
    RAMIOBufferId buffer = nullptr;
    IOFuture ram_future;

    uint8_t* data = nullptr;
    uint64_t size = 0;
};

template <>
struct CID<struct VRAMDStorageComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMDStorageComponent final : public IORequestComponent
{
    VRAMDStorageComponent(IIORequest* const request) SKR_NOEXCEPT;

    void set_dstorage_queue(CGPUDStorageQueueId queue) SKR_NOEXCEPT
    {
        dstorage_queue = queue;
    }

    CGPUDStorageQueueId dstorage_queue = nullptr;
    SkrDStorageFileHandle dfile = nullptr;
};

template <>
struct CID<struct VRAMBufferComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMBufferComponent final : public IORequestComponent
{
    VRAMBufferComponent(IIORequest* const request) SKR_NOEXCEPT;
    ~VRAMBufferComponent() SKR_NOEXCEPT;

    void set_buffer(CGPUBufferId buffer, uint64_t offset) SKR_NOEXCEPT;
    void set_buffer(CGPUDeviceId device, const CGPUBufferDescriptor* desc) SKR_NOEXCEPT;

    SObjectPtr<IVRAMIOBuffer> artifact;

    enum class Type
    {
        Imported,
        ServiceCreated
    };
    Type type;
    uint64_t offset;
    CGPUBufferId buffer;
    CGPUDeviceId device;
    CGPUBufferDescriptor desc;
};

template <>
struct CID<struct VRAMTextureComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMTextureComponent final : public IORequestComponent
{
    VRAMTextureComponent(IIORequest* const request) SKR_NOEXCEPT;
    ~VRAMTextureComponent() SKR_NOEXCEPT;
    
    void set_texture(CGPUTextureId texture) SKR_NOEXCEPT;
    void set_texture(CGPUDeviceId device, const CGPUTextureDescriptor* desc) SKR_NOEXCEPT;
    void set_slices(uint32_t first_slice, uint32_t slice_count) SKR_NOEXCEPT;

    SObjectPtr<IVRAMIOTexture> artifact;

    enum class Type
    {
        Imported,
        ServiceCreated
    };
    Type type;
    CGPUTextureId texture;
    CGPUDeviceId device;
    CGPUTextureDescriptor desc;

    uint32_t first_slice = 0;
    uint32_t slice_count = 0;
};

constexpr skr_guid_t CID<struct VRAMUploadComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"15a2c517-fc77-4938-90df-2842a75b82a9"_guid;
} 

constexpr skr_guid_t CID<struct VRAMDStorageComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"5063c4b2-a197-496d-b058-b7a71656c8c1"_guid;
} 

constexpr skr_guid_t CID<struct VRAMBufferComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"78e4e3f0-5983-43b0-8567-f1a2653f8ea0"_guid;
} 

constexpr skr_guid_t CID<struct VRAMTextureComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"2d517d3b-3c08-4e6d-9b2b-189b0f591171"_guid;
} 

} // namespace io
} // namespace skr