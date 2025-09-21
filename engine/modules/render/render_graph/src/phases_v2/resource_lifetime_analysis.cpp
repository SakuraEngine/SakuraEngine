#include "SkrRenderGraph/phases_v2/resource_lifetime_analysis.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrCore/log.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

ResourceLifetimeAnalysis::ResourceLifetimeAnalysis(
    const PassInfoAnalysis& pass_info_analysis,
    const PassDependencyAnalysis& dependency_analysis,
    const QueueSchedule& queue_schedule)
    : dependency_analysis_(dependency_analysis)
    , pass_info_analysis_(pass_info_analysis)
    , queue_schedule_(queue_schedule)
{
}

void ResourceLifetimeAnalysis::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResourceLifetimeAnalysis");
    
    analyze_resource_lifetimes(graph);
}

void ResourceLifetimeAnalysis::analyze_resource_lifetimes(RenderGraph* graph) SKR_NOEXCEPT
{
    const auto& resources = get_resources(graph);
    const auto& schedule_result = queue_schedule_.get_schedule_result();
    
    for (ResourceNode* resource : resources)
    {
        if (resource->is_imported())
            continue;
        
        ResourceLifetime& lifetime = lifetime_result_.resource_lifetimes.try_add_default(resource).value();
        lifetime.resource = resource;
        lifetime.start_dependency_level = UINT32_MAX;
        lifetime.end_dependency_level = 0;
        lifetime.primary_queue = 0;
        
        // 找到所有使用该资源的Pass
        const auto& passes = get_passes(graph);
        for (PassNode* pass : passes)
        {
            const auto* pass_info = pass_info_analysis_.get_pass_info(pass);
            if (!pass_info) continue;
            
            // 检查该Pass是否使用了这个资源
            bool uses_resource = false;
            for (const auto& access : pass_info->resource_info.resource_accesses)
            {
                if (access.resource == resource)
                {
                    uses_resource = true;
                    break;
                }
            }
            
            if (uses_resource)
            {
                uint32_t dependency_level = dependency_analysis_.get_dependency_level(pass);
                // 更新生命周期范围
                if (dependency_level < lifetime.start_dependency_level)
                {
                    lifetime.start_dependency_level = dependency_level;
                    lifetime.first_using_pass = pass; // 记录第一个使用该资源的Pass
                    lifetime.first_using_state = pass_info->resource_info.resource_accesses.find_if(
                        [resource](auto info) { return info.resource == resource; }
                    ).ref().resource_state;
                    
                    if (lifetime.last_using_pass == nullptr)
                    {
                        lifetime.last_using_pass = lifetime.first_using_pass;
                        lifetime.last_using_state = lifetime.first_using_state;
                    }
                }
                if (dependency_level > lifetime.end_dependency_level)
                {
                    lifetime.end_dependency_level = dependency_level;
                    lifetime.last_using_pass = pass; // 记录最后一个使用该资源的Pass
                    lifetime.last_using_state = pass_info->resource_info.resource_accesses.find_if(
                        [resource](auto info) { return info.resource == resource; }
                    ).ref().resource_state;
                }
            }
        }
        
        // 计算主要队列（第一次使用该资源的Pass所在的队列）
        if (lifetime.first_using_pass)
        {
            if (auto queue_it = schedule_result.pass_queue_assignments.find(lifetime.first_using_pass))
            {
                lifetime.primary_queue = queue_it.value();
            }
        }
        
        // 如果没有找到使用该资源的Pass，设置默认值
        if (lifetime.start_dependency_level == UINT32_MAX)
        {
            lifetime.start_dependency_level = 0;
        }
    }
    
    // 为MemoryAliasingPhase构建按大小降序排列的资源列表
    // 收集所有有生命周期信息的资源
    for (const auto& [resource, lifetime] : lifetime_result_.resource_lifetimes)
    {
        const auto* resource_info = pass_info_analysis_.get_resource_info(resource);
        if (resource_info && resource_info->memory_size > 0)
        {
            lifetime_result_.resources_by_size_desc.add(resource);
        }
    }
    
    // 按内存大小降序排序
    std::sort(lifetime_result_.resources_by_size_desc.begin(), 
              lifetime_result_.resources_by_size_desc.end(),
              [this](ResourceNode* a, ResourceNode* b) {
                  const auto* info_a = pass_info_analysis_.get_resource_info(a);
                  const auto* info_b = pass_info_analysis_.get_resource_info(b);
                  return info_a->memory_size > info_b->memory_size;
              });
}

const ResourceLifetime* ResourceLifetimeAnalysis::get_resource_lifetime(ResourceNode* resource) const
{
    if (auto it = lifetime_result_.resource_lifetimes.find(resource))
        return &it.value();
    return nullptr;
}

bool ResourceLifetime::conflicts_with(const ResourceLifetime& other) const
{
    // 两个资源生命周期冲突，当且仅当它们的依赖级别范围有重叠
    return !(end_dependency_level < other.start_dependency_level || 
             other.end_dependency_level < start_dependency_level);
}

} // namespace render_graph
} // namespace skr