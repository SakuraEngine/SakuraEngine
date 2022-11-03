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

// - dstorage & bc: dstorage
// - dstorage & bc & zlib: dstorage with custom decompress queue
// - bc & zlib: [TODO] ram service & decompress service & upload
//    - upload with copy queue
//    - upload with gfx queue
struct SKR_RENDERER_API STextureFactoryImpl : public STextureFactory
{
    STextureFactoryImpl(const STextureFactory::Root& root)
        : root(root)
    {

    }
    ~STextureFactoryImpl() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override;
    ESkrLoadStatus Load(skr_resource_record_t* record) override;
    ESkrLoadStatus UpdateLoad(skr_resource_record_t* record) override;
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    void DestroyResource(skr_resource_record_t* record) override;
    
    ESkrInstallStatus InstallWithDStorage(skr_resource_record_t* record);
    ESkrInstallStatus InstallWithQueue(skr_resource_record_t* record);

    Root root;
    skr::flat_hash_map<skr_texture_resource_id, skr_async_io_request_t> mRamRequests;
    skr::flat_hash_map<skr_texture_resource_id, skr_async_io_request_t> mVRamRequests;
    skr::flat_hash_map<skr_texture_resource_id, skr_async_io_request_t> mDStorageRequests;
};

STextureFactory* STextureFactory::Create(const Root& root)
{
    return SkrNew<STextureFactoryImpl>(root);
}

void STextureFactory::Destroy(STextureFactory* factory)
{
    SkrDelete(factory);
}

skr_type_id_t STextureFactoryImpl::GetResourceType()
{
     return {};
}

bool STextureFactoryImpl::AsyncIO()
{
     return true; 
}

ESkrLoadStatus STextureFactoryImpl::Load(skr_resource_record_t* record)
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

ESkrLoadStatus STextureFactoryImpl::UpdateLoad(skr_resource_record_t* record)
{
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

bool STextureFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    SkrDelete((skr_texture_resource_t*)record->resource);
    return true; 
}

ESkrInstallStatus STextureFactoryImpl::Install(skr_resource_record_t* record)
{
    auto guid = record->activeRequest->GetGuid();

    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

ESkrInstallStatus STextureFactoryImpl::InstallWithDStorage(skr_resource_record_t* record)
{
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

ESkrInstallStatus STextureFactoryImpl::InstallWithQueue(skr_resource_record_t* record)
{
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

bool STextureFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true; 
}

ESkrInstallStatus STextureFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; 
}

void STextureFactoryImpl::DestroyResource(skr_resource_record_t* record)
{
    return; 
}

}
}