#include "SkrRenderer/resources/shader_resource.hpp"
#include "platform/memory.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"

namespace skr
{
namespace resource
{

struct SKR_RENDERER_API SShaderResourceFactoryImpl : public SShaderResourceFactory
{
    SShaderResourceFactoryImpl(const SShaderResourceFactory::Root& root)
        : root(root)
    {

    }

    ~SShaderResourceFactoryImpl() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    ESkrLoadStatus Load(skr_resource_record_t* record) override;
    ESkrLoadStatus UpdateLoad(skr_resource_record_t* record) override;
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    void DestroyResource(skr_resource_record_t* record) override;

    Root root;
};

SShaderResourceFactory* SShaderResourceFactory::Create(const Root& root)
{
    return SkrNew<SShaderResourceFactoryImpl>(root);
}

void SShaderResourceFactory::Destroy(SShaderResourceFactory *factory)
{
    return SkrDelete(factory);
}

skr_type_id_t SShaderResourceFactoryImpl::GetResourceType()
{
    const auto resource_type = skr::type::type_id<skr_platform_shader_resource_t>::get();
    return resource_type;
}

ESkrLoadStatus SShaderResourceFactoryImpl::Load(skr_resource_record_t* record)
{ 
    auto newShader = SkrNew<skr_platform_shader_resource_t>();    
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
    skr::binary::Archive(&archive, *newShader);

    record->resource = newShader;
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

ESkrLoadStatus SShaderResourceFactoryImpl::UpdateLoad(skr_resource_record_t* record)
{
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

bool SShaderResourceFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto shader_resource = (skr_platform_shader_resource_t*)record->resource;
    if (shader_resource->shader) cgpu_free_shader_library(shader_resource->shader);
    SkrDelete(shader_resource);
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::Install(skr_resource_record_t* record)
{
    // TODO: load & create shader
    return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
}

bool SShaderResourceFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
}

void SShaderResourceFactoryImpl::DestroyResource(skr_resource_record_t* record)
{
    return;
}

} // namespace resource
} // namespace skr