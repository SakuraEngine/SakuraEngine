#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/async/thread_job.hpp"
#include "SkrCore/id_range_allocator.hpp"
#include "SkrCore/memory/sp.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/graphics/shader_map.hpp"
#include "SkrRenderer/graphics/pso_map.hpp"
#include "SkrRenderer/shared/gpu_scene.hpp"

namespace skr
{
using namespace skr;
using MaterialFutureLancher = skr::FutureLauncher<bool>;

void MaterialResource::SetBoolParameterValue(skr::StringView name, bool v)
{
    overrides.bools.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueBool value = {
        .slot_name = name,
        .value = v
    };
    overrides.bools.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetDoubleParameterValue(skr::StringView name, double v)
{
    overrides.doubles.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueDouble value = {
        .slot_name = name,
        .value = v
    };
    overrides.doubles.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetFloatParameterValue(skr::StringView name, float v)
{
    overrides.floats.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueFloat value = {
        .slot_name = name,
        .value = v
    };
    overrides.floats.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetFloat2ParameterValue(skr::StringView name, float2 v)
{
    overrides.float2s.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueFloat2 value = {
        .slot_name = name,
        .value = v
    };
    overrides.float2s.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetFloat3ParameterValue(skr::StringView name, float3 v)
{
    overrides.float3s.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueFloat3 value = {
        .slot_name = name,
        .value = v
    };
    overrides.float3s.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetFloat4ParameterValue(skr::StringView name, float4 v)
{
    overrides.float4s.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueFloat4 value = {
        .slot_name = name,
        .value = v
    };
    overrides.float4s.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetTextureParameterValue(skr::StringView name, AsyncResource<TextureResource> tex)
{
    overrides.textures.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueTexture value = {
        .slot_name = name,
        .value = tex.get_guid()
    };
    overrides.textures.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::SetSamplerParameterValue(skr::StringView name, AsyncResource<TextureSamplerResource> tex)
{
    overrides.samplers.remove_all_if([&](auto v) { return v.slot_name == name; });
    MaterialValueSampler value = {
        .slot_name = name,
        .value = tex.get_guid()
    };
    overrides.samplers.add(value);
    if (factory != nullptr)
        factory->MarkMaterialDirty(this);
}

void MaterialResource::storeToGPUTable(gpu::TableInstance& table)
{
    gpu::PBRMaterial mat_data;
    mat_data.global_index = this->mat_id;
    mat_data.basecolor_tex = ~0;
    mat_data.metallic_roughness_tex = ~0;
    mat_data.emission_tex = ~0;
    mat_data.normal_tex = ~0;
    for (const auto& f : overrides.floats)
    {
        if (f.slot_name == u8"Metallic")
            mat_data.metallic = saturate(f.value);
        else if (f.slot_name == u8"Roughness")
            mat_data.roughness = saturate(f.value);
    }
    for (const auto& f3 : overrides.float3s)
    {
        if (f3.slot_name == u8"BaseColor")
            mat_data.basecolor = saturate(f3.value);
    }
    for (const auto& tex : overrides.textures)
    {
        if (tex.slot_name == u8"BaseColorTexture")
            mat_data.basecolor_tex = tex.bindless_id;
        else if (tex.slot_name == u8"MetallicRoughness")
            mat_data.metallic_roughness_tex = tex.bindless_id;
        else if (tex.slot_name == u8"Emissive")
            mat_data.emission_tex = tex.bindless_id;
        else if (tex.slot_name == u8"NormalMap")
            mat_data.normal_tex = tex.bindless_id;
    }
    gpu::GPUDatablock<gpu::PBRMaterial>::StoreInstance(table, mat_data.global_index, mat_data);
}

struct MaterialFactoryImpl : public MaterialFactory
{
    MaterialFactoryImpl(const MaterialFactoryImpl::Root& root)
        : root(root)
    {
        auto cgpu_device = root.render_device->get_cgpu_device();

        // 0.async launcher
        launcher = skr::SP<MaterialFutureLancher>::New(root.job_queue);

        // 1.create shader map
        shader_map = root.shader_map;

        // 2.create root signature pool
        CGPURootSignaturePoolDescriptor rs_pool_desc = {};
        rs_pool_desc.name = u8"MaterialRootSignaturePool";
        rs_pool = cgpu_create_root_signature_pool(cgpu_device, &rs_pool_desc);

        // 3.create pso map
        skr_pso_map_root_t pso_map_root;
        pso_map_root.job_queue = root.job_queue;
        pso_map_root.device = cgpu_device;
        pso_map = skr_pso_map_create(&pso_map_root);

        // 4.create descriptor buffer
        desc_buffer.count = 4096;
        CGPUDescriptorBufferDescriptor bdls_desc = { .count = desc_buffer.count };
        desc_buffer.descriptor_buffer = cgpu_create_descriptor_buffer(cgpu_device, &bdls_desc);

        if (auto TableManager = root.table_manager)
        {
            gpu::TableConfig table_builder(root.render_device->get_cgpu_device(), u8"Materials");
            table_builder.with_instances(16 * 1024);
            gpu::GPUDatablock<gpu::PBRMaterial>::SetupTableConfig(table_builder);
            mMaterialTable = TableManager->CreateTable(table_builder);
            mMaterialIdRangeAllocator.resize(mMaterialTable->GetInstanceCapacity());
        }
    }

    ~MaterialFactoryImpl()
    {
        if (pso_map)
            skr_pso_map_free(pso_map);

        if (desc_buffer.descriptor_buffer)
            cgpu_free_descriptor_buffer(desc_buffer.descriptor_buffer);

        if (rs_pool)
            cgpu_free_root_signature_pool(rs_pool);

        mMaterialTable.reset();
    }

    GUID GetResourceType() override
    {
        return ::skr::type_id_of<MaterialResource>();
    }

    bool AsyncIO() override { return true; }

    bool Unload_Pass(MaterialResource::InstalledPass& pass)
    {
        // 1.free PSO & map key
        if (pass.key)
        {
            pso_map->uninstall_pso(pass.key);
            pso_map->free_key(pass.key);
        }

        // 2.free RS
        if (pass.bind_table)
        {
            cgpux_free_bind_table(pass.bind_table);
            pass.bind_table = nullptr;
        }
        if (pass.root_signature)
        {
            cgpu_free_root_signature(pass.root_signature);
            pass.root_signature = nullptr;
        }

        // 3.RC free installed shaders
        for (const auto& installed_shader : pass.shaders)
        {
            shader_map->free_shader(installed_shader.identifier);
        }

        return true;
    }

    bool Unload(SResourceRecord* record) override
    {
        auto material = static_cast<MaterialResource*>(record->resource);
        bool unloaded = true;
        for (auto& pass : material->installed_passes)
        {
            unloaded &= Unload_Pass(pass);
        }
        mMaterialIdRangeAllocator.deallocate(material->mat_id);
        SkrDelete(material);
        return unloaded;
    }

    ESkrInstallStatus Install(SResourceRecord* record) override
    {
        auto material = static_cast<MaterialResource*>(record->resource);
        material->factory = this;
        if (!material->material_type.is_null())
        {
            auto matType = material->material_type.install();
            // install shaders
            for (auto& pass_template : matType->passes)
            {
                auto& installed_pass = material->installed_passes.add_default().ref();
                installed_pass.name = pass_template.pass;
                for (auto& shader : pass_template.shader_resources)
                {
                    bool installed = false;
                    const auto pShaderCollection = shader.install();
                    const auto shaderCollectionGUID = shader.get_guid();
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
                                const auto backend = root.render_device->get_backend();
                                const auto bytecode_type = ShaderResourceFactory::GetRuntimeBytecodeType(backend);
                                if (bytecode_type == platform_id.bytecode_type)
                                {
                                    const auto status = shader_map->install_shader(platform_id);
                                    if (status != EShaderMapShaderStatus::FAILED)
                                    {
                                        auto& installed_shader = installed_pass.shaders.add_default().ref();
                                        installed_shader.identifier = platform_id;
                                        installed_shader.entry = multiShader.entry.c_str();
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
        }
        createBindlessDescriptors(material);
        addToGPUTable(material);
        return material ? SKR_INSTALL_STATUS_INPROGRESS : SKR_INSTALL_STATUS_FAILED;
    }

    bool Uninstall(SResourceRecord* record) override
    {
        auto material = static_cast<MaterialResource*>(record->resource);
        mDirtyMaterials.erase(material);
        return true;
    }

    skr::RG::BufferHandle UpdateGPUTable(skr::RG::RenderGraph* graph) override
    {
        auto dirties = mDirtyMaterials;
        mDirtyMaterials.clear();
        for (auto dirty : dirties)
        {
            dirty->storeToGPUTable(*mMaterialTable);
        }

        auto handle = mMaterialTable->UpdateTableBuffer(graph, mMaterialIdRangeAllocator.getMaxIds());
        mMaterialTable->DispatchSparseUpload(graph, {});
        return handle;
    }

    void MarkMaterialDirty(MaterialResource* mat) override
    {
        mDirtyMaterials.insert(mat);
    }

    ESkrInstallStatus UpdateInstall(SResourceRecord* record) override
    {
        auto material = static_cast<MaterialResource*>(record->resource);
        // foreach pass check if all shaders are installed.
        bool all_okay = true;
        for (auto& installed_pass : material->installed_passes)
        {
            const auto pass_status = UpdateInstall_Pass(record, installed_pass);
            if (pass_status != SKR_INSTALL_STATUS_SUCCEED) all_okay = false;
        }
        return all_okay ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
    }

    void addToGPUTable(MaterialResource* material)
    {
        auto id_range = mMaterialIdRangeAllocator.allocate(1);
        if (id_range.empty())
        {
            auto neededCount = 1 + mMaterialIdRangeAllocator.getMaxIds();
            mMaterialIdRangeAllocator.resize(neededCount * 2);
            id_range = mMaterialIdRangeAllocator.allocate(1);
        }
        material->mat_id = id_range.start;
        material->storeToGPUTable(*mMaterialTable);
    }

    CGPURootSignatureId createMaterialRS(MaterialResource::InstalledPass& installed_pass, skr::Span<CGPUShaderLibraryId> shaders) const
    {
        CGPUShaderEntryDescriptor ppl_shaders[CGPU_SHADER_STAGE_COUNT];
        for (size_t i = 0; i < installed_pass.shaders.size(); i++)
        {
            ppl_shaders[i].library = shaders[i];
            ppl_shaders[i].entry = (const char8_t*)installed_pass.shaders[i].entry.data();
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
        const auto root_signature = cgpu_create_root_signature(root.render_device->get_cgpu_device(), &rs_desc);
        return root_signature;
    }

    struct
    {
        CGPUDescriptorBufferId descriptor_buffer = nullptr;
        uint32_t count = 0;
        uint32_t next = 0;
        skr::Vector<uint32_t> free_list;
    } desc_buffer;

    CGPUDescriptorBufferId descriptor_buffer() override
    {
        return desc_buffer.descriptor_buffer;
    }

    skr::RC<gpu::TableInstance> material_table() override
    {
        return mMaterialTable;
    }

    void createBindlessDescriptors(MaterialResource* material)
    {
        // 3.create bindless descriptor
        for (auto& override : material->overrides.textures)
        {
            skr::AsyncResource<TextureResource> hdl = override.value;

            uint32_t free_id = 0;
            if (!desc_buffer.free_list.is_empty())
                free_id = desc_buffer.free_list.pop_back_get();
            else
                free_id = desc_buffer.next++;
            override.bindless_id = free_id;

            auto texture = hdl.install()->texture;
            CGPUTextureViewDescriptor tv_desc = {
                .name = u8"MaterialTexture",
                .texture = texture,
                .format = texture->info->format,
                .view_usages = CGPU_TEXTURE_VIEW_USAGE_SRV,
                .aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR,
                .dims = CGPU_TEXTURE_DIMENSION_2D,
                // TODO: mipmaps
                .base_mip_level = 0,
                .mip_level_count = 1
            };
            CGPUDescriptorBufferElement elem = {};
            elem.index = free_id;
            elem.resource_type = CGPU_RESOURCE_TYPE2_TEXTURE;
            elem.texture = tv_desc;
            cgpu_update_descriptor_buffer(desc_buffer.descriptor_buffer, &elem, 1);
        }
    }

    const char* sampler_name = "color_sampler";
    CGPUXBindTableId createMaterialBindTable(const MaterialResource* material, CGPURootSignatureId root_signature)
    {
        // 1.make bind table
        // TODO: multi bind table
        CGPUXBindTableDescriptor table_desc = {};
        table_desc.root_signature = root_signature;
        skr::InlineVector<const char8_t*, 16> slot_names;
        for (uint32_t i = 0; i < root_signature->table_count; i++)
        {
            const auto& table = root_signature->tables[i];
            for (uint32_t j = 0; j < table.resources_count; j++)
            {
                const auto& resource = table.resources[j];
                if (resource.type == CGPU_RESOURCE_TYPE2_SAMPLER)
                {
                    for (const auto& override : material->overrides.samplers)
                    {
                        if (override.slot_name.starts_with(resource.name) && strlen((const char*)resource.name) == override.slot_name.size()) // slot name matches
                        {
                            slot_names.emplace(resource.name);
                        }
                    }
                }
                else if (resource.type == CGPU_RESOURCE_TYPE2_TEXTURE)
                {
                    for (const auto& override : material->overrides.textures)
                    {
                        if (override.slot_name.starts_with(resource.name) && strlen((const char*)resource.name) == override.slot_name.size()) // slot name matches
                        {
                            slot_names.emplace(resource.name);
                        }
                    }
                }
                else if (resource.type == CGPU_RESOURCE_TYPE2_BUFFER)
                {
                    // SKR_UNIMPLEMENTED_FUNCTION();
                }
            }
        }
        table_desc.names_count = (uint32_t)slot_names.size();
        table_desc.names = slot_names.data();
        const auto bind_table = cgpux_create_bind_table(root.render_device->get_cgpu_device(), &table_desc);

        // 2.update values
        skr::InlineVector<CGPUDescriptorData, 16> updates;
        for (const auto& override : material->overrides.samplers)
        {
            auto hdl = skr::AsyncResource<TextureSamplerResource>(override.value);
            auto& update = updates.emplace().ref();
            update.by_name.name = override.slot_name.data();
            update.count = 1;
            update.samplers = &hdl.install()->sampler;
        }
        for (const auto& override : material->overrides.textures)
        {
            skr::AsyncResource<TextureResource> hdl = override.value;
            auto& update = updates.emplace().ref();
            update.by_name.name = override.slot_name.data();
            update.count = 1; // TODO: Tex array parameter
            update.textures = &hdl.install()->texture_view;
        }
        cgpux_bind_table_update(bind_table, updates.data(), (uint32_t)updates.size());
        return bind_table;
    }

    CGPURootSignatureId requestRS(SResourceRecord* record, MaterialResource::InstalledPass& installed_pass, skr::Span<CGPUShaderLibraryId> shaders)
    {
        auto material = static_cast<MaterialResource*>(record->resource);
        // 0.return if ready
        if (installed_pass.root_signature) return installed_pass.root_signature; // already created

        const auto materialGUID = record->header.guid;
        auto iter = mRootSignatureRequests.find(materialGUID);
        if (iter == mRootSignatureRequests.end())
        {
            auto rsRequest = SP<RootSignatureRequest>::New(material, this, installed_pass, shaders);
            mRootSignatureRequests.emplace(materialGUID, rsRequest);
            if (auto async_launcher = launcher.get())
            {
                rsRequest->execute(*async_launcher);
            }
        }
        else if (auto drive = iter->second->on_callback_loop())
        {
            installed_pass.root_signature = iter->second->root_signature;
            installed_pass.bind_table = iter->second->bind_table;
            return iter->second->root_signature;
        }
        return nullptr;
    }

    skr_pso_map_key_id makePsoMapKey(MaterialResource* material, MaterialResource::InstalledPass& installed_pass, skr::Span<CGPUShaderLibraryId> shaders) const SKR_NOEXCEPT
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
            ref->entry = installed_pass.shaders[i].entry.data();
        }
        // 2.fill vertex layout
        auto vert_layout = make_zeroed<CGPUVertexLayout>();
        const auto matType = material->material_type.install();
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
                                                               (blend_modes.size() ? blend_modes[blend_modes.size() - 1] :
                                                                                     EMaterialBlendMode::Opaque);
            switch (blend_mode)
            {
            case EMaterialBlendMode::Opaque: {
                blend_state.src_factors[i] = CGPU_BLEND_CONST_ONE;
                blend_state.dst_factors[i] = CGPU_BLEND_CONST_ZERO;
                blend_state.src_alpha_factors[i] = CGPU_BLEND_CONST_ONE;
                blend_state.dst_alpha_factors[i] = CGPU_BLEND_CONST_ZERO;
            }
            break;
            case EMaterialBlendMode::Blend: {
                blend_state.src_factors[i] = CGPU_BLEND_CONST_SRC_ALPHA;
                blend_state.dst_factors[i] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
                blend_state.src_alpha_factors[i] = CGPU_BLEND_CONST_ONE;
                blend_state.dst_alpha_factors[i] = CGPU_BLEND_CONST_ZERO;
            }
            break;
            case EMaterialBlendMode::Count:
            case EMaterialBlendMode::Mask: {
                SKR_UNIMPLEMENTED_FUNCTION();
            }
            break;
            }
        }
        desc.blend_state = &blend_state;
        // 4.fill depth state
        auto depth_state = make_zeroed<CGPUDepthStateDescriptor>();
        depth_state.depth_func = CGPU_CMP_LEQUAL; // TODO: Depth Contril
        depth_state.depth_write = true;           // TODO: Depth Write Control
        depth_state.depth_test = true;            // TODO: Depth Test Control
        desc.depth_state = &depth_state;
        // 5.fill raster state
        auto raster_desc = make_zeroed<CGPURasterizerStateDescriptor>();
        raster_desc.depth_bias = 0;                   // TODO: Depth Bias Control
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
        desc.render_target_count = 1;                               // TODO: MRT
        desc.color_formats = &fmt;                                  // TODO: use correct screen buffer format
        desc.sample_count = CGPU_SAMPLE_COUNT_1;                    // TODO: MSAA
        desc.sample_quality = 0u;                                   // TODO: MSAA
        desc.color_resolve_disable_mask = 0u;                       // TODO: Color resolve mask (this is a vulkan-only feature)
        desc.depth_stencil_format = CGPU_FORMAT_D32_SFLOAT_S8_UINT; // TODO: depth stencil format
        desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;               // TODO: non-triangle list topology support
        desc.enable_indirect_command = false;                       // TODO: indirect command support
        return skr_pso_map_create_key(pso_map, &desc);
    }

    CGPURenderPipelineId requestPSO(SResourceRecord* record, MaterialResource::InstalledPass& installed_pass, skr::Span<CGPUShaderLibraryId> shaders, bool& fail)
    {
        auto material = static_cast<MaterialResource*>(record->resource);
        if (!installed_pass.key)
        {
            installed_pass.key = makePsoMapKey(material, installed_pass, shaders);
            auto status = skr_pso_map_install_pso(pso_map, installed_pass.key);
            if (status == SKR_PSO_MAP_PSO_STATUS_FAILED) fail = true;
        }
        return skr_pso_map_find_pso(pso_map, installed_pass.key);
    }

    ESkrInstallStatus UpdateInstall_Pass(SResourceRecord* record, MaterialResource::InstalledPass& installed_pass)
    {
        // 1.all shaders are installed ?
        skr::InlineVector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
        for (const auto& identifier : installed_pass.shaders)
        {
            if (auto library = shader_map->find_shader(identifier.identifier))
            {
                shaders.emplace(library);
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

    struct RootSignatureRequest
        : public skr::AsyncProgress<MaterialFutureLancher, int, bool>
    {
        RootSignatureRequest(const MaterialResource* material, MaterialFactoryImpl* factory, MaterialResource::InstalledPass& installed_pass, skr::Span<CGPUShaderLibraryId> shaders)
            : material(material)
            , installed_pass(installed_pass)
            , factory(factory)
            , shaders(shaders.data(), shaders.size())
        {
        }

        bool do_in_background() override
        {
            root_signature = factory->createMaterialRS(installed_pass, shaders);
            bind_table = factory->createMaterialBindTable(material, root_signature);
            return root_signature;
        }

        const MaterialResource* material = nullptr;
        MaterialResource::InstalledPass& installed_pass;
        MaterialFactoryImpl* factory = nullptr;
        CGPURootSignatureId root_signature = nullptr;
        CGPUXBindTableId bind_table = nullptr;
        skr::InlineVector<CGPUShaderLibraryId, CGPU_SHADER_STAGE_COUNT> shaders;
    };

    skr::ParallelFlatHashSet<MaterialResource*> mDirtyMaterials;
    skr::FlatHashMap<GUID, SP<RootSignatureRequest>, skr::Hash<GUID>> mRootSignatureRequests;
    skr::SP<MaterialFutureLancher> launcher = nullptr;

    skr::IdRangeAllocator mMaterialIdRangeAllocator;
    skr::RC<gpu::TableInstance> mMaterialTable;

    ShaderMap* shader_map = nullptr;
    skr_pso_map_id pso_map = nullptr;
    CGPURootSignaturePoolId rs_pool = nullptr;
    Root root;
};

MaterialFactory* MaterialFactory::Create(const Root& root)
{
    return SkrNew<MaterialFactoryImpl>(root);
}

void MaterialFactory::Destroy(MaterialFactory* factory)
{
    SkrDelete(factory);
}

} // namespace skr