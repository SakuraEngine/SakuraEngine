#include "skr_renderer/resources/texture_resource.h"
#include "resource/resource_system.h"
#include "utils/make_zeroed.hpp"
#include "platform/debug.h"
#include "cgpu/io.hpp"

#ifdef _WIN32
#include "cgpu/extensions/dstorage_windows.h"
#endif

#include "tracy/Tracy.hpp"

/*
void skr_render_texture_create_from_png(skr_render_texture_io_t* io, const char* texture_path, 
    skr_render_texture_request_t* texture_io_request, skr_async_vtexture_destination_t* texture_destination)
{
    SKR_UNIMPLEMENTED_FUNCTION();
#ifdef _WIN32
    if (io->file_dstorage_queue)
    {
        ZoneScopedN("RequestLive2DTexture");

        auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
        auto resolution = 2048;
        vram_texture_io.device = io->device;

        vram_texture_io.dstorage.path = texture_path;
        vram_texture_io.dstorage.compression = SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE;
        vram_texture_io.dstorage.source_type = CGPU_DSTORAGE_SOURCE_FILE;
        vram_texture_io.dstorage.queue = io->file_dstorage_queue;
        vram_texture_io.dstorage.uncompressed_size = resolution * resolution * 4;

        vram_texture_io.vtexture.resource_types = CGPU_RESOURCE_TYPE_NONE;
        vram_texture_io.vtexture.texture_name = texture_path;
        vram_texture_io.vtexture.width = resolution;
        vram_texture_io.vtexture.height = resolution;
        vram_texture_io.vtexture.depth = 1;
        vram_texture_io.vtexture.format = CGPU_FORMAT_R8G8B8A8_UNORM;
        
        vram_texture_io.src_memory.size = resolution * resolution * 4;

        vram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data){
        };
        vram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = nullptr;
        io->vram_service->request(&vram_texture_io, &texture_io_request->vtexture_request, texture_destination);
    }
#endif
}
*/

namespace skr
{
namespace resource
{

skr_type_id_t STextureFactory::GetResourceType()
{
     return {};
}

bool STextureFactory::AsyncIO()
{
     return true; 
}

ESkrLoadStatus STextureFactory::Load(skr_resource_record_t* record)
{ 
    auto newTexture = SkrNew<skr_texture_resource_t>();    
    auto resourceRequest = record->activeRequest;
    auto loadedData = resourceRequest->GetData();

    struct SpanReader
    {
        gsl::span<const uint8_t> data;
        size_t offset = 0;
        int read(void* dst, size_t size)
        {
            if (offset + size > data.size())
                return -1;
            memcpy(dst, data.data() + offset, size);
            offset += size;
            return 0;
        }
    } reader = {loadedData};

    skr_binary_reader_t archive{reader};
    skr::binary::Archive(&archive, *newTexture);

    record->resource = newTexture;
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

ESkrLoadStatus STextureFactory::UpdateLoad(skr_resource_record_t* record)
{
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

bool STextureFactory::Unload(skr_resource_record_t* record)
{ 
    SkrDelete((skr_texture_resource_t*)record->resource);
    return true; 
}

ESkrInstallStatus STextureFactory::Install(skr_resource_record_t* record)
{
    return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; 
}

bool STextureFactory::Uninstall(skr_resource_record_t* record)
{
    return true; 
}

ESkrInstallStatus STextureFactory::UpdateInstall(skr_resource_record_t* record)
{
    return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; 
}

void STextureFactory::DestroyResource(skr_resource_record_t* record)
{
    return; 
}

}
}