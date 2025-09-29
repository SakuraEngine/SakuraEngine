#include "SkrContainersDef/set.hpp"
#include "SkrRenderGraph/phases_v2/pass_info_analysis.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrGraphics/flags.h"

namespace skr::RG {

void PassInfoAnalysis::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("PassInfoAnalysis");
    
    auto& passes = get_passes(graph);
    pass_infos.reserve(passes.size());
    resource_infos.reserve(graph->get_resources().size());
    for (PassNode* pass : passes)
    {
        extract_pass_info(pass);
    }
}

const PassInfo* PassInfoAnalysis::get_pass_info(PassNode* pass) const
{
    if (auto found = pass_infos.find(pass); found != pass_infos.end())
        return &found->second;
    return nullptr;
}

const ResourceInfo* PassInfoAnalysis::get_resource_info(ResourceNode* resource) const
{
    if (auto found = resource_infos.find(resource); found != resource_infos.end())
        return &found->second;
    return nullptr;
}

const PassResourceInfo* PassInfoAnalysis::get_resource_info(PassNode* pass) const
{
    const auto* info = get_pass_info(pass);
    return info ? &info->resource_info : nullptr;
}

const PassPerformanceInfo* PassInfoAnalysis::get_performance_info(PassNode* pass) const
{
    const auto* info = get_pass_info(pass);
    return info ? &info->performance_info : nullptr;
}

void PassInfoAnalysis::extract_pass_info(PassNode* pass)
{
    PassInfo info;
    info.pass = pass;
    info.pass_type = pass->pass_type;
    
    extract_resource_info(pass, info.resource_info);
    extract_performance_info(pass, info.performance_info);
    
    pass_infos[pass] = info;
}

void PassInfoAnalysis::extract_resource_info(PassNode* pass, PassResourceInfo& info)
{
    SkrZoneScopedN("ExtractResourceInfo");
    
    info.resource_accesses.reserve(pass->buffers_count() + pass->textures_count());
    
    // Extract textures with detailed access info
    pass->foreach_textures([&](TextureNode* texture, TextureEdge* edge) {
        ResourceAccessInfo access_info;
        access_info.pass = pass;
        access_info.resource = texture;
        switch (edge->get_type())
        {
        case ERelationshipType::TextureRead:
            access_info.access_type = EResourceAccessType::Read;
            access_info.resource_state = static_cast<TextureReadEdge*>(edge)->requested_state;
            // 子资源范围
            access_info.mip_base = static_cast<TextureReadEdge*>(edge)->get_mip_base();
            access_info.mip_count = static_cast<TextureReadEdge*>(edge)->get_mip_count();
            access_info.array_base = static_cast<TextureReadEdge*>(edge)->get_array_base();
            access_info.array_count = static_cast<TextureReadEdge*>(edge)->get_array_count();
            break;
        case ERelationshipType::TextureWrite:
            access_info.access_type = EResourceAccessType::Write;
            access_info.resource_state = static_cast<TextureRenderEdge*>(edge)->requested_state;
            // RTV 只支持单 mip
            access_info.mip_base = static_cast<TextureRenderEdge*>(edge)->get_mip_level();
            access_info.mip_count = 1;
            access_info.array_base = static_cast<TextureRenderEdge*>(edge)->get_array_base();
            access_info.array_count = static_cast<TextureRenderEdge*>(edge)->get_array_count();
            break;
        case ERelationshipType::TextureReadWrite:
            access_info.access_type = EResourceAccessType::ReadWrite;
            access_info.resource_state = static_cast<TextureReadWriteEdge*>(edge)->requested_state;
            access_info.mip_base = static_cast<TextureReadWriteEdge*>(edge)->get_mip_level();
            access_info.mip_count = 1;
            access_info.array_base = static_cast<TextureReadWriteEdge*>(edge)->get_array_base();
            access_info.array_count = static_cast<TextureReadWriteEdge*>(edge)->get_array_count();
            break;
        default:
            break;
        }
        info.resource_accesses.add(access_info);
        
        // 更新全局资源信息
        auto& global_resource_info = resource_infos[texture];
        global_resource_info.used_states.add(pass, access_info.resource_state);
        global_resource_info.memory_size = texture->get_size();
        
        // 根据 Pass 类型推断队列类型
        ECGPUQueueType queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
        if (pass->pass_type == EPassType::Compute)
            queue_type = CGPU_QUEUE_TYPE_COMPUTE;
        else if (pass->pass_type == EPassType::Copy)
            queue_type = CGPU_QUEUE_TYPE_TRANSFER;
        global_resource_info.access_queues.add(queue_type);
        info.total_resource_count += 1;
        
        return true;
    });
    
    // Extract buffers with detailed access info
    pass->foreach_buffers([&](BufferNode* buffer, BufferEdge* edge) {
        ResourceAccessInfo access_info;
        access_info.pass = pass;
        access_info.resource = buffer;
        access_info.resource_state = static_cast<BufferEdge*>(edge)->requested_state;
        
        switch (edge->get_type())
        {
        case ERelationshipType::BufferRead:
            access_info.access_type = EResourceAccessType::Read;
            {
                auto handle = static_cast<BufferReadEdge*>(edge)->get_handle();
                access_info.buffer_from = handle.from;
                access_info.buffer_to = handle.to;
            }
            break;
        case ERelationshipType::BufferReadWrite:
            access_info.access_type = EResourceAccessType::ReadWrite;
            {
                auto handle = static_cast<BufferReadWriteEdge*>(edge)->get_handle();
                access_info.buffer_from = handle.from;
                access_info.buffer_to = handle.to;
            }
            break;
        default:
            break;
        }
        
        info.resource_accesses.add(access_info);
        
        // 更新全局资源信息
        auto& global_resource_info = resource_infos[buffer];
        global_resource_info.resource = buffer;
        global_resource_info.used_states.add(pass, access_info.resource_state);
        global_resource_info.memory_size = buffer->get_desc().size;
        
        // 根据 Pass 类型推断队列类型
        ECGPUQueueType queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
        if (pass->pass_type == EPassType::Compute)
            queue_type = CGPU_QUEUE_TYPE_COMPUTE;
        else if (pass->pass_type == EPassType::Copy)
            queue_type = CGPU_QUEUE_TYPE_TRANSFER;

        global_resource_info.access_queues.add(queue_type);
        info.total_resource_count += 1;
        
        return true;
    });
}

void PassInfoAnalysis::extract_performance_info(PassNode* pass, PassPerformanceInfo& info)
{
    // Direct flag reading only
    info.is_compute_intensive = pass->has_flags(EPassFlags::ComputeIntensive);
    info.is_bandwidth_intensive = pass->has_flags(EPassFlags::BandwidthIntensive);
    info.is_vertex_bound = pass->has_flags(EPassFlags::VertexBoundIntensive);
    info.is_pixel_bound = pass->has_flags(EPassFlags::PixelBoundIntensive);
    info.has_small_working_set = pass->has_flags(EPassFlags::SmallWorkingSet);
    info.has_large_working_set = pass->has_flags(EPassFlags::LargeWorkingSet);
    info.has_random_access = pass->has_flags(EPassFlags::RandomAccess);
    info.has_streaming_access = pass->has_flags(EPassFlags::StreamingAccess);
    info.prefers_async_compute = pass->has_flags(EPassFlags::PreferAsyncCompute);
    info.separate_command_buffer = pass->has_flags(EPassFlags::SeperateFromCommandBuffer);
}

// For dependency analysis - avoid recomputation
EResourceAccessType PassInfoAnalysis::get_resource_access_type(PassNode* pass, ResourceNode* resource) const
{
    const auto* resource_info = get_resource_info(pass);
    if (!resource_info) return EResourceAccessType::Read;
    
    // Quick lookup in pre-computed access info
    for (const auto& access : resource_info->resource_accesses)
    {
        if (access.resource == resource)
        {
            return access.access_type;
        }
    }
    
    return EResourceAccessType::Read; // Default fallback
}

ECGPUResourceState PassInfoAnalysis::get_resource_state(PassNode* pass, ResourceNode* resource) const
{
    if (auto it = resource_infos.find(resource); it != resource_infos.end())
    {
        if (auto access = it->second.used_states.find(pass))
        {
            return access.value();
        }
    }
    return CGPU_RESOURCE_STATE_UNDEFINED; // Default fallback
}

} // namespace skr::RG