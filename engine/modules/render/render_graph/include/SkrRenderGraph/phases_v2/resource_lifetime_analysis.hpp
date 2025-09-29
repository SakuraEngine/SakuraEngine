#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "pass_dependency_analysis.hpp"
#include "pass_info_analysis.hpp"
#include "queue_schedule.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/hashmap.hpp"

namespace skr {
namespace RG {

// 资源生命周期信息
struct ResourceLifetime
{
    ResourceNode* resource;
    uint32_t start_dependency_level;    // 首次使用的依赖级别
    uint32_t end_dependency_level;      // 最后使用的依赖级别
    uint32_t primary_queue;             // 主要使用的队列
    PassNode* first_using_pass;         // 首次使用该资源的Pass
    PassNode* last_using_pass;          // 最后使用该资源的Pass
    ECGPUResourceState first_using_state = CGPU_RESOURCE_STATE_UNDEFINED; // 首次使用时的状态
    ECGPUResourceState last_using_state = CGPU_RESOURCE_STATE_UNDEFINED;  // 最后使用时的状态
    
    // 判断两个资源生命周期是否冲突（用于别名化）
    bool conflicts_with(const ResourceLifetime& other) const;
    
    // 获取生命周期长度（用于别名化排序）
    uint32_t get_lifetime_span() const { return end_dependency_level - start_dependency_level + 1; }
};

// 资源生命周期分析结果
struct ResourceLifetimeResult
{
    // 按资源分组的生命周期信息
    StackMap<ResourceNode*, ResourceLifetime> resource_lifetimes;
    
    // 按大小降序排列的资源（用于内存别名化）
    StackVector<ResourceNode*> resources_by_size_desc;
    
    // 统计信息
    uint64_t total_memory_requirement;
    uint32_t max_concurrent_resources;
    uint32_t total_resource_count;
};

// 资源生命周期分析Phase - 极简版本
class SKR_RENDER_GRAPH_API ResourceLifetimeAnalysis : public IRenderGraphPhase
{
public:
    ResourceLifetimeAnalysis(
        const PassInfoAnalysis& pass_info_analysis,
        const PassDependencyAnalysis& dependency_analysis,
        const QueueSchedule& queue_schedule);
    ~ResourceLifetimeAnalysis() override = default;

    // IRenderGraphPhase 接口
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    // 查询接口
    const ResourceLifetimeResult& get_result() const { return lifetime_result_; }
    const ResourceLifetime* get_resource_lifetime(ResourceNode* resource) const;

private:
    // 核心分析方法
    void analyze_resource_lifetimes(RenderGraph* graph) SKR_NOEXCEPT;

private:
    // 输入Phase引用
    const PassDependencyAnalysis& dependency_analysis_;
    const PassInfoAnalysis& pass_info_analysis_;
    const QueueSchedule& queue_schedule_;
    
    // 分析结果
    ResourceLifetimeResult lifetime_result_;
};

} // namespace RG
} // namespace skr