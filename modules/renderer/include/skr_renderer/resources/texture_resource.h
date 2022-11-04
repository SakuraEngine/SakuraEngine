#pragma once
#include "containers/hashmap.hpp"
#include "SkrRenderer/module.configure.h"
#include "skr_renderer/fwd_types.h"
#include "platform/filesystem.hpp"
#include "utils/io.h"
#include "utils/types.h"
#include "cgpu/io.h"
#ifndef __meta__
#include "SkrRenderer/resources/texture_resource.generated.h"
#endif

// (GPU) texture resource
sreflect_struct("guid" : "f8821efb-f027-4367-a244-9cc3efb3a3bf")
sattr("serialize" : "bin")
skr_texture_resource_t
{
    uint32_t format; // TODO: TEnum<ECGPUFormat>
    uint32_t mips_count;
    uint64_t data_size;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    sattr("transient": true)
    CGPUTextureId texture;
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
        skr_vfs_t* texture_vfs = nullptr;
        skr::filesystem::path dstorage_root;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_io_vram_service_t* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    [[nodiscard]] static STextureFactory* Create(const Root& root);
    static void Destroy(STextureFactory* factory); 
};
} // namespace resource
} // namespace skr
#endif