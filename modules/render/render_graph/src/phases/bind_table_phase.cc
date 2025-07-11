#include "SkrRenderGraph/phases/bind_table_phase.hpp"
#include "SkrRenderGraph/phases/resource_allocation_phase.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrContainers/string.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

// Utility function to find shader resource
const CGPUShaderResource* find_shader_resource(uint64_t name_hash, CGPURootSignatureId root_sig, ECGPUResourceType* type = nullptr)
{
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        for (uint32_t j = 0; j < root_sig->tables[i].resources_count; j++)
        {
            const auto& resource = root_sig->tables[i].resources[j];
            if (resource.name_hash == name_hash)
            {
                if (type) *type = resource.type;
                return &root_sig->tables[i].resources[j];
            }
        }
    }
    return nullptr;
}

void BindTablePhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{
    find_resource_allocation_phase(graph);
}

void BindTablePhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    // Bind table 操作在 Pass 执行时按需进行
}

void BindTablePhase::find_resource_allocation_phase(RenderGraph* graph) SKR_NOEXCEPT
{
    auto backend = static_cast<RenderGraphBackend*>(graph);
    for (auto& phase : backend->phases)
    {
        if (auto alloc_phase = skr::dynamic_pointer_cast<ResourceAllocationPhase>(phase))
        {
            resource_allocation_phase = alloc_phase.get();
            break;
        }
    }
}

CGPUXBindTableId BindTablePhase::alloc_update_pass_bind_table(RenderGraph* graph, RenderGraphFrameExecutor& executor, 
                                                             PassNode* pass, CGPURootSignatureId root_sig) SKR_NOEXCEPT
{
    if (!root_sig || !resource_allocation_phase) return nullptr;

    SkrZoneScopedN("UpdateBindings");
    
    auto backend = static_cast<RenderGraphBackend*>(graph);
    auto tex_read_edges = pass->tex_read_edges();
    auto tex_rw_edges = pass->tex_readwrite_edges();
    auto buf_read_edges = pass->buf_read_edges();
    auto buf_rw_edges = pass->buf_readwrite_edges();
    (void)buf_rw_edges;

    // Allocate or get descriptor set heap
    auto&& table_pool_iter = executor.bind_table_pools.find(root_sig);
    if (table_pool_iter == executor.bind_table_pools.end())
    {
        executor.bind_table_pools.emplace(root_sig, SkrNew<BindTablePool>(root_sig));
    }
    
    skr::String bind_table_keys = u8"";
    // Bind resources
    skr::InlineVector<CGPUDescriptorData, 8> desc_set_updates;
    skr::InlineVector<const char8_t*, 8> bindTableValueNames = {};
    // CBV Buffers
    skr::InlineVector<CGPUBufferId, 8> cbvs(buf_read_edges.size());
    skr::InlineVector<CGPUTextureViewId, 8> srvs(tex_read_edges.size());
    skr::InlineVector<CGPUTextureViewId, 8> uavs(tex_rw_edges.size());
    
    {
        for (uint32_t e_idx = 0; e_idx < buf_read_edges.size(); e_idx++)
        {
            auto& read_edge = buf_read_edges[e_idx];
            SKR_ASSERT(!read_edge->name.is_empty());
            // TODO: refactor this
            const auto& resource = *find_shader_resource(read_edge->name_hash, root_sig);

            ECGPUResourceType resource_type = resource.type;
            {
                bind_table_keys.append(read_edge->name.is_empty() ? resource.name : (const char8_t*)read_edge->name.c_str());
                bind_table_keys.append(u8";");
                bindTableValueNames.emplace(resource.name);

                auto buffer_readed = read_edge->get_buffer_node();
                CGPUDescriptorData update = {};
                update.count = 1;
                update.name = resource.name;
                update.binding_type = resource_type;
                update.binding = resource.binding;
                cbvs[e_idx] = resource_allocation_phase->resolve_buffer(executor, *buffer_readed);
                update.buffers = &cbvs[e_idx];
                desc_set_updates.emplace(update);
            }
        }
        
        // SRVs
        for (uint32_t e_idx = 0; e_idx < tex_read_edges.size(); e_idx++)
        {
            auto& read_edge = tex_read_edges[e_idx];
            SKR_ASSERT(!read_edge->name.is_empty());
            const auto& resource = *find_shader_resource(read_edge->name_hash, root_sig);

            {
                bind_table_keys.append(read_edge->name.is_empty() ? resource.name : (const char8_t*)read_edge->name.c_str());
                bind_table_keys.append(u8";");
                bindTableValueNames.emplace(resource.name);

                auto texture_readed = read_edge->get_texture_node();
                CGPUDescriptorData update = {};
                update.count = 1;
                update.name = resource.name;
                update.binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
                update.binding = resource.binding;
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = resource_allocation_phase->resolve_texture(executor, *texture_readed);
                view_desc.base_array_layer = read_edge->get_array_base();
                view_desc.array_layer_count = read_edge->get_array_count();
                view_desc.base_mip_level = read_edge->get_mip_base();
                view_desc.mip_level_count = read_edge->get_mip_count();
                view_desc.format = view_desc.texture->info->format;
                const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(view_desc.format);
                const bool is_depth_only = FormatUtil_IsDepthStencilFormat(view_desc.format);
                view_desc.aspects =
                is_depth_stencil ?
                is_depth_only ? CGPU_TVA_DEPTH : CGPU_TVA_DEPTH | CGPU_TVA_STENCIL :
                CGPU_TVA_COLOR;
                view_desc.usages = CGPU_TVU_SRV;
                view_desc.dims = read_edge->get_dimension();
                srvs[e_idx] = backend->texture_view_pool.allocate(view_desc, backend->frame_index);
                update.textures = &srvs[e_idx];
                desc_set_updates.emplace(update);
            }
        }
        
        // UAVs
        for (uint32_t e_idx = 0; e_idx < tex_rw_edges.size(); e_idx++)
        {
            auto& rw_edge = tex_rw_edges[e_idx];
            SKR_ASSERT(!rw_edge->name.is_empty());
            const auto& resource = *find_shader_resource(rw_edge->name_hash, root_sig);

            {
                bind_table_keys.append(rw_edge->name.is_empty() ? resource.name : (const char8_t*)rw_edge->name.c_str());
                bind_table_keys.append(u8";");
                bindTableValueNames.emplace(resource.name);

                auto texture_readwrite = rw_edge->get_texture_node();
                CGPUDescriptorData update = {};
                update.count = 1;
                update.name = resource.name;
                update.binding_type = CGPU_RESOURCE_TYPE_RW_TEXTURE;
                update.binding = resource.binding;
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = resource_allocation_phase->resolve_texture(executor, *texture_readwrite);
                view_desc.base_array_layer = 0;
                view_desc.array_layer_count = 1;
                view_desc.base_mip_level = 0;
                view_desc.mip_level_count = 1;
                view_desc.aspects = CGPU_TVA_COLOR;
                view_desc.format = view_desc.texture->info->format;
                view_desc.usages = CGPU_TVU_UAV;
                view_desc.dims = CGPU_TEX_DIMENSION_2D;
                uavs[e_idx] = backend->texture_view_pool.allocate(view_desc, backend->frame_index);
                update.textures = &uavs[e_idx];
                desc_set_updates.emplace(update);
            }
        }
    }
    
    auto bind_table = executor.bind_table_pools[root_sig]->pop(bind_table_keys.c_str(), 
                                                              bindTableValueNames.data(), 
                                                              (uint32_t)bindTableValueNames.size());
    cgpux_bind_table_update(bind_table, desc_set_updates.data(), (uint32_t)desc_set_updates.size());
    return bind_table;
}

} // namespace render_graph
} // namespace skr