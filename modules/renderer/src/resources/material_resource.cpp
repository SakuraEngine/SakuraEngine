#include <EASTL/fixed_vector.h>
#include "platform/guid.hpp"
#include "containers/sptr.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/shader_map.h"
#include "SkrRenderer/pso_map.h"
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
        // 1.create shader map
        skr_shader_map_root_t shader_map_root;
        shader_map_root.bytecode_vfs = root.bytecode_vfs;
        shader_map_root.ram_service = root.ram_service;
        shader_map_root.device = root.device;
        shader_map_root.aux_service = root.aux_service;
        shader_map = skr_shader_map_create(&shader_map_root);

        // 2.create root signature pool
        CGPURootSignaturePoolDescriptor rs_pool_desc = {};
        rs_pool_desc.name = "MaterialRootSignaturePool";
        rs_pool = cgpu_create_root_signature_pool(root.device, &rs_pool_desc);

        // 3.create pso map
        skr_pso_map_root_t pso_map_root;
        pso_map_root.aux_service = root.aux_service;
        pso_map_root.device = root.device;
        pso_map = skr_pso_map_create(&pso_map_root);
    }

    ~SMaterialFactoryImpl()
    {
        skr_pso_map_free(pso_map);
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
        // 1.free PSO & map key
        if (material->key) 
        {
            pso_map->uninstall_pso(material->key);
            pso_map->free_key(material->key);
        }

        // 2.free RS
        if (material->bind_table) cgpux_free_bind_table(material->bind_table);
        if (material->root_signature) cgpu_free_root_signature(material->root_signature);

        // 3.RC free installed shaders
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
                                SKR_UNREACHABLE_CODE(); // shader install failed handler
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
        rs_desc.shaders = ppl_shaders; 
        // TODO: static samplers & push constants
        rs_desc.push_constant_count = 1;
        const char* push_const_name = "push_constants";
        rs_desc.push_constant_names = &push_const_name;
        rs_desc.static_sampler_count = 0;
        rs_desc.static_samplers = nullptr;
        rs_desc.static_sampler_names = nullptr;
        const auto root_signature = cgpu_create_root_signature(root.device, &rs_desc);
        return root_signature;
    }

    const char* sampler_name = "color_sampler";
    CGPUXBindTableId createMaterialBindTable(const skr_material_resource_t* material, CGPURootSignatureId root_signature) const
    {
        // make bind table
        CGPUXBindTableDescriptor table_desc = {};
        table_desc.root_signature = root_signature;
        // TODO: multi bind table
        eastl::fixed_vector<const char*, 16> slot_names;
        for (uint32_t i = 0; i < root_signature->table_count; i++)
        {
            const auto& table = root_signature->tables[i];
            for (uint32_t j = 0; j < table.resources_count; j++)
            {
                const auto& resource = table.resources[j];
                if (resource.type == CGPU_RESOURCE_TYPE_SAMPLER)
                {
                    for (const auto& override : material->overrides.samplers)
                    {
                        if (override.slot_name.starts_with(resource.name) 
                        && strlen(resource.name) == override.slot_name.size()) // slot name matches
                        {
                            slot_names.emplace_back(resource.name);
                        }
                    }
                }
                else if (resource.type == CGPU_RESOURCE_TYPE_TEXTURE)
                {
                    for (const auto& override : material->overrides.textures)
                    {
                        if (override.slot_name.starts_with(resource.name) 
                        && strlen(resource.name) == override.slot_name.size()) // slot name matches
                        {
                            slot_names.emplace_back(resource.name);
                        }
                    }
                }
                else if (resource.type == CGPU_RESOURCE_TYPE_BUFFER)
                {
                    // SKR_UNIMPLEMENTED_FUNCTION();
                }
            }
        }
        table_desc.names_count = slot_names.size();
        table_desc.names = slot_names.data();
        const auto bind_table = cgpux_create_bind_table(root.device, &table_desc);
        return bind_table;
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
            material->bind_table = createMaterialBindTable(material, material->root_signature);
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
                material->bind_table = rsRequest->bind_table;
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
                rsRequest->bind_table = factory->createMaterialBindTable(rsRequest->material, rsRequest->root_signature);
            };
            aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = rsRequest.get();
            aux_service->request(&aux_task, &rsRequest->request);
            mRootSignatureRequests.emplace(materialGUID, rsRequest);
            return nullptr;
        }
    }

    skr_pso_map_key_id make_pso_map_key(skr_material_resource_t* material, skr::span<CGPUShaderLibraryId> shaders) const SKR_NOEXCEPT
    {
        auto desc = make_zeroed<CGPURenderPipelineDescriptor>();
        desc.root_signature = material->root_signature;
        // 1.fill pipeline shaders
        auto vertex_shader = make_zeroed<CGPUPipelineShaderDescriptor>();
        auto tesc_shader = make_zeroed<CGPUPipelineShaderDescriptor>();
        auto tese_shader = make_zeroed<CGPUPipelineShaderDescriptor>();
        auto geom_shader = make_zeroed<CGPUPipelineShaderDescriptor>();
        auto fragment_shader = make_zeroed<CGPUPipelineShaderDescriptor>();
        for (uint32_t i = 0; i < shaders.size(); i++)
        {
            CGPUPipelineShaderDescriptor* ref = &vertex_shader;
            switch (material->shader_stages[i])
            {
            case CGPU_SHADER_STAGE_VERT:
                ref = &vertex_shader; 
                desc.vertex_shader = &vertex_shader;
                break;
            case CGPU_SHADER_STAGE_TESC:
                ref = &tesc_shader; 
                desc.tesc_shader = &tesc_shader;
                break;
            case CGPU_SHADER_STAGE_TESE:
                ref = &tese_shader;
                desc.tese_shader = &tese_shader;
                break;
            case CGPU_SHADER_STAGE_GEOM:
                ref = &geom_shader; 
                desc.geom_shader = &geom_shader;
                break;
            case CGPU_SHADER_STAGE_FRAG:
                ref = &fragment_shader; 
                desc.fragment_shader = &fragment_shader;
                break;
            default:
                SKR_ASSERT(false && "wrong shader stage");
                break;
            }
            ref->library = shaders[i];
            ref->entry = material->shader_entries[i].data();
            ref->stage = material->shader_stages[i];
            // TODO: const spec
            ref->constants = nullptr;
            ref->num_constants = 0; 
        }
        // 2.fill vertex layout
        auto vert_layout = make_zeroed<CGPUVertexLayout>();
        const auto matType = material->material_type.get_resolved();
        const auto vertType = matType->vertex_type;
        skr_mesh_resource_query_vertex_layout(vertType, &vert_layout);
        desc.vertex_layout = &vert_layout;
        // 3.fill blend state
        auto blend_state = make_zeroed<CGPUBlendStateDescriptor>();
        blend_state.src_factors[0] = CGPU_BLEND_CONST_ONE; // TODO: MRT
        blend_state.dst_factors[0] = CGPU_BLEND_CONST_ZERO; 
        blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
        blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ZERO;
        blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD; 
        blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD; 
        blend_state.masks[0] = CGPU_COLOR_MASK_ALL; 
        blend_state.alpha_to_coverage = false;
        blend_state.independent_blend = false;
        desc.blend_state = &blend_state;
        // 4.fill depth state 
        auto depth_state = make_zeroed<CGPUDepthStateDescriptor>();
        depth_state.depth_func = CGPU_CMP_LEQUAL; // TODO: Depth Contril
        depth_state.depth_write = true; // TODO: Depth Write Control
        depth_state.depth_test = true; // TODO: Depth Test Control
        desc.depth_state = &depth_state;
        // 5.fill raster state 
        auto raster_desc = make_zeroed<CGPURasterizerStateDescriptor>();
        raster_desc.cull_mode = CGPU_CULL_MODE_BACK; // TODO: Cull Mode Control
        raster_desc.depth_bias = 0; // TODO: Depth Bias Control
        raster_desc.fill_mode = CGPU_FILL_MODE_SOLID; // TODO: Fill Mode Control
        raster_desc.front_face = CGPU_FRONT_FACE_CCW; // TODO: Front Face Control
        desc.rasterizer_state = &raster_desc;
        // 6.miscs
        const auto fmt = CGPU_FORMAT_B8G8R8A8_UNORM;
        desc.render_target_count = 1; // TODO: MRT
        desc.color_formats = &fmt; // TODO: use correct screen buffer format
        desc.sample_count = CGPU_SAMPLE_COUNT_1; // TODO: MSAA
        desc.sample_quality = 0u; // TODO: MSAA
        desc.color_resolve_disable_mask = 0u; // TODO: Color resolve mask (this is a vulkan-only feature)
        desc.depth_stencil_format = CGPU_FORMAT_D32_SFLOAT_S8_UINT; // TODO: depth stencil format
        desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST; // TODO: non-triangle list topology support
        desc.enable_indirect_command = false; // TODO: indirect command support
        return skr_pso_map_create_key(pso_map, &desc);
    }

    CGPURenderPipelineId requestPSO(skr_resource_record_t* record, skr_material_resource_t* material, skr::span<CGPUShaderLibraryId> shaders, bool& fail)
    {
        if (!material->key)
        {
            material->key = make_pso_map_key(material, shaders);
            auto status = skr_pso_map_install_pso(pso_map, material->key);
            if (status == SKR_PSO_MAP_PSO_STATUS_FAILED) fail = true;
        }
        return skr_pso_map_find_pso(pso_map, material->key);
    }

    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        // 1.all shaders are installed ?
        eastl::fixed_vector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
        for (const auto& identifier : material->installed_shaders)
        {
            if (auto library = shader_map->find_shader(identifier))
                shaders.emplace_back(library);
            else
                return SKR_INSTALL_STATUS_INPROGRESS;
        }

        // 2.make RS. CGPU has rs pools so we can just create a pooled RS here.
        // CGPU will route it to the right backend unique RootSignature.
        material->root_signature = requestRS(record, shaders);

        // 3.make PSO, root signature needs to be ready for the request.
        bool exception = false;
        material->pso = material->root_signature ? requestPSO(record, material, shaders, exception) : nullptr;

        if (exception) return SKR_INSTALL_STATUS_FAILED;
        return material->pso ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
    }

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
        CGPUXBindTableId bind_table = nullptr;
        eastl::fixed_vector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
    };
    skr::flat_hash_map<skr_guid_t, SPtr<RootSignatureRequest>, skr::guid::hash> mRootSignatureRequests;

    skr_shader_map_id shader_map = nullptr;
    skr_pso_map_id pso_map = nullptr;
    CGPURootSignaturePoolId rs_pool = nullptr;
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