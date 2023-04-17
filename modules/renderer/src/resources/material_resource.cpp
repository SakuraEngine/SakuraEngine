#include <EASTL/fixed_vector.h>
#include "platform/guid.hpp"
#include "containers/sptr.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
#include "SkrRenderer/render_device.h"

#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/texture_resource.h"
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
        rs_pool_desc.name = u8"MaterialRootSignaturePool";
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
    
    bool Unload_Pass(skr_material_resource_t::installed_pass& pass)
    {
        // 1.free PSO & map key
        if (pass.key) 
        {
            pso_map->uninstall_pso(pass.key);
            pso_map->free_key(pass.key);
        }

        // 2.free RS
        if (pass.bind_table) cgpux_free_bind_table(pass.bind_table);
        if (pass.root_signature) cgpu_free_root_signature(pass.root_signature);

        // 3.RC free installed shaders
        for (const auto installed_shader : pass.shaders)
        {
            shader_map->free_shader(installed_shader.identifier);
        }

        return true;
    }

    bool Unload(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        bool unloaded = true;
        for (auto& pass : material->installed_passes)
        {
            unloaded &= Unload_Pass(pass);
        }
        return unloaded;
    }

    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        if (!material->material_type.is_resolved()) 
            material->material_type.resolve(true, nullptr);
        auto matType = material->material_type.get_resolved();
        // TODO: early reserve
        // install shaders
        for (auto& pass_template : matType->passes)
        {
            auto& installed_pass = material->installed_passes.emplace_back();
            installed_pass.name = pass_template.pass;
            for (auto& shader : pass_template.shader_resources)
            {
                bool installed = false;
                if (!shader.is_resolved()) shader.resolve(true, nullptr);
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
                                    auto& installed_shader = installed_pass.shaders.emplace_back();
                                    installed_shader.identifier = platform_id;
                                    installed_shader.entry = multiShader.entry;
                                    installed_shader.stage = multiShader.shader_stage;

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
            // all shaders have been installed
            installed_pass.status = SKR_INSTALL_STATUS_INPROGRESS;
        }
        return material ? SKR_INSTALL_STATUS_INPROGRESS : SKR_INSTALL_STATUS_FAILED;
    }
    
    bool Uninstall(skr_resource_record_t* record) override
    {
        return true;
    }
    
    CGPURootSignatureId createMaterialRS( skr_material_resource_t::installed_pass& installed_pass, skr::span<CGPUShaderLibraryId> shaders) const
    {
        CGPUShaderEntryDescriptor ppl_shaders[CGPU_SHADER_STAGE_COUNT];
        for (size_t i = 0; i < installed_pass.shaders.size(); i++)
        {
            ppl_shaders[i].library = shaders[i];
            ppl_shaders[i].entry = (const char8_t*)installed_pass.shaders[i].entry.data();
            ppl_shaders[i].stage = installed_pass.shaders[i].stage;
        }
        CGPURootSignatureDescriptor rs_desc = {};
        rs_desc.pool = rs_pool;
        rs_desc.shader_count = static_cast<uint32_t>(shaders.size());
        rs_desc.shaders = ppl_shaders; 
        // TODO: static samplers & push constants
        rs_desc.push_constant_count = 1;
        const char8_t* push_const_name = u8"push_constants";
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
        // 1.make bind table
        // TODO: multi bind table
        CGPUXBindTableDescriptor table_desc = {};
        table_desc.root_signature = root_signature;
        eastl::fixed_vector<const char8_t*, 16> slot_names;
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
                        if (override.slot_name.starts_with((const char*)resource.name) 
                        && strlen((const char*)resource.name) == override.slot_name.size()) // slot name matches
                        {
                            slot_names.emplace_back(resource.name);
                        }
                    }
                }
                else if (resource.type == CGPU_RESOURCE_TYPE_TEXTURE)
                {
                    for (const auto& override : material->overrides.textures)
                    {
                        if (override.slot_name.starts_with((const char*)resource.name) 
                        && strlen((const char*)resource.name) == override.slot_name.size()) // slot name matches
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
        table_desc.names_count = (uint32_t)slot_names.size();
        table_desc.names = slot_names.data();
        const auto bind_table = cgpux_create_bind_table(root.device, &table_desc);

        // 2.update values
        eastl::fixed_vector<CGPUDescriptorData, 16> updates;
        for (const auto& override : material->overrides.samplers)
        {
            skr::resource::TResourceHandle<skr_texture_sampler_resource_t> hdl = override.value;
            hdl.resolve(true, nullptr);

            auto& update = updates.emplace_back();
            update.name = (const char8_t*)override.slot_name.data();
            update.count = 1;
            update.samplers = &hdl.get_resolved()->sampler;
            update.binding_type = CGPU_RESOURCE_TYPE_SAMPLER;
        }
        for (const auto& override : material->overrides.textures)
        {
            skr::resource::TResourceHandle<skr_texture_resource_t> hdl = override.value;
            hdl.resolve(true, nullptr);

            auto& update = updates.emplace_back();
            update.name = (const char8_t*)override.slot_name.data();
            update.count = 1; // TODO: Tex array parameter
            update.textures = &hdl.get_resolved()->texture_view;
            update.binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
        }
        cgpux_bind_table_update(bind_table, updates.data(), (uint32_t)updates.size());
        return bind_table;
    }

    CGPURootSignatureId requestRS(skr_resource_record_t* record, skr_material_resource_t::installed_pass& installed_pass, skr::span<CGPUShaderLibraryId> shaders)
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        // 0.return if ready
        if (installed_pass.root_signature) return installed_pass.root_signature; // already created

        // 1.sync version
        if (root.aux_service == nullptr)
        {
            installed_pass.root_signature = createMaterialRS(installed_pass, shaders);
            installed_pass.bind_table = createMaterialBindTable(material, installed_pass.root_signature);
            return installed_pass.root_signature;
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
                installed_pass.root_signature = rsRequest->root_signature;
                installed_pass.bind_table = rsRequest->bind_table;
                mRootSignatureRequests.erase(materialGUID);
            }
            return installed_pass.root_signature;
        }
        else
        {
            // 2.2.1 fire async create task
            auto rsRequest = SPtr<RootSignatureRequest>::Create(material, this, installed_pass, shaders);
            auto aux_service = root.aux_service;
            auto aux_task = make_zeroed<skr_service_task_t>();
            aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                ZoneScopedN("CreateRootSignature(AuxService)");
                auto rsRequest = static_cast<RootSignatureRequest*>(usrdata);
                const auto factory = rsRequest->factory;

                rsRequest->root_signature = factory->createMaterialRS(rsRequest->installed_pass, rsRequest->shaders);
                rsRequest->bind_table = factory->createMaterialBindTable(rsRequest->material, rsRequest->root_signature);
            };
            aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = rsRequest.get();
            aux_service->request(&aux_task, &rsRequest->request);
            mRootSignatureRequests.emplace(materialGUID, rsRequest);
            return nullptr;
        }
    }

    skr_pso_map_key_id makePsoMapKey(skr_material_resource_t* material, skr_material_resource_t::installed_pass& installed_pass, skr::span<CGPUShaderLibraryId> shaders) const SKR_NOEXCEPT
    {
        auto desc = make_zeroed<CGPURenderPipelineDescriptor>();
        desc.root_signature = installed_pass.root_signature;
        // 1.fill pipeline shaders
        auto vertex_shader = make_zeroed<CGPUShaderEntryDescriptor>();
        auto tesc_shader = make_zeroed<CGPUShaderEntryDescriptor>();
        auto tese_shader = make_zeroed<CGPUShaderEntryDescriptor>();
        auto geom_shader = make_zeroed<CGPUShaderEntryDescriptor>();
        auto fragment_shader = make_zeroed<CGPUShaderEntryDescriptor>();
        for (uint32_t i = 0; i < shaders.size(); i++)
        {
            CGPUShaderEntryDescriptor* ref = &vertex_shader;
            switch (installed_pass.shaders[i].stage)
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
            ref->entry = (const char8_t*)installed_pass.shaders[i].entry.data();
            ref->stage = installed_pass.shaders[i].stage;
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
        blend_state.alpha_to_coverage = false;
        blend_state.independent_blend = true;
        // TODO: MRT & Custom Blend
        const auto pass_index = &installed_pass - material->installed_passes.data();
        const auto& blend_modes = matType->passes[pass_index].blend_modes;
        for (uint32_t i = 0; i < CGPU_MAX_MRT_COUNT; i++)
        {
            blend_state.blend_modes[i] = CGPU_BLEND_MODE_ADD; 
            blend_state.blend_alpha_modes[i] = CGPU_BLEND_MODE_ADD; 
            blend_state.masks[i] = CGPU_COLOR_MASK_ALL; 
            const auto blend_mode = (blend_modes.size() > i) ? blend_modes[i] : 
                (blend_modes.size() ? blend_modes.back() : EMaterialBlendMode::Opaque);
            switch (blend_mode)
            {
                case EMaterialBlendMode::Opaque:
                {
                    blend_state.src_factors[i] = CGPU_BLEND_CONST_ONE; 
                    blend_state.dst_factors[i] = CGPU_BLEND_CONST_ZERO; 
                    blend_state.src_alpha_factors[i] = CGPU_BLEND_CONST_ONE;
                    blend_state.dst_alpha_factors[i] = CGPU_BLEND_CONST_ZERO;
                }
                break;
                case EMaterialBlendMode::Blend:
                {
                    blend_state.src_factors[i] = CGPU_BLEND_CONST_SRC_ALPHA; 
                    blend_state.dst_factors[i] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA; 
                    blend_state.src_alpha_factors[i] = CGPU_BLEND_CONST_ONE;
                    blend_state.dst_alpha_factors[i] = CGPU_BLEND_CONST_ZERO;
                }
                break;
                case EMaterialBlendMode::Mask:
                {
                    SKR_UNIMPLEMENTED_FUNCTION();
                }
                break;
            }
        }
        desc.blend_state = &blend_state;
        // 4.fill depth state 
        auto depth_state = make_zeroed<CGPUDepthStateDescriptor>();
        depth_state.depth_func = CGPU_CMP_LEQUAL; // TODO: Depth Contril
        depth_state.depth_write = true; // TODO: Depth Write Control
        depth_state.depth_test = true; // TODO: Depth Test Control
        desc.depth_state = &depth_state;
        // 5.fill raster state 
        auto raster_desc = make_zeroed<CGPURasterizerStateDescriptor>();
        raster_desc.depth_bias = 0; // TODO: Depth Bias Control
        raster_desc.fill_mode = CGPU_FILL_MODE_SOLID; // TODO: Fill Mode Control
        if (matType->passes[pass_index].two_sided)
        {
            raster_desc.cull_mode = CGPU_CULL_MODE_NONE; 
        }
        else
        {
            raster_desc.cull_mode = CGPU_CULL_MODE_BACK; // TODO: Cull Mode Control
        }
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

    CGPURenderPipelineId requestPSO(skr_resource_record_t* record, skr_material_resource_t::installed_pass& installed_pass, skr::span<CGPUShaderLibraryId> shaders, bool& fail)
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        if (!installed_pass.key)
        {
            installed_pass.key = makePsoMapKey(material, installed_pass, shaders);
            auto status = skr_pso_map_install_pso(pso_map, installed_pass.key);
            if (status == SKR_PSO_MAP_PSO_STATUS_FAILED) fail = true;
        }
        return skr_pso_map_find_pso(pso_map, installed_pass.key);
    }

    ESkrInstallStatus UpdateInstall_Pass(skr_resource_record_t* record, skr_material_resource_t::installed_pass& installed_pass)
    {
        // 1.all shaders are installed ?
        eastl::fixed_vector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
        for (const auto& identifier : installed_pass.shaders)
        {
            if (auto library = shader_map->find_shader(identifier.identifier))
            {
                shaders.emplace_back(library);
            }
            else
            {
                installed_pass.status = SKR_INSTALL_STATUS_INPROGRESS;
                return SKR_INSTALL_STATUS_INPROGRESS;
            }
        }

        // 2.make RS. CGPU has rs pools so we can just create a pooled RS here.
        // CGPU will route it to the right backend unique RootSignature.
        installed_pass.root_signature = requestRS(record, installed_pass, shaders);

        // 3.make PSO, root signature needs to be ready for the request.
        bool exception = false;
        installed_pass.pso = installed_pass.root_signature ? requestPSO(record, installed_pass, shaders, exception) : nullptr;
        if (exception) return SKR_INSTALL_STATUS_FAILED;
        return installed_pass.pso ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
    }

    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        auto material = static_cast<skr_material_resource_t*>(record->resource);
        // foreach pass check if all shaders are installed.
        bool all_okay = true;
        for (auto& installed_pass : material->installed_passes)
        {
            const auto pass_status = UpdateInstall_Pass(record, installed_pass);
            if (pass_status != SKR_INSTALL_STATUS_SUCCEED) all_okay = false;
        }
        return all_okay ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
    }

    struct RootSignatureRequest
    {
        RootSignatureRequest(const skr_material_resource_t* material, SMaterialFactoryImpl* factory, skr_material_resource_t::installed_pass& installed_pass, skr::span<CGPUShaderLibraryId> shaders)
            : material(material), installed_pass(installed_pass), factory(factory), shaders(shaders.data(), shaders.data() + shaders.size())
        {

        }
        skr_async_request_t request;
        const skr_material_resource_t* material = nullptr;
        skr_material_resource_t::installed_pass& installed_pass;
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