#include "SkrRenderGraph/phases_v2/bind_table_phase.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrCore/log.hpp"
#include "SkrProfile/profile.h"
#include "SkrGraphics/flags.h"

#define BIND_TABLE_LOG(...)

namespace skr {
namespace RG {

// Utility function to find shader resource (from old implementation)
const CGPUShaderResource* find_shader_resource(const char8_t* name, uint64_t name_hash, CGPURootSignatureId root_sig, ECGPUResourceType* type = nullptr)
{
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        for (uint32_t j = 0; j < root_sig->tables[i].resources_count; j++)
        {
            const auto& table = root_sig->tables[i];
            const auto& resource = table.resources[j];
            if (resource.name_hash == name_hash && 
                strcmp((const char*)resource.name, (const char*)name) == 0)
            {
                if (type)
                    *type = resource.type;
                return &root_sig->tables[i].resources[j];
            }
        }
    }
    return nullptr;
}

BindTablePhase::BindTablePhase(
    const PassInfoAnalysis& pass_info_analysis,
    const ResourceAllocationPhase& resource_allocation_phase)
    : pass_info_analysis_(pass_info_analysis)
    , resource_allocation_phase_(resource_allocation_phase)
{
}

BindTablePhase::~BindTablePhase()
{

}

void BindTablePhase::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("BindTablePhase");
    BIND_TABLE_LOG(u8"BindTablePhase: Starting bind table creation");
    
    // Get all passes from graph
    const auto& all_passes = get_passes(graph);
    
    for (auto* pass : all_passes)
    {
        // Get root signature based on pass type
        CGPURootSignatureId root_sig = nullptr;
        if (pass->pass_type == EPassType::Render)
        {
            auto* render_pass = static_cast<RenderPassNode*>(pass);
            root_sig = render_pass->get_root_signature();
        }
        if (pass->pass_type == EPassType::Compute)
        {
            auto* compute_pass = static_cast<ComputePassNode*>(pass);
            root_sig = compute_pass->get_root_signature();
        }
        
        // Skip passes without root signature
        if (!root_sig) 
            continue;
        
        // Create bind table for this pass
        CGPUXBindTableId bind_table = create_bind_table_for_pass(graph, *executor, pass, root_sig);
        if (!bind_table)
        {
            if ((pass->pass_type == EPassType::Render) || (pass->pass_type == EPassType::Compute))
            {
                SKR_LOG_ERROR(u8"Failed to create bind table for pass: %s", pass->get_name());
                SKR_BREAK();
            }
            continue;
        }
        
        // Store result
        PassBindTableInfo bind_info;
        bind_info.pass = pass;
        bind_info.root_signature = root_sig;
        bind_info.bind_table = bind_table;
        
        bind_table_result_.pass_bind_tables.add(pass, std::move(bind_info));
        bind_table_result_.total_bind_tables_created++;
        
        BIND_TABLE_LOG(u8"Created bind table for pass: %s", pass->get_name());
    }
    
    BIND_TABLE_LOG(u8"BindTablePhase: Created %u bind tables, %u texture views",
                  bind_table_result_.total_bind_tables_created,
                  bind_table_result_.total_texture_views_created);
}

CGPUXBindTableId BindTablePhase::create_bind_table_for_pass(RenderGraph* graph_, RenderGraphFrameExecutor& executor, PassNode* pass, CGPURootSignatureId root_sig) SKR_NOEXCEPT
{
    RenderGraphBackend* graph = static_cast<RenderGraphBackend*>(graph_);
    // Generate binding key for caching (similar to old implementation)
    skr::String bind_table_keys = u8"";
    
    // Collect all binding names and prepare descriptor updates
    skr::InlineVector<CGPUDescriptorData, 8> desc_set_updates;
    skr::InlineVector<const char8_t*, 8> bindTableValueNames;
    
    // Temporary storage for resources (released after binding)
    skr::InlineVector<CGPUBufferViewId, 8> buf_reads;
    skr::InlineVector<CGPUBufferViewId, 8> buf_uavs;
    skr::InlineVector<CGPUTextureViewId, 8> tex_reads;
    skr::InlineVector<CGPUTextureViewId, 8> tex_uavs;
    skr::InlineVector<CGPUAccelerationStructureId, 8> acceleration_structures;
    
    uint32_t texture_count = 0;
    uint32_t buffer_count = 0;
    uint32_t acceleration_structure_count = 0;
    
    auto buf_read_edges = pass->buf_read_edges();
    buf_reads.resize_zeroed(buf_read_edges.size());    
    for (uint32_t e_idx = 0; e_idx < buf_read_edges.size(); e_idx++)
    {
        auto& read_edge = buf_read_edges[e_idx];
        
        if (const auto resource = find_shader_resource(read_edge->get_name(), read_edge->name_hash, root_sig))
        {
            bind_table_keys.append(read_edge->get_name() ? read_edge->get_name() : resource->name);
            bind_table_keys.append(u8";");
            bindTableValueNames.emplace(resource->name);
            
            auto buffer_readed = read_edge->get_buffer_node();
            CGPUDescriptorData update = {};
            update.count = 1;
            update.by_name.name = resource->name;

            CGPUBufferViewDescriptor view_desc = {};
            view_desc.buffer = resource_allocation_phase_.get_resource(buffer_readed);
            view_desc.view_usages = resource->view_usages;
            view_desc.offset = read_edge->get_handle().from;
            view_desc.size = std::min(buffer_readed->get_desc().size, read_edge->get_handle().to) - read_edge->get_handle().from;
            view_desc.structure.element_stride = buffer_readed->get_view_desc().structure.element_stride;
            buf_reads[e_idx] = graph->get_buffer_view_pool().allocate(view_desc, graph->get_frame_index());
            update.buffers = &buf_reads[e_idx];
            desc_set_updates.emplace(update);
            
            buffer_count++;
        }
    }
    
    auto buf_write_edges = pass->buf_readwrite_edges();
    buf_uavs.resize_zeroed(buf_write_edges.size());    
    for (uint32_t e_idx = 0; e_idx < buf_write_edges.size(); e_idx++)
    {
        auto& rw_edge = buf_write_edges[e_idx];
        
        if (const auto resource = find_shader_resource(rw_edge->get_name(), rw_edge->name_hash, root_sig))
        {
            bind_table_keys.append(rw_edge->get_name() ? rw_edge->get_name() : resource->name);
            bind_table_keys.append(u8";");
            bindTableValueNames.emplace(resource->name);
            
            auto buffer_writed = rw_edge->get_buffer_node();
            CGPUDescriptorData update = {};
            update.count = 1;
            update.by_name.name = resource->name;

            CGPUBufferViewDescriptor view_desc = {};
            view_desc.buffer = resource_allocation_phase_.get_resource(buffer_writed);
            view_desc.view_usages = resource->view_usages;
            view_desc.offset = rw_edge->get_handle().from;
            view_desc.size = std::min(buffer_writed->get_desc().size, rw_edge->get_handle().to) - rw_edge->get_handle().from;
            view_desc.structure.element_stride = buffer_writed->get_view_desc().structure.element_stride;
            buf_uavs[e_idx] = graph->get_buffer_view_pool().allocate(view_desc, graph->get_frame_index());
            update.buffers = &buf_uavs[e_idx];
            desc_set_updates.emplace(update);
            
            buffer_count++;
        }
    }
    
    // Process texture SRV resources
    auto tex_read_edges = pass->tex_read_edges();
    tex_reads.resize_zeroed(tex_read_edges.size());
    for (uint32_t e_idx = 0; e_idx < tex_read_edges.size(); e_idx++)
    {
        auto& read_edge = tex_read_edges[e_idx];
        
        if (const auto resource = find_shader_resource(read_edge->get_name(), read_edge->name_hash, root_sig))
        {
            bind_table_keys.append(read_edge->get_name() ? read_edge->get_name() : resource->name);
            bind_table_keys.append(u8";");
            bindTableValueNames.emplace(resource->name);
            
            auto texture_readed = read_edge->get_texture_node();
            CGPUDescriptorData update = {};
            update.count = 1;
            update.by_name.name = resource->name;
            
            // Create texture view
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = resource_allocation_phase_.get_resource(texture_readed);
            view_desc.base_array_layer = read_edge->get_array_base();
            view_desc.array_layer_count = read_edge->get_array_count();
            view_desc.base_mip_level = read_edge->get_mip_base();
            view_desc.mip_level_count = read_edge->get_mip_count();
            view_desc.format = view_desc.texture->info->format;
            view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_SRV;
            view_desc.dims = resource->dim;
            if (view_desc.dims == CGPU_TEXTURE_DIMENSION_3D) 
            {
                view_desc.base_array_layer = 0;
                view_desc.array_layer_count = texture_readed->get_desc().depth;
            }
            
            const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(view_desc.format);
            const bool is_depth_only = FormatUtil_IsDepthOnlyFormat(view_desc.format);
            view_desc.aspects = is_depth_stencil ?
                (is_depth_only ? CGPU_TEXTURE_VIEW_ASPECTS_DEPTH : CGPU_TEXTURE_VIEW_ASPECTS_DEPTH | CGPU_TEXTURE_VIEW_ASPECTS_STENCIL) :
                CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
            
            tex_reads[e_idx] = graph->get_texture_view_pool().allocate(view_desc, graph->get_frame_index());
            update.textures = &tex_reads[e_idx];
            desc_set_updates.emplace(update);
            
            texture_count++;
            bind_table_result_.total_texture_views_created++;
        }

    }
    
    // Process texture UAV resources
    auto tex_rw_edges = pass->tex_readwrite_edges();
    tex_uavs.resize_zeroed(tex_rw_edges.size());
    for (uint32_t e_idx = 0; e_idx < tex_rw_edges.size(); e_idx++)
    {
        auto& rw_edge = tex_rw_edges[e_idx];
        
        if (const auto resource = find_shader_resource(rw_edge->get_name(), rw_edge->name_hash, root_sig))
        {
            bind_table_keys.append(rw_edge->get_name() ? rw_edge->get_name() : resource->name);
            bind_table_keys.append(u8";");
            bindTableValueNames.emplace(resource->name);
            
            auto texture_readwrite = rw_edge->get_texture_node();
            CGPUDescriptorData update = {};
            update.count = 1;
            update.by_name.name = resource->name;
            
            // Create UAV texture view
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = resource_allocation_phase_.get_resource(texture_readwrite);
            view_desc.base_array_layer = rw_edge->get_array_base();
            view_desc.array_layer_count = rw_edge->get_array_count();
            view_desc.base_mip_level = rw_edge->get_mip_level();
            view_desc.mip_level_count = 1;
            view_desc.aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
            view_desc.format = view_desc.texture->info->format;
            view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_UAV;
            view_desc.dims = resource->dim;
            if (view_desc.dims == CGPU_TEXTURE_DIMENSION_3D) 
            {
                view_desc.base_array_layer = 0;
                view_desc.array_layer_count = texture_readwrite->get_desc().depth;
            }

            tex_uavs[e_idx] = graph->get_texture_view_pool().allocate(view_desc, graph->get_frame_index());
            update.textures = &tex_uavs[e_idx];
            desc_set_updates.emplace(update);
            
            texture_count++;
            bind_table_result_.total_texture_views_created++;
        }
    }
    
    // Process acceleration structure SRV resources
    auto as_read_edges = pass->acceleration_structure_read_edges();
    acceleration_structures.resize_zeroed(as_read_edges.size());
    for (uint32_t e_idx = 0; e_idx < as_read_edges.size(); e_idx++)
    {
        auto& read_edge = as_read_edges[e_idx];
        
        if (const auto resource = find_shader_resource(read_edge->get_name(), read_edge->name_hash, root_sig))
        {
            bind_table_keys.append(read_edge->get_name() ? read_edge->get_name() : resource->name);
            bind_table_keys.append(u8";");
            bindTableValueNames.emplace(resource->name);
            
            auto acceleration_structure_readed = read_edge->get_acceleration_structure_node();
            CGPUDescriptorData update = {};
            update.count = 1;
            update.by_name.name = resource->name;
            acceleration_structures[e_idx] = acceleration_structure_readed->get_imported();
            update.acceleration_structures = &acceleration_structures[e_idx];
            desc_set_updates.emplace(update);
            
            acceleration_structure_count++;
        }
    }
    
    // Get bind table from executor's pool (similar to old implementation)
    auto&& table_pool_iter = executor.bind_table_pools.find(root_sig);
    if (table_pool_iter == executor.bind_table_pools.end())
    {
        executor.bind_table_pools.emplace(root_sig, SkrNew<BindTablePool>(root_sig));
    }
    
    auto bind_table = executor.bind_table_pools[root_sig]->pop(bind_table_keys.c_str(), 
                                                              bindTableValueNames.data(), 
                                                              (uint32_t)bindTableValueNames.size());
    
    // Update bind table with descriptors
    cgpux_bind_table_update(bind_table, desc_set_updates.data(), (uint32_t)desc_set_updates.size());
    
    // Update statistics in result
    auto& bind_info = bind_table_result_.pass_bind_tables.try_add_default(pass).value();
    bind_info.bound_texture_count = texture_count;
    bind_info.bound_buffer_count = buffer_count;
    bind_info.bound_acceleration_structure_count = acceleration_structure_count;
    
    return bind_table;
}

const PassBindTableInfo* BindTablePhase::get_pass_bind_table_info(PassNode* pass) const
{
    auto it = bind_table_result_.pass_bind_tables.find(pass);
    return it ? &it.value() : nullptr;
}

CGPUXBindTableId BindTablePhase::get_pass_bind_table(PassNode* pass) const
{
    const auto* info = get_pass_bind_table_info(pass);
    return info ? info->bind_table : nullptr;
}

} // namespace RG
} // namespace skr