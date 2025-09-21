#pragma once
#include "SkrContainers/hashmap.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "pass_info_analysis.hpp"

namespace skr
{
namespace render_graph
{

class PassNode;
class ResourceNode;
struct CrossQueueSyncPoint;
class QueueSchedule;

// Single resource dependency info
struct ResourceDependency {
    PassNode* dependent_pass;                // The depended pass
    ResourceNode* resource;                  // The involved resource
    EResourceAccessType current_access;      // Current pass access type
    EResourceAccessType previous_access;     // Previous pass access type
    ECGPUResourceState current_state;        // Current pass expected resource state
    ECGPUResourceState previous_state;       // Previous pass resource state
};

// 逻辑拓扑分析结果
struct LogicalTopologyResult
{
    struct DependencyLevel
    {
        uint32_t level = 0;
        StackVector<PassNode*> passes;  // 该级别中的所有Pass
        
        // 统计信息
        uint32_t total_resources_accessed = 0;  // 该级别访问的资源总数
        uint32_t cross_level_dependencies = 0;  // 跨级别依赖数
    };
    
    // === 逻辑拓扑信息（基于依赖关系，永不变） ===
    StackVector<DependencyLevel> logical_levels;         // 按逻辑依赖级别分组的Pass
    StackVector<PassNode*> logical_topological_order;    // 逻辑拓扑排序后的Pass列表
    StackVector<PassNode*> logical_critical_path;        // 逻辑关键路径（基于依赖关系）
    
    // 统计信息
    uint32_t max_logical_dependency_depth = 0;           // 最大逻辑依赖深度
};

// Pass dependencies result (逻辑依赖信息)
struct PassDependencies {
    // === 逻辑依赖信息（永不变，不受重排序影响） ===
    StackVector<ResourceDependency> resource_dependencies; // All resource dependencies of this pass
    StackVector<PassNode*> dependent_passes;    // Pass-level dependencies (extracted from resource dependencies)
    StackVector<PassNode*> dependent_by_passes; // Pass-level dependents
    
    // === 逻辑拓扑信息（基于依赖关系，一次计算永不变） ===
    uint32_t logical_dependency_level = 0;       // 逻辑依赖级别（最长路径深度）
    uint32_t logical_topological_order = 0;      // 逻辑拓扑排序索引
    uint32_t logical_critical_path_length = 0;   // 逻辑关键路径长度（依赖链深度）

    // Query interface
    bool has_dependency_on(PassNode* pass) const;
};

// Forward declaration
class PassInfoAnalysis;

// Pass dependency analysis phase - 逻辑依赖分析+拓扑排序
class SKR_RENDER_GRAPH_API PassDependencyAnalysis : public IRenderGraphPhase
{
public:
    PassDependencyAnalysis(const PassInfoAnalysis& pass_info_analysis);
    ~PassDependencyAnalysis() override = default;

    // IRenderGraphPhase 接口
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    // Query interface - 供后续Phase使用
    const PassDependencies* get_pass_dependencies(PassNode* pass) const;
    bool has_dependencies(PassNode* pass) const;

    // For QueueSchedule - get pass-level dependencies directly
    const StackVector<PassNode*>& get_dependent_passes(PassNode* pass) const;
    const StackVector<PassNode*>& get_dependent_by_passes(PassNode* pass) const;
    
    // 逻辑拓扑查询 (NEW)
    uint32_t get_dependency_level(PassNode* pass) const;
    uint32_t get_topological_order(PassNode* pass) const;
    uint32_t get_logical_critical_path_length(PassNode* pass) const;
    const LogicalTopologyResult& get_logical_topology_result() const { return logical_topology_; }
    const StackVector<PassNode*>& get_topological_order() const { return logical_topology_.logical_topological_order; }
    const StackVector<PassNode*>& get_logical_critical_path() const { return logical_topology_.logical_critical_path; }
    
    // 逻辑并行性查询
    bool can_execute_in_parallel_logically(PassNode* pass1, PassNode* pass2) const;
    
    // 跨队列同步点生成 (NEW)
    void generate_cross_queue_sync_points(const QueueSchedule& queue_schedule, StackVector<CrossQueueSyncPoint>& sync_points) const;

    // Debug output
    void dump_dependencies() const;
    void dump_logical_topology() const;
    void dump_logical_critical_path() const;

private:
    // Analysis result: Pass -> its dependency info
    StackMap<PassNode*, PassDependencies> pass_dependencies_;

    // Working data for pass dependencies
    struct LastResourceAccess {
        PassNode* last_pass = nullptr;
        EResourceAccessType last_access_type = EResourceAccessType::Read;
        ECGPUResourceState last_state = CGPU_RESOURCE_STATE_UNDEFINED;
    };

    // Logical topology cache
    StackMap<PassNode*, uint32_t> in_degrees_;
    StackVector<PassNode*> topo_queue_;
    StackVector<uint32_t> topo_levels_;
    
    // 逻辑拓扑分析结果 (NEW)
    LogicalTopologyResult logical_topology_;

    // Reference to pass info analysis (to avoid recomputation)
    const PassInfoAnalysis& pass_info_analysis;

    // 依赖分析方法
    void analyze_pass_dependencies(RenderGraph* graph);
    
    // 逻辑拓扑分析方法 (NEW)
    void perform_logical_topological_sort_optimized(); // 优化版本：合并拓扑排序和级别计算
    void identify_logical_critical_path();
    
    // 旧版本方法（保留以备需要）
    void perform_logical_topological_sort();
    void calculate_logical_dependency_levels();
};

} // namespace render_graph
} // namespace skr