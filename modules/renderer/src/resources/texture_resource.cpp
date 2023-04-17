#include <platform/filesystem.hpp>
#include "SkrRenderer/resources/texture_resource.h"
#include "cgpu/api.h"
#include "containers/text.hpp"
#include "containers/sptr.hpp"
#include "containers/hashmap.hpp"
#include "platform/configure.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"
#include "type/type_id.hpp"
#include "utils/format.hpp"
#include "SkrRenderer/render_device.h"
#include "utils/io.h"
#include "cgpu/io.h"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "platform/debug.h"

#ifdef _WIN32
//#include "cgpu/extensions/dstorage_windows.h"
#endif

#include "tracy/Tracy.hpp"

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
        dstorage_root = skr::text::text::from_utf8(root.dstorage_root);
        this->root.dstorage_root = dstorage_root.c_str();
    }
    ~STextureFactoryImpl() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    
    ESkrInstallStatus InstallWithDStorage(skr_resource_record_t* record);
    ESkrInstallStatus InstallWithUpload(skr_resource_record_t* record);

    enum class EInstallMethod : uint32_t
    {
        DSTORAGE,
        UPLOAD,
        COUNT
    };

    enum class ECompressMethod : uint32_t
    {
        BC_OR_ASTC,
        ZLIB,
        IMAGE_CODER,
        COUNT
    };

    struct InstallType
    {
        EInstallMethod install_method;
        ECompressMethod compress_method;
    };

    struct DStorageRequest
    {
        DStorageRequest()
        {
            texture_destination.texture = nullptr;
        }
        ~DStorageRequest()
        {
            SKR_LOG_TRACE("DStorage for texture resource %s finished!", absPath.c_str());
        }
        std::string absPath;
        skr_async_request_t vtexture_request;
        skr_async_vtexture_destination_t texture_destination = {};
    };

    struct UploadRequest
    {
        UploadRequest() = default;
        UploadRequest(STextureFactoryImpl* factory, const char* resource_uri, skr_texture_resource_id texture_resource)
            : factory(factory), resource_uri(resource_uri), texture_resource(texture_resource)
        {
            texture_destination.texture = nullptr;
        }
        ~UploadRequest()
        {
            SKR_LOG_TRACE("Upload for texture resource %s finished!", resource_uri.c_str());
        }
        STextureFactoryImpl* factory = nullptr;
        std::string resource_uri;
        skr_texture_resource_id texture_resource = nullptr;
        skr_async_request_t ram_request;
        skr_async_ram_destination_t ram_destination;
        skr_async_request_t vram_request;
        skr_async_vtexture_destination_t texture_destination = {};
    };

    // TODO: refactor this
    const char* GetSuffixWithCompressionFormat(ECGPUFormat format)
    {
        switch (format) 
        {
            case CGPU_FORMAT_DXBC1_RGB_UNORM:
            case CGPU_FORMAT_DXBC1_RGB_SRGB:
            case CGPU_FORMAT_DXBC1_RGBA_UNORM:
            case CGPU_FORMAT_DXBC1_RGBA_SRGB:
                return ".bc1";
            case CGPU_FORMAT_DXBC2_UNORM:
            case CGPU_FORMAT_DXBC2_SRGB:
                return ".bc2";
            case CGPU_FORMAT_DXBC3_UNORM:
            case CGPU_FORMAT_DXBC3_SRGB:
                return ".bc3";
            case CGPU_FORMAT_DXBC4_UNORM:
            case CGPU_FORMAT_DXBC4_SNORM:
                return ".bc4";
            case CGPU_FORMAT_DXBC5_UNORM:
            case CGPU_FORMAT_DXBC5_SNORM:
                return ".bc5";
            case CGPU_FORMAT_DXBC6H_UFLOAT:
            case CGPU_FORMAT_DXBC6H_SFLOAT:
                return ".bc6h";
            case CGPU_FORMAT_DXBC7_UNORM:
            case CGPU_FORMAT_DXBC7_SRGB:
                return ".bc7";
            default:
                return ".raw";
        }
        return ".raw";
    }

    skr::text::text dstorage_root;
    Root root;
    skr::flat_hash_map<skr_texture_resource_id, InstallType> mInstallTypes;
    skr::flat_hash_map<skr_texture_resource_id, SPtr<UploadRequest>> mUploadRequests;
    skr::flat_hash_map<skr_texture_resource_id, SPtr<DStorageRequest>> mDStorageRequests;
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
    const auto resource_type = skr::type::type_id<skr_texture_resource_t>::get();
    return resource_type;
}

bool STextureFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto texture_resource = (skr_texture_resource_t*)record->resource;
    if (texture_resource->texture_view) cgpu_free_texture_view(texture_resource->texture_view);
    if (texture_resource->texture) cgpu_free_texture(texture_resource->texture);
    SkrDelete(texture_resource);
    return true; 
}

ESkrInstallStatus STextureFactoryImpl::Install(skr_resource_record_t* record)
{
    if (auto render_device = root.render_device)
    {
        // direct storage
        if (auto file_dstorage_queue = render_device->get_file_dstorage_queue())
        {
            return InstallWithDStorage(record);
        }
        else
        {
            return InstallWithUpload(record);
        }
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
}

ESkrInstallStatus STextureFactoryImpl::InstallWithDStorage(skr_resource_record_t* record)
{
    const auto gpuCompressOnly = true;
    auto texture_resource = (skr_texture_resource_t*)record->resource;
    auto guid = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        // direct storage
        if (auto file_dstorage_queue = render_device->get_file_dstorage_queue())
        {
            if (gpuCompressOnly)
            {
                const char* suffix = GetSuffixWithCompressionFormat((ECGPUFormat)texture_resource->format);
                auto compressedBin = skr::format("{}{}", guid, suffix); //TODO: choose compression format
                auto compressedPath = skr::filesystem::path(root.dstorage_root) / compressedBin.c_str();
                auto dRequest = SPtr<DStorageRequest>::Create();
                InstallType installType = {EInstallMethod::DSTORAGE, ECompressMethod::BC_OR_ASTC};
                auto found = mDStorageRequests.find(texture_resource);
                SKR_ASSERT(found == mDStorageRequests.end());
                mDStorageRequests.emplace(texture_resource, dRequest);
                mInstallTypes.emplace(texture_resource, installType);
                dRequest->absPath = compressedPath.string();

                auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
                vram_texture_io.device = render_device->get_cgpu_device();
                vram_texture_io.dstorage.path = (const char8_t*)dRequest->absPath.c_str();
                vram_texture_io.dstorage.compression = CGPU_DSTORAGE_COMPRESSION_NONE;
                vram_texture_io.dstorage.source_type = CGPU_DSTORAGE_SOURCE_FILE;
                vram_texture_io.dstorage.queue = file_dstorage_queue;
                vram_texture_io.dstorage.uncompressed_size = texture_resource->data_size;
                
                vram_texture_io.vtexture.resource_types = CGPU_RESOURCE_TYPE_NONE;
                vram_texture_io.vtexture.texture_name = nullptr; // TODO: Name
                vram_texture_io.vtexture.width = texture_resource->width;
                vram_texture_io.vtexture.height = texture_resource->height;
                vram_texture_io.vtexture.depth = texture_resource->depth;
                vram_texture_io.vtexture.format = (ECGPUFormat)texture_resource->format;

                vram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data){

                };
                vram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = nullptr;
                
                root.vram_service->request(&vram_texture_io, &dRequest->vtexture_request, &dRequest->texture_destination);
            }
            else
            {
                SKR_UNIMPLEMENTED_FUNCTION();
            }        
        }
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

ESkrInstallStatus STextureFactoryImpl::InstallWithUpload(skr_resource_record_t* record)
{
    const auto gpuCompressOnly = true;
    auto texture_resource = (skr_texture_resource_t*)record->resource;
    auto guid = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        if (gpuCompressOnly)
        {
            const char* suffix = GetSuffixWithCompressionFormat((ECGPUFormat)texture_resource->format);
            auto uRequest = SPtr<UploadRequest>::Create(
                this, fmt::format("{}{}", guid, suffix).c_str(), texture_resource);
            InstallType installType = {EInstallMethod::UPLOAD, ECompressMethod::BC_OR_ASTC};
            auto found = mUploadRequests.find(texture_resource);
            SKR_ASSERT(found == mUploadRequests.end());
            mUploadRequests.emplace(texture_resource, uRequest);
            mInstallTypes.emplace(texture_resource, installType);

            // emit ram request
            auto ram_texture_io = make_zeroed<skr_ram_io_t>();
            ram_texture_io.path = (const char8_t*)uRequest->resource_uri.c_str();
            ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
                ZoneScopedN("Upload Image");
                // upload
                auto uRequest = (UploadRequest*)data;
                auto factory = uRequest->factory;
                auto render_device = factory->root.render_device;
                auto texture_resource = uRequest->texture_resource;

                auto& texture_io_request = uRequest->vram_request;
                auto& texture_destination = uRequest->texture_destination;
                auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
                vram_texture_io.device = render_device->get_cgpu_device();
                vram_texture_io.transfer_queue = render_device->get_cpy_queue();

                SKR_ASSERT(texture_resource->data_size == uRequest->ram_destination.size);

                vram_texture_io.vtexture.texture_name = nullptr; // TODO: debug name
                vram_texture_io.vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;
                vram_texture_io.vtexture.width = texture_resource->width;
                vram_texture_io.vtexture.height = texture_resource->height;
                vram_texture_io.vtexture.depth = texture_resource->depth;
                vram_texture_io.vtexture.format = (ECGPUFormat)texture_resource->format;
                vram_texture_io.src_memory.size = texture_resource->data_size;
                vram_texture_io.src_memory.bytes = uRequest->ram_destination.bytes;
                vram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data){};
                vram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = nullptr;
                factory->root.vram_service->request(&vram_texture_io, &texture_io_request, &texture_destination);
            };
            ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)uRequest.get();
            root.ram_service->request(root.vfs, &ram_texture_io, &uRequest->ram_request, &uRequest->ram_destination);
        }
        else
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

bool STextureFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true; 
}

ESkrInstallStatus STextureFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    auto texture_resource = (skr_texture_resource_t*)record->resource;
    auto installType = mInstallTypes[texture_resource];
    if (installType.install_method == EInstallMethod::DSTORAGE)
    {
        auto dRequest = mDStorageRequests.find(texture_resource);
        if (dRequest != mDStorageRequests.end())
        {
            bool okay = dRequest->second->vtexture_request.is_ready();
            auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
            if (okay)
            {
                texture_resource->texture = dRequest->second->texture_destination.texture;
                // TODO: mipmap view
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = texture_resource->texture;
                view_desc.array_layer_count = 1;
                view_desc.base_array_layer = 0;
                view_desc.mip_level_count = 1;
                view_desc.base_mip_level = 0;
                view_desc.aspects = CGPU_TVA_COLOR;
                view_desc.dims = CGPU_TEX_DIMENSION_2D;
                view_desc.format = (ECGPUFormat)texture_resource->format;
                view_desc.usages = CGPU_TVU_SRV;
                texture_resource->texture_view = cgpu_create_texture_view(root.render_device->get_cgpu_device(), &view_desc);

                mDStorageRequests.erase(texture_resource);
                mInstallTypes.erase(texture_resource);
            }
            return status;
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }
    else if (installType.install_method == EInstallMethod::UPLOAD)
    {
        auto uRequest = mUploadRequests.find(texture_resource);
        if (uRequest != mUploadRequests.end())
        {
            bool okay = uRequest->second->vram_request.is_ready();
            auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
            if (okay)
            {
                texture_resource->texture = uRequest->second->texture_destination.texture;
                // TODO: mipmap view
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = texture_resource->texture;
                view_desc.array_layer_count = 1;
                view_desc.base_array_layer = 0;
                view_desc.mip_level_count = 1;
                view_desc.base_mip_level = 0;
                view_desc.aspects = CGPU_TVA_COLOR;
                view_desc.dims = CGPU_TEX_DIMENSION_2D;
                view_desc.format = (ECGPUFormat)texture_resource->format;
                view_desc.usages = CGPU_TVU_SRV;
                texture_resource->texture_view = cgpu_create_texture_view(root.render_device->get_cgpu_device(), &view_desc);

                mUploadRequests.erase(texture_resource);
                mInstallTypes.erase(texture_resource);
            }
            return status;
        }
        else
        {
            SKR_UNREACHABLE_CODE();
        }
    }

    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

}
}
