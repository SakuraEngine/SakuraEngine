#pragma once
#include "SkrRTTR/enum_tools.hpp"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/io/vram_io.hpp"
#include "SkrRuntime/resource/resource_factory.h"
#include "SkrRenderer/fwd_types.h"
#include "SkrRenderer/resources/texture_resource.generated.h" // IWYU pragma: export

namespace skr
{
// (GPU) texture resource
struct [[sattr(
    guid = "f8821efb-f027-4367-a244-9cc3efb3a3bf"
    serde = @enable
)]] TextureResource
{
    skr::EnumAsValue<ECGPUFormat> format;
    uint32_t mips_count;
    uint64_t data_size;
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    [[sattr(serde = @disable)]]
    CGPUTextureId texture = nullptr;
    [[sattr(serde = @disable)]]
    CGPUTextureViewId texture_view = nullptr;
};

enum class [[sattr(guid = "a9ff6d5f-620b-444b-8cb3-3b926ec1316e" serde = @enable)]]
ETextureSamplerFilterType SKR_IF_CPP( : uint32_t)
{
    NEAREST,
    LINEAR
};

enum class [[sattr(guid = "01eccfa6-ac6c-4195-b725-66c865529d6f" serde = @enable)]]
ETextureSamplerMipmapMode SKR_IF_CPP( : uint32_t)
{
    NEAREST,
    LINEAR
};

enum class [[sattr(guid = "b5dee0a2-5b20-4062-a036-79905b1d325f" serde = @enable)]]
ETextureSamplerAddressMode SKR_IF_CPP( : uint32_t)
{
    MIRROR,
    REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};

enum class [[sattr(guid = "566ef8d0-9c68-4578-be0b-b33781fc1a0f" serde = @enable)]]
ETextureSamplerCompareMode SKR_IF_CPP( : uint32_t)
{
    NEVER,
    LESS,
    EQUAL,
    LEQUAL,
    GREATER,
    NOTEQUAL,
    GEQUAL,
    ALWAYS,
};

// (GPU) texture sampler resource
struct [[sattr(guid = "ab483a53-5024-48f2-87a7-9502063c97f3" serde = @enable)]]
TextureSamplerResource
{
    ETextureSamplerFilterType min_filter;
    ETextureSamplerFilterType mag_filter;
    ETextureSamplerMipmapMode mipmap_mode;
    ETextureSamplerAddressMode address_u;
    ETextureSamplerAddressMode address_v;
    ETextureSamplerAddressMode address_w;
    float mip_lod_bias;
    float max_anisotropy;
    ETextureSamplerCompareMode compare_func;

    [[sattr(serde = @disable)]]
    CGPUSamplerId sampler = nullptr;
};

// - dstorage & bc: dstorage
// - dstorage & bc & zlib: dstorage with custom decompress queue
// - bc & zlib: [TODO] ram service & decompress service & upload
//    - upload with copy queue
//    - upload with gfx queue
struct SKR_RENDERER_API TextureFactory : public ResourceFactory
{
    virtual ~TextureFactory() = default;

    struct Root
    {
        skr_vfs_t* vfs = nullptr;
        const skr_char8* dstorage_root;
        skr::io::IRAMService* ram_service = nullptr;
        skr::io::IVRAMService* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 2.5f; }
    [[nodiscard]] static TextureFactory* Create(const Root& root);
    static void Destroy(TextureFactory* factory);
};

struct SKR_RENDERER_API TextureSamplerFactory : public ResourceFactory
{
    virtual ~TextureSamplerFactory() = default;

    struct Root
    {
        CGPUDeviceId device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 0.5f; }
    [[nodiscard]] static TextureSamplerFactory* Create(const Root& root);
    static void Destroy(TextureSamplerFactory* factory);
};
} // namespace skr