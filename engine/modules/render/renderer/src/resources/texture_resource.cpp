#include "SkrCore/memory/sp.hpp"
#include "SkrGraphics/api.h"
#include "SkrRuntime/io/ram_io.hpp"
#include <SkrOS/filesystem.hpp>
#include "SkrBase/misc/debug.h"
#include "SkrRuntime/io/vram_io.hpp"
#include "SkrRuntime/resource/resource_factory.hpp"
#include "SkrRuntime/resource/resource_system.hpp"
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

// - dstorage & bc: dstorage
// - dstorage & bc & zlib: dstorage with custom decompress queue
// - bc & zlib: [TODO] ram service & decompress service & upload
//    - upload with copy queue
//    - upload with gfx queue
struct SKR_RENDERER_API TextureFactoryImpl : public TextureFactory
{
    TextureFactoryImpl(const TextureFactory::Root& root)
        : root(root)
    {
        dstorage_root = skr::String::From(root.dstorage_root);
        this->root.dstorage_root = dstorage_root.c_str();
    }
    ~TextureFactoryImpl() noexcept = default;
    GUID GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(SResourceRecord* record) override;
    ESkrInstallStatus Install(SResourceRecord* record) override;
    bool Uninstall(SResourceRecord* record) override;
    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override;

    ESkrInstallStatus InstallImpl(SResourceRecord* record);

    enum class ECompressMethod : uint32_t
    {
        BC_OR_ASTC,
        ZLIB,
        IMAGE_CODER,
        COUNT
    };

    struct InstallType
    {
        ECompressMethod compress_method;
    };

    struct TextureRequest
    {
        ~TextureRequest()
        {
            SKR_LOG_TRACE(u8"DStorage for texture resource %s finished!", absPath.c_str());
        }
        std::string absPath;
        skr_io_future_t vtexture_future;
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

    skr::String dstorage_root;
    Root root;
    skr::FlatHashMap<TextureResource*, InstallType> mInstallTypes;
    skr::FlatHashMap<TextureResource*, SP<TextureRequest>> mTextureRequests;
};

TextureFactory* TextureFactory::Create(const Root& root)
{
    return SkrNew<TextureFactoryImpl>(root);
}

void TextureFactory::Destroy(TextureFactory* factory)
{
    SkrDelete(factory);
}

GUID TextureFactoryImpl::GetResourceType()
{
    const auto resource_type = ::skr::type_id_of<TextureResource>();
    return resource_type;
}

bool TextureFactoryImpl::Unload(SResourceRecord* record)
{
    auto texture_resource = (TextureResource*)record->resource;
    if (texture_resource->texture_view) cgpu_free_texture_view(texture_resource->texture_view);
    if (texture_resource->texture) cgpu_free_texture(texture_resource->texture);
    SkrDelete(texture_resource);
    return true;
}

ESkrInstallStatus TextureFactoryImpl::Install(SResourceRecord* record)
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

ESkrInstallStatus TextureFactoryImpl::InstallImpl(SResourceRecord* record)
{
    const auto gpuCompressOnly = true;
    auto vram_service = root.vram_service;
    auto texture_resource = (TextureResource*)record->resource;
    auto guid = record->activeRequest->GetGuid();
    if (auto render_device = root.render_device)
    {
        auto batch = vram_service->open_batch(1);
        if (gpuCompressOnly)
        {
            const char* suffix = GetSuffixWithCompressionFormat((ECGPUFormat)texture_resource->format);
            auto compressedBin = skr::format(u8"{}{}", guid, suffix); // TODO: choose compression format
            auto dRequest = SP<TextureRequest>::New();
            InstallType installType = { ECompressMethod::BC_OR_ASTC };
            auto found = mTextureRequests.find(texture_resource);
            SKR_ASSERT(found == mTextureRequests.end());
            mTextureRequests.emplace(texture_resource, dRequest);
            mInstallTypes.emplace(texture_resource, installType);
            // auto compressedPath = skr::fs::path(root.dstorage_root) / compressedBin.c_str();
            // dRequest->absPath = compressedPath.string();

            CGPUTextureDescriptor tdesc = {};
            tdesc.usages = CGPU_TEXTURE_USAGE_SHADER_READ;
            tdesc.name = nullptr; // TODO: Name
            tdesc.width = texture_resource->width;
            tdesc.height = texture_resource->height;
            tdesc.depth = texture_resource->depth;
            tdesc.format = (ECGPUFormat)texture_resource->format;
            tdesc.mip_levels = texture_resource->mips_count;

            auto request = vram_service->open_texture_request();
            request->set_vfs(root.vfs);
            request->set_path(compressedBin.c_str());
            request->set_texture(render_device->get_cgpu_device(), &tdesc);
            request->set_transfer_queue(render_device->get_cpy_queue());
            auto result = batch->add_request(request, &dRequest->vtexture_future);
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

bool TextureFactoryImpl::Uninstall(SResourceRecord* record)
{
    return true;
}

ESkrInstallStatus TextureFactoryImpl::UpdateInstall(SResourceRecord* record)
{
    auto texture_resource = (TextureResource*)record->resource;
    [[maybe_unused]] auto installType = mInstallTypes[texture_resource];
    auto dRequest = mTextureRequests.find(texture_resource);
    if (dRequest != mTextureRequests.end())
    {
        bool okay = dRequest->second->vtexture_future.is_ready();
        auto status = okay ? ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED : ESkrInstallStatus::SKR_INSTALL_STATUS_INPROGRESS;
        if (okay)
        {
            texture_resource->texture = dRequest->second->io_texture->get_texture();
            // TODO: mipmap view
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = texture_resource->texture;
            view_desc.array_layer_count = 1;
            view_desc.base_array_layer = 0;
            view_desc.mip_level_count = 1;
            view_desc.base_mip_level = 0;
            view_desc.aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
            view_desc.dims = CGPU_TEXTURE_DIMENSION_2D;
            view_desc.format = (ECGPUFormat)texture_resource->format;
            view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_SRV;
            texture_resource->texture_view = cgpu_create_texture_view(root.render_device->get_cgpu_device(), &view_desc);

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
} // namespace skr
