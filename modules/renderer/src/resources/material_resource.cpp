#include <EASTL/fixed_vector.h>
#include "platform/guid.hpp"
#include "containers/sptr.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
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
        shader_map_root.device = root.device;
        shader_map_root.aux_service = root.aux_service;
        shader_map = skr_shader_map_create(&shader_map_root);
        CGPURootSignaturePoolDescriptor rs_pool_desc = {};
        rs_pool_desc.name = "MaterialRootSignaturePool";
        rs_pool = cgpu_create_root_signature_pool(root.device, &rs_pool_desc);
    }

    ~SMaterialFactoryImpl()
    {
        if (rs_pool) cgpu_free_root_signature_pool(rs_pool);
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
        // free RS
        if (material->root_signature) cgpu_free_root_signature(material->root_signature);
        // RC free installed shaders
        for (const auto installedId : material->installed_shaders)
        {
            shader_map->free_shader(installedId);
        }
        SkrDelete(material);
        return true;
    }

    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        if (!material->material_type.is_resolved()) 
            material->material_type.resolve(true, nullptr);
        auto matType = material->material_type.get_resolved();
        material->installed_shaders.reserve(matType->shader_resources.size()); // early reserve
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
                    auto& multiShader = pShaderCollection->GetStaticVariant(switch_hash);
                    const auto platform_ids = multiShader.GetDynamicVariants(option_hash);
                    for (auto platform_id : platform_ids)
                    {
                        const auto backend = root.device->adapter->instance->backend;
                        const auto bytecode_type = SShaderResourceFactory::GetRuntimeBytecodeType(backend);
                        if (bytecode_type == platform_id.bytecode_type)
                        {
                            const auto status = shader_map->install_shader(platform_id);
                            if (status != SKR_SHADER_MAP_SHADER_STATUS_FAILED)
                            {
                                material->installed_shaders.emplace_back(platform_id);
                                material->shader_entries.emplace_back(multiShader.entry);
                                material->shader_stages.emplace_back(multiShader.shader_stage);
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
        return material ? SKR_INSTALL_STATUS_INPROGRESS : SKR_INSTALL_STATUS_FAILED;
    }
    
    bool Uninstall(skr_resource_record_t* record) override
    {
        return true;
    }
    
    CGPURootSignatureId createMaterialRS(const skr_material_resource_t* material, skr::span<CGPUShaderLibraryId> shaders) const
    {
        CGPUPipelineShaderDescriptor ppl_shaders[CGPU_SHADER_STAGE_COUNT];
        for (size_t i = 0; i < shaders.size(); i++)
        {
            ppl_shaders[i].library = shaders[i];
            ppl_shaders[i].entry = material->shader_entries[i].data();
            ppl_shaders[i].stage = material->shader_stages[i];
        }
        CGPURootSignatureDescriptor rs_desc = {};
        rs_desc.pool = rs_pool;
        rs_desc.shader_count = static_cast<uint32_t>(shaders.size());
        rs_desc.shaders = ppl_shaders; // FIXME: we need to get the shader pointers from the async shader map
        // TODO: static samplers & push constants
        rs_desc.push_constant_count = 0;
        rs_desc.push_constant_names = nullptr;
        rs_desc.static_sampler_count = 0;
        rs_desc.static_samplers = nullptr;
        rs_desc.static_sampler_names = nullptr;
        return cgpu_create_root_signature(root.device, &rs_desc);
    }

    CGPURootSignatureId requestRS(skr_resource_record_t* record, skr::span<CGPUShaderLibraryId> shaders)
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        // 0.return if ready
        if (material->root_signature) return material->root_signature; // already created

        // 1.sync version
        if (root.aux_service == nullptr)
        {
            material->root_signature = createMaterialRS(material, shaders);
            return material->root_signature;
        }

        // 2.async version
        const auto materialGUID = record->header.guid;
        if (auto iter = mRootSignatureRequests.find(materialGUID); iter != mRootSignatureRequests.end())
        {
            // 2.1.1 assign & erase request if finished
            const auto& rsRequest = iter->second;
            const auto ready = rsRequest->request.is_ready();
            if (ready)
            {
                material->root_signature = rsRequest->root_signature;
                mRootSignatureRequests.erase(materialGUID);
            }
            return material->root_signature;
        }
        else
        {
            // 2.2.1 fire async create task
            auto rsRequest = SPtr<RootSignatureRequest>::Create(material, this, shaders);
            auto aux_service = root.aux_service;
            auto aux_task = make_zeroed<skr_service_task_t>();
            aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                ZoneScopedN("CreateRootSignature(AuxService)");
                auto rsRequest = static_cast<RootSignatureRequest*>(usrdata);
                const auto factory = rsRequest->factory;

                rsRequest->root_signature = factory->createMaterialRS(rsRequest->material, rsRequest->shaders);
            };
            aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = rsRequest.get();
            aux_service->request(&aux_task, &rsRequest->request);
            mRootSignatureRequests.emplace(materialGUID, rsRequest);
            return nullptr;
        }
    }

    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        // all shaders are installed ?
        eastl::fixed_vector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
        for (const auto& identifier : material->installed_shaders)
        {
            if (auto library = shader_map->find_shader(identifier))
                shaders.emplace_back(library);
            else
                return SKR_INSTALL_STATUS_INPROGRESS;
        }
        // make RS. CGPU has rs pools so we can just create a pooled RS here.
        // CGPU will route it to the right backend unique RootSignature.
        const auto rootSignature = requestRS(record, shaders);
        return rootSignature ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
    }

    skr_shader_map_id shader_map = nullptr;
    CGPURootSignaturePoolId rs_pool = nullptr;
    Root root;

    struct RootSignatureRequest
    {
        RootSignatureRequest(const skr_material_resource_t* material, SMaterialFactoryImpl* factory, skr::span<CGPUShaderLibraryId> shaders)
            : material(material), factory(factory), shaders(shaders.data(), shaders.data() + shaders.size())
        {

        }
        skr_async_request_t request;
        const skr_material_resource_t* material = nullptr;
        SMaterialFactoryImpl* factory = nullptr;
        CGPURootSignatureId root_signature = nullptr;
        eastl::fixed_vector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
    };
    skr::flat_hash_map<skr_guid_t, SPtr<RootSignatureRequest>, skr::guid::hash> mRootSignatureRequests;
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