#include "SkrRenderer/resources/texture_resource.h"

#include "tracy/Tracy.hpp"

namespace skr
{
namespace resource
{
struct SKR_RENDERER_API STextureSamplerFactoryImpl : public STextureSamplerFactory
{
    STextureSamplerFactoryImpl(const STextureSamplerFactory::Root& root)
        : root(root)
    {

    }
    ~STextureSamplerFactoryImpl() noexcept = default;
    bool AsyncIO() override { return true; }

    skr_type_id_t GetResourceType() override;
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;

    Root root;

    ECGPUFilterType translate(ESkrTextureSamplerFilterType type)
    {
        switch (type)
        {
        case ESkrTextureSamplerFilterType::NEAREST: return CGPU_FILTER_TYPE_NEAREST;
        case ESkrTextureSamplerFilterType::LINEAR: return CGPU_FILTER_TYPE_LINEAR;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_FILTER_TYPE_MAX_ENUM_BIT;
        }
    }

    ECGPUMipMapMode translate(ESkrTextureSamplerMipmapMode v)
    {
        switch (v)
        {
        case ESkrTextureSamplerMipmapMode::NEAREST: return CGPU_MIPMAP_MODE_NEAREST;
        case ESkrTextureSamplerMipmapMode::LINEAR: return CGPU_MIPMAP_MODE_LINEAR;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_MIPMAP_MODE_MAX_ENUM_BIT;
        }
    }

    ECGPUAddressMode translate(ESkrTextureSamplerAddressMode v)
    {
        switch (v)
        {
        case ESkrTextureSamplerAddressMode::MIRROR: return CGPU_ADDRESS_MODE_MIRROR;
        case ESkrTextureSamplerAddressMode::REPEAT: return CGPU_ADDRESS_MODE_REPEAT;
        case ESkrTextureSamplerAddressMode::CLAMP_TO_EDGE: return CGPU_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ESkrTextureSamplerAddressMode::CLAMP_TO_BORDER: return CGPU_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_ADDRESS_MODE_MAX_ENUM_BIT;
        }
    }
    ECGPUCompareMode translate(ESkrTextureSamplerCompareMode v)
    {
        switch (v)
        {
        case ESkrTextureSamplerCompareMode::NEVER: return CGPU_CMP_NEVER;
        case ESkrTextureSamplerCompareMode::LESS: return CGPU_CMP_LESS;
        case ESkrTextureSamplerCompareMode::EQUAL: return CGPU_CMP_EQUAL;
        case ESkrTextureSamplerCompareMode::LEQUAL: return CGPU_CMP_LEQUAL;
        case ESkrTextureSamplerCompareMode::GREATER: return CGPU_CMP_GREATER;
        case ESkrTextureSamplerCompareMode::NOTEQUAL: return CGPU_CMP_NOTEQUAL;
        case ESkrTextureSamplerCompareMode::GEQUAL: return CGPU_CMP_GEQUAL;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            return CGPU_CMP_MAX_ENUM_BIT;
        }
    }
};

STextureSamplerFactory* STextureSamplerFactory::Create(const Root& root)
{
    return SkrNew<STextureSamplerFactoryImpl>(root);
}

void STextureSamplerFactory::Destroy(STextureSamplerFactory* factory)
{
    SkrDelete(factory);
}

skr_type_id_t STextureSamplerFactoryImpl::GetResourceType()
{
    const auto resource_type = skr::type::type_id<skr_texture_sampler_resource_t>::get();
    return resource_type;
}

bool STextureSamplerFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto sampler_resource = (skr_texture_sampler_resource_t*)record->resource;
    if (sampler_resource->sampler) cgpu_free_sampler(sampler_resource->sampler);
    SkrDelete(sampler_resource);
    return true; 
}

ESkrInstallStatus STextureSamplerFactoryImpl::Install(skr_resource_record_t* record)
{
    auto sampler_resource = (skr_texture_sampler_resource_t*)record->resource;
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

bool STextureSamplerFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true;
}

ESkrInstallStatus STextureSamplerFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    auto sampler_resource = (skr_texture_sampler_resource_t*)record->resource;
    return sampler_resource->sampler ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
}

}
}