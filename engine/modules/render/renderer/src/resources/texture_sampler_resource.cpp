#include "SkrRenderer/resources/texture_resource.h"

namespace skr
{
struct SKR_RENDERER_API TextureSamplerFactoryImpl : public TextureSamplerFactory
{
    TextureSamplerFactoryImpl(const TextureSamplerFactory::Root& root)
        : root(root)
    {
    }
    ~TextureSamplerFactoryImpl() noexcept = default;
    bool AsyncIO() override { return true; }

    GUID GetResourceType() override;
    bool Unload(SResourceRecord* record) override;
    ESkrInstallStatus Install(SResourceRecord* record) override;
    bool Uninstall(SResourceRecord* record) override;
    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override;

    Root root;

    ECGPUFilterType translate(ETextureSamplerFilterType type)
    {
        switch (type)
        {
        case ETextureSamplerFilterType::NEAREST:
            return CGPU_FILTER_TYPE_NEAREST;
        case ETextureSamplerFilterType::LINEAR:
            return CGPU_FILTER_TYPE_LINEAR;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_FILTER_TYPE_MAX_ENUM_BIT;
        }
    }

    ECGPUMipMapMode translate(ETextureSamplerMipmapMode v)
    {
        switch (v)
        {
        case ETextureSamplerMipmapMode::NEAREST:
            return CGPU_MIPMAP_MODE_NEAREST;
        case ETextureSamplerMipmapMode::LINEAR:
            return CGPU_MIPMAP_MODE_LINEAR;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_MIPMAP_MODE_MAX_ENUM_BIT;
        }
    }

    ECGPUAddressMode translate(ETextureSamplerAddressMode v)
    {
        switch (v)
        {
        case ETextureSamplerAddressMode::MIRROR:
            return CGPU_ADDRESS_MODE_MIRROR;
        case ETextureSamplerAddressMode::REPEAT:
            return CGPU_ADDRESS_MODE_REPEAT;
        case ETextureSamplerAddressMode::CLAMP_TO_EDGE:
            return CGPU_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ETextureSamplerAddressMode::CLAMP_TO_BORDER:
            return CGPU_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_ADDRESS_MODE_MAX_ENUM_BIT;
        }
    }
    ECGPUCompareMode translate(ETextureSamplerCompareMode v)
    {
        switch (v)
        {
        case ETextureSamplerCompareMode::NEVER:
            return CGPU_CMP_NEVER;
        case ETextureSamplerCompareMode::LESS:
            return CGPU_CMP_LESS;
        case ETextureSamplerCompareMode::EQUAL:
            return CGPU_CMP_EQUAL;
        case ETextureSamplerCompareMode::LEQUAL:
            return CGPU_CMP_LEQUAL;
        case ETextureSamplerCompareMode::GREATER:
            return CGPU_CMP_GREATER;
        case ETextureSamplerCompareMode::NOTEQUAL:
            return CGPU_CMP_NOTEQUAL;
        case ETextureSamplerCompareMode::GEQUAL:
            return CGPU_CMP_GEQUAL;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_CMP_MAX_ENUM_BIT;
        }
    }
};

TextureSamplerFactory* TextureSamplerFactory::Create(const Root& root)
{
    return SkrNew<TextureSamplerFactoryImpl>(root);
}

void TextureSamplerFactory::Destroy(TextureSamplerFactory* factory)
{
    SkrDelete(factory);
}

GUID TextureSamplerFactoryImpl::GetResourceType()
{
    const auto resource_type = ::skr::type_id_of<TextureSamplerResource>();
    return resource_type;
}

bool TextureSamplerFactoryImpl::Unload(SResourceRecord* record)
{
    auto sampler_resource = (TextureSamplerResource*)record->resource;
    if (sampler_resource->sampler) cgpu_free_sampler(sampler_resource->sampler);
    SkrDelete(sampler_resource);
    return true;
}

ESkrInstallStatus TextureSamplerFactoryImpl::Install(SResourceRecord* record)
{
    auto sampler_resource = (TextureSamplerResource*)record->resource;
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.min_filter = translate(sampler_resource->min_filter);
    sampler_desc.mag_filter = translate(sampler_resource->mag_filter);
    sampler_desc.mipmap_mode = translate(sampler_resource->mipmap_mode);
    sampler_desc.address_u = translate(sampler_resource->address_u);
    sampler_desc.address_v = translate(sampler_resource->address_v);
    sampler_desc.address_w = translate(sampler_resource->address_w);
    sampler_desc.mip_lod_bias = sampler_resource->mip_lod_bias;
    sampler_desc.max_anisotropy = sampler_resource->max_anisotropy;
    sampler_desc.compare_func = translate(sampler_resource->compare_func);

    sampler_resource->sampler = cgpu_create_sampler(root.device, &sampler_desc);
    return sampler_resource->sampler ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
}

bool TextureSamplerFactoryImpl::Uninstall(SResourceRecord* record)
{
    return true;
}

ESkrInstallStatus TextureSamplerFactoryImpl::UpdateInstall(SResourceRecord* record)
{
    auto sampler_resource = (TextureSamplerResource*)record->resource;
    return sampler_resource->sampler ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
}

} // namespace skr