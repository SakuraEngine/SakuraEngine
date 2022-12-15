#include "SkrRenderer/render_device.h"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/shader_map.h"
#include "platform/guid.hpp"

namespace skr
{
namespace resource
{

struct SMaterialFactoryImpl : public SMaterialFactory
{
    SMaterialFactoryImpl(const SMaterialFactoryImpl::Root& root)
        : root(root)
    {
        skr_shader_map_root_t shader_map_root;
        shader_map_root.bytecode_vfs = root.bytecode_vfs;
        shader_map_root.ram_service = root.ram_service;
        shader_map_root.render_device = root.render_device;
        shader_map_root.aux_service = root.aux_service;
        shader_map = skr_shader_map_create(&shader_map_root);
    }

    ~SMaterialFactoryImpl()
    {
        skr_shader_map_free(shader_map);
    }

    skr_type_id_t GetResourceType() override
    {
        return skr::type::type_id<skr_material_resource_t>::get();
    }
    
    bool AsyncIO() override { return true; }
    
    bool Unload(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        SkrDelete(material);
        return true;
    }

    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        if (!material->material_type.is_resolved()) 
            material->material_type.resolve(true, nullptr);
        auto matType = material->material_type.get_resolved();
        material->runtime_installed.reserve(matType->shader_resources.size()); // early reserve
        // install shaders
        for (auto& shader : matType->shader_resources)
        {
            bool installed = false;
            if (!shader.is_resolved()) 
                shader.resolve(true, nullptr);
            const auto pShaderCollection = shader.get_resolved();
            const auto shaderCollectionGUID = shader.get_record()->header.guid;
            for (auto switchVariant : material->overrides.switch_variants)
            {
                const auto theCollectionGUID = switchVariant.shader_collection;
                if (theCollectionGUID == shaderCollectionGUID) // hit this variant
                {
                    const auto switch_hash = switchVariant.switch_hash;
                    const auto option_hash = switchVariant.option_hash;
                    const auto platform_ids = pShaderCollection->GetStaticVariant(switch_hash).GetDynamicVariants(option_hash);
                    for (auto platform_id : platform_ids)
                    {
                        const auto bytecode_type = SShaderResourceFactory::GetRuntimeBytecodeType(root.render_device->get_backend());
                        if (bytecode_type == platform_id.bytecode_type)
                        {
                            const auto status = shader_map->install_shader(platform_id);
                            if (status != SKR_SHADER_MAP_SHADER_STATUS_FAILED)
                            {
                                material->runtime_installed.emplace_back(platform_id);
                                installed = true;
                            }
                            else
                            {
                                SKR_UNREACHABLE_CODE();
                            }
                        }
                    }
                }
            }
            SKR_ASSERT(installed && "Specific shader resource in material not installed!");
        }
        // make RS
        
        // make PSO

        return material ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_FAILED;
    }
    
    bool Uninstall(skr_resource_record_t* record) override
    {
        return true;
    }
    
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        return SKR_INSTALL_STATUS_SUCCEED;
    }

    skr_shader_map_id shader_map = nullptr;
    Root root;
};

SMaterialFactory* SMaterialFactory::Create(const Root &root)
{
    return SkrNew<SMaterialFactoryImpl>(root);
}

void SMaterialFactory::Destroy(SMaterialFactory *factory)
{
    SkrDelete(factory);
}

}
}