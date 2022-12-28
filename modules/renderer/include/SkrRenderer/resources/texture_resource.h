#pragma once
#include "SkrRenderer/fwd_types.h"
#include "utils/io.h"
#include "cgpu/io.h"
#ifndef __meta__
#include "SkrRenderer/resources/texture_resource.generated.h"
#endif

// (GPU) texture resource
sreflect_struct("guid" : "f8821efb-f027-4367-a244-9cc3efb3a3bf")
sattr("rtti" : true)
sattr("serialize" : "bin")
skr_texture_resource_t
{
    uint32_t format; // TODO: TEnum<ECGPUFormat>
    uint32_t mips_count;
    uint64_t data_size;
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    spush_attr("no-rtti" : true, "transient": true)
    CGPUTextureId texture;
    CGPUTextureViewId texture_view;
};
typedef struct skr_texture_resource_t skr_texture_resource_t;
typedef struct skr_texture_resource_t* skr_texture_resource_id;

sreflect_enum_class("guid" : "a9ff6d5f-620b-444b-8cb3-3b926ec1316e")
sattr("rtti" : true, "serialize" : ["bin", "json"])
ESkrTextureSamplerFilterType SKR_IF_CPP(: uint32_t)
{
    NEAREST,
    LINEAR
};

sreflect_enum_class("guid" : "01eccfa6-ac6c-4195-b725-66c865529d6f")
sattr("rtti" : true, "serialize" : ["bin", "json"])
ESkrTextureSamplerMipmapMode SKR_IF_CPP(: uint32_t)
{
    NEAREST,
    LINEAR
};

sreflect_enum_class("guid" : "b5dee0a2-5b20-4062-a036-79905b1d325f")
sattr("rtti" : true, "serialize" : ["bin", "json"])
ESkrTextureSamplerAddressMode SKR_IF_CPP(: uint32_t)
{
    MIRROR,
    REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};

sreflect_enum_class("guid" : "566ef8d0-9c68-4578-be0b-b33781fc1a0f")
sattr("rtti" : true, "serialize" : ["bin", "json"])
ESkrTextureSamplerCompareMode SKR_IF_CPP(: uint32_t)
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
sreflect_struct("guid" : "ab483a53-5024-48f2-87a7-9502063c97f3")
sattr("rtti" : true, "serialize" : ["bin", "json"])
skr_texture_sampler_resource_t
{
    ESkrTextureSamplerFilterType min_filter;
    ESkrTextureSamplerFilterType mag_filter;
    ESkrTextureSamplerMipmapMode mipmap_mode;
    ESkrTextureSamplerAddressMode address_u;
    ESkrTextureSamplerAddressMode address_v;
    ESkrTextureSamplerAddressMode address_w;
    float mip_lod_bias;
    float max_anisotropy;
    ESkrTextureSamplerCompareMode compare_func;

    spush_attr("no-rtti" : true, "transient": true)
    CGPUSamplerId sampler;
};
typedef struct skr_texture_resource_t skr_texture_resource_t;
typedef struct skr_texture_resource_t* skr_texture_resource_id;

#ifdef __cplusplus
#include "resource/resource_factory.h"

namespace skr sreflect
{
namespace resource sreflect
{
// - dstorage & bc: dstorage
// - dstorage & bc & zlib: dstorage with custom decompress queue
// - bc & zlib: [TODO] ram service & decompress service & upload
//    - upload with copy queue
//    - upload with gfx queue
struct SKR_RENDERER_API STextureFactory : public SResourceFactory {
    virtual ~STextureFactory() = default;

    struct Root {
        skr_vfs_t* vfs = nullptr;
        skr::string dstorage_root;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_io_vram_service_t* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 2.5f; }
    [[nodiscard]] static STextureFactory* Create(const Root& root);
    static void Destroy(STextureFactory* factory); 
};

struct SKR_RENDERER_API STextureSamplerFactory : public SResourceFactory {
    virtual ~STextureSamplerFactory() = default;

    struct Root {
        CGPUDeviceId device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 0.5f; }
    [[nodiscard]] static STextureSamplerFactory* Create(const Root& root);
    static void Destroy(STextureSamplerFactory* factory); 
};
} // namespace resource
} // namespace skr
#endif