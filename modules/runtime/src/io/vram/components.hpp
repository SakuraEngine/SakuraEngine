#pragma once
#include "../components/status_component.hpp"

struct CGPUBuffer;
struct CGPUTexture;

namespace skr {
namespace io {

template <>
struct IORequestComponentTID<struct VRAMIOStagingComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMIOStagingComponent final : public IOStatusComponent
{
    VRAMIOStagingComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;

    uint8_t* buffer = nullptr;
    uint64_t size = 0;
};

template <>
struct IORequestComponentTID<struct VRAMIOResourceComponent> 
{
    static constexpr skr_guid_t Get();
};
struct VRAMIOResourceComponent final : public IOStatusComponent
{
    VRAMIOResourceComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual skr_guid_t get_tid() const SKR_NOEXCEPT override;

    enum class Type : uint32_t 
    {
        Buffer,
        Texture,
        TiledTexture
    };
    Type type;
    CGPUBuffer* buffer;
    CGPUTexture* texture;
};

constexpr skr_guid_t IORequestComponentTID<struct VRAMIOResourceComponent>::Get()
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