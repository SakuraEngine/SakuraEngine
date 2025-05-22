#include "SkrCore/memory/sp.hpp"
#include "SkrGraphics/api.h"
#include "SkrRT/io/ram_io.hpp"
#include <SkrOS/filesystem.hpp>
#include "SkrBase/misc/debug.h"
#include "SkrRT/io/vram_io.hpp"
#include "SkrRT/resource/resource_factory.h"
#include "SkrRT/resource/resource_system.h"
#include "SkrCore/log.h"
#include "SkrBase/misc/make_zeroed.hpp"

#include "SkrContainers/string.hpp"
#include "SkrContainers/hashmap.hpp"

#include "SkrRenderer/render_device.h"
#include "SkrRenderer/resources/texture_resource.h"

#ifdef _WIN32
// #include "SkrCore/platform/win/dstorage_windows.h"
#endif

#include "SkrProfile/profile.h"

namespace skr
{
namespace resource
{

// - dstorage & bc: dstorage
// - dstorage & bc & zlib: dstorage with custom decompress queue
// - bc & zlib: [TODO] ram service & decompress service & upload
//    - upload with copy queue
//    - upload with gfx queue
struct SKR_RENDERER_API STextureFactoryImpl : public STextureFactory {
    STextureFactoryImpl(const STextureFactory::Root& root)
        : root(root)
    {
        dstorage_root            = skr::String::From(root.dstorage_root);
        this->root.dstorage_root = dstorage_root.c_str();
    }
    ~STextureFactoryImpl() noexcept = default;
    skr_guid_t        GetResourceType() override;
    bool              AsyncIO() override { return true; }
    bool              Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool              Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;

    ESkrInstallStatus InstallImpl(skr_resource_record_t* record);

    enum class ECompressMethod : uint32_t
    {
        BC_OR_ASTC,
        ZLIB,
        IMAGE_CODER,
        COUNT
    };

    struct InstallType {
        ECompressMethod compress_method;
    };

    struct TextureRequest {
        ~TextureRequest()
        {
            SKR_LOG_TRACE(u8"DStorage for texture resource %s finished!", absPath.c_str());
        }
        std::string              absPath;
        skr_io_future_t          vtexture_future;
        skr::io::VRAMIOTextureId io_texture = nullptr;
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

    skr::String                                                     dstorage_root;
    Root                                                            root;
    skr::FlatHashMap<skr_texture_resource_id, InstallType>          mInstallTypes;
    skr::FlatHashMap<skr_texture_resource_id, SP<TextureRequest>> mTextureRequests;
};

STextureFactory* STextureFactory::Create(const Root& root)
{
    return SkrNew<STextureFactoryImpl>(root);
}

void STextureFactory::Destroy(STextureFactory* factory)
{
    SkrDelete(factory);
}

skr_guid_t STextureFactoryImpl::GetResourceType()
{
    const auto resource_type = ::skr::type_id_of<skr_texture_resource_t>();
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
        return InstallImpl(record);
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
}

ESkrInstallStatus STextureFactoryImpl::InstallImpl(skr_resource_record_t* record)
{
    const auto gpuCompressOnly  = true;
    auto       vram_service     = root.vram_service;
    auto       texture_resource = (skr_texture_resource_t*)record->resource;
    auto       guid             = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        auto batch = vram_service->open_batch(1);
        if (gpuCompressOnly)
        {
            const char* suffix        = GetSuffixWithCompressionFormat((ECGPUFormat)texture_resource->format);
            auto        compressedBin = skr::format(u8"{}{}", guid, suffix); // TODO: choose compression format
            auto        dRequest      = SP<TextureRequest>::New();
            InstallType installType   = { ECompressMethod::BC_OR_ASTC };
            auto        found         = mTextureRequests.find(texture_resource);
            SKR_ASSERT(found == mTextureRequests.end());
            mTextureRequests.emplace(texture_resource, dRequest);
            mInstallTypes.emplace(texture_resource, installType);
            // auto compressedPath = skr::filesystem::path(root.dstorage_root) / compressedBin.c_str();
            // dRequest->absPath = compressedPath.string();

            CGPUTextureDescriptor tdesc = {};
            tdesc.descriptors           = CGPU_RESOURCE_TYPE_NONE;
            tdesc.name                  = nullptr; // TODO: Name
            tdesc.width                 = texture_resource->width;
            tdesc.height                = texture_resource->height;
            tdesc.depth                 = texture_resource->depth;
            tdesc.format                = (ECGPUFormat)texture_resource->format;

            auto request = vram_service->open_texture_request();
            request->set_vfs(root.vfs);
            request->set_path(compressedBin.c_str());
            request->set_texture(render_device->get_cgpu_device(), &tdesc);
            request->set_transfer_queue(render_device->get_cpy_queue());
            auto result          = batch->add_request(request, &dRequest->vtexture_future);
            dRequest->io_texture = result.cast_static<skr::io::IVRAMIOTexture>();
        }
        else
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
        vram_service->request(batch);
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

bool STextureFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true;
}

ESkrInstallStatus STextureFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    auto                  texture_resource = (skr_texture_resource_t*)record->resource;
    [[maybe_unused]] auto installType      = mInstallTypes[texture_resource];
    auto                  dRequest         = mTextureRequests.find(texture_resource);
    if (dRequest != mTextureRequests.end())
    {
        bool okay   = dRequest->second->vtexture_future.is_ready();
        auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
        if (okay)
        {
            texture_resource->texture = dRequest->second->io_texture->get_texture();
            // TODO: mipmap view
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture                   = texture_resource->texture;
            view_desc.array_layer_count         = 1;
            view_desc.base_array_layer          = 0;
            view_desc.mip_level_count           = 1;
            view_desc.base_mip_level            = 0;
            view_desc.aspects                   = CGPU_TVA_COLOR;
            view_desc.dims                      = CGPU_TEX_DIMENSION_2D;
            view_desc.format                    = (ECGPUFormat)texture_resource->format;
            view_desc.usages                    = CGPU_TVU_SRV;
            texture_resource->texture_view      = cgpu_create_texture_view(root.render_device->get_cgpu_device(), &view_desc);

            mTextureRequests.erase(texture_resource);
            mInstallTypes.erase(texture_resource);
        }
        return status;
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }

    return ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
}

} // namespace resource
} // namespace skr
