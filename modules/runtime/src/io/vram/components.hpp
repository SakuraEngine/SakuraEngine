#pragma once
#include "SkrRT/misc/types.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/io/vram_io.hpp"
#include "io/ram/ram_service.hpp"
#include "io/components/component.hpp"

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
public:
    VRAMUploadComponent(IIORequest* const request) SKR_NOEXCEPT;

    void set_transfer_queue(CGPUQueueId queue) SKR_NOEXCEPT
    {
        transfer_queue = queue;
    }

    CGPUQueueId get_transfer_queue() const SKR_NOEXCEPT
    {
        return transfer_queue;
    }

    RAMIOBufferId pin_staging_buffer(RAMService* ram_service) SKR_NOEXCEPT
    {
        ram_buffer = ram_service->ram_buffer_pool->allocate();
        return ram_buffer;
    }

protected:
    friend struct CommonVRAMReader;
    CGPUQueueId transfer_queue = nullptr;
    RAMIOBufferId ram_buffer = nullptr;
    IOFuture ram_future = {};
    uint8_t* src_data = nullptr;
    uint64_t src_size = 0;
};

template <>
struct CID<struct VRAMDStorageComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMDStorageComponent final : public IORequestComponent
{
    VRAMDStorageComponent(IIORequest* const request) SKR_NOEXCEPT;

    void set_force_enable_dstorage(bool enable) SKR_NOEXCEPT
    {
        use_force = true;
        force_enabled = enable;
    }

    void set_enable_dstorage(bool enable) SKR_NOEXCEPT
    {
        enabled = enable;
    }

    void set_dstorage_compression(SkrDStorageCompression compression, uint64_t uncompressed_size) SKR_NOEXCEPT
    {
        this->compression = compression;
        this->uncompressed_size = uncompressed_size;
    }

    void get_dstorage_compression(SkrDStorageCompression& c, uint64_t& sz) const SKR_NOEXCEPT
    {
        if (this->compression != SKR_DSTORAGE_COMPRESSION_NONE)
        {
            c = this->compression;
            sz = this->uncompressed_size;
        }
    }

    bool should_use_dstorage() SKR_NOEXCEPT
    {
        bool _enabled = enabled;
        if (use_force)
        {
            _enabled = force_enabled;
        }
        auto vram_service = static_cast<IVRAMService*>(this->request->get_service());
        return _enabled && vram_service->get_dstoage_available();
    }

private:
    friend struct DStorageVRAMReader;
    friend struct AllocateVRAMResourceResolver;
    SkrDStorageFileHandle dfile = nullptr;
    SkrDStorageCompression compression = SKR_DSTORAGE_COMPRESSION_NONE;
    uint64_t uncompressed_size = 0;
    bool use_force = false;
    bool force_enabled = true;
    bool enabled = true;
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
        None,
        Imported,
        ServiceCreated
    };
    Type type = Type::None;
    uint64_t offset = 0;
    CGPUBufferId buffer = nullptr;
    CGPUDeviceId device = nullptr;
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
        None,
        Imported,
        ServiceCreated
    };
    Type type = Type::None;
    CGPUTextureId texture = nullptr;
    CGPUDeviceId device = nullptr;
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