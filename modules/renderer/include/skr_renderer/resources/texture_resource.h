#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/io.h"
#include "utils/types.h"
#include "cgpu/io.h"
#ifndef __meta__
#include "SkrRenderer/resources/texture_resource.generated.h"
#endif

typedef struct skr_texture_resource_io_t {
    skr_io_ram_service_t* ram_service;
    skr_io_vram_service_t* vram_service;
    CGPUDeviceId device;
    CGPUQueueId copy_queue;
    CGPUDStorageQueueId file_dstorage_queue;
} skr_texture_resource_io_t;

typedef struct skr_texture_resource_request_t {
    skr_async_io_request_t ram_request;
    skr_async_io_request_t vtexture_request;
} skr_texture_resource_request_t;

// (GPU) texture resource
sreflect_struct("guid" : "f8821efb-f027-4367-a244-9cc3efb3a3bf")
sattr("serialize" : "bin")
skr_texture_resource_t
{
    uint32_t format; // TODO: TEnum<ECGPUFormat>
    uint32_t mips_count;
    uint64_t data_size;
    sattr("transient": true)
    skr_texture_resource_io_t texture_io;
    sattr("transient": true)
    skr_async_ram_destination_t ram_dest;
    sattr("transient": true)
    skr_async_vtexture_destination_t vram_dest;
};
typedef struct skr_texture_resource_t skr_texture_resource_t;

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
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override;
    ESkrLoadStatus Load(skr_resource_record_t* record) override;
    ESkrLoadStatus UpdateLoad(skr_resource_record_t* record) override;
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    void DestroyResource(skr_resource_record_t* record) override;
};
} // namespace resource
} // namespace skr
#endif