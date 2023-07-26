#pragma once
#include "SkrRT/io/vram_io.hpp"
#include "../components/status_component.hpp"
#include "SkrRT/platform/debug.h"

struct CGPUBuffer;
struct CGPUTexture;

namespace skr {
namespace io {

template <>
struct IORequestComponentTID<struct VRAMIOStagingComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMIOStagingComponent final : public IORequestComponent
{
    VRAMIOStagingComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;

    void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT
    {
        transfer_queue = queue;
    }

    void set_dstorage_queue(CGPUDStorageQueueId queue) SKR_NOEXCEPT
    {
        dstorage_queue = queue;
    }

    void set_memory_src(uint8_t* memory, uint64_t bytes) SKR_NOEXCEPT
    {
        buffer = memory;
        size = bytes;
    }

    uint8_t* buffer = nullptr;
    uint64_t size = 0;
    CGPUQueueId transfer_queue = nullptr;
    CGPUDStorageQueueId dstorage_queue = nullptr;
};

template <>
struct IORequestComponentTID<struct VRAMBufferComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMBufferComponent final : public IORequestComponent
{
    VRAMBufferComponent(IIORequest* const request) SKR_NOEXCEPT;
    ~VRAMBufferComponent() SKR_NOEXCEPT;

    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;

    void set_buffer(CGPUBufferId buffer) SKR_NOEXCEPT
    {
        this->buffer = buffer;
        this->device = buffer->device;
        this->type = Type::Imported;
    }

    void set_buffer(CGPUDeviceId device, const CGPUBufferDescriptor* desc) SKR_NOEXCEPT
    {
        this->device = device;
        this->desc = *desc;
        this->type = Type::ServiceCreated;
    }

    SObjectPtr<IVRAMIOBuffer> artifact;

    enum class Type
    {
        Imported,
        ServiceCreated
    };
    Type type;
    CGPUBufferId buffer;
    CGPUDeviceId device;
    CGPUBufferDescriptor desc;
};

template <>
struct IORequestComponentTID<struct VRAMTextureComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMTextureComponent final : public IORequestComponent
{
    VRAMTextureComponent(IIORequest* const request) SKR_NOEXCEPT;
    ~VRAMTextureComponent() SKR_NOEXCEPT;
    
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;

    void set_texture(CGPUTextureId texture) SKR_NOEXCEPT
    {
        this->texture = texture;
        this->device = texture->device;
        this->type = Type::Imported;
    }
    
    void set_texture(CGPUDeviceId device, const CGPUTextureDescriptor* desc) SKR_NOEXCEPT
    {
        this->device = device;
        this->desc = *desc;
        this->type = Type::ServiceCreated;
    }

    void set_slices(uint32_t first_slice, uint32_t slice_count) SKR_NOEXCEPT
    {
        this->first_slice = first_slice;
        this->slice_count = slice_count;
    }

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

constexpr skr_guid_t IORequestComponentTID<struct VRAMBufferComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"78e4e3f0-5983-43b0-8567-f1a2653f8ea0"_guid;
} 

constexpr skr_guid_t IORequestComponentTID<struct VRAMTextureComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"2d517d3b-3c08-4e6d-9b2b-189b0f591171"_guid;
} 

constexpr skr_guid_t IORequestComponentTID<struct VRAMIOStagingComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"15a2c517-fc77-4938-90df-2842a75b82a9"_guid;
} 
} // namespace io
} // namespace skr