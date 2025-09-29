#include "SkrContainersDef/set.hpp"
#include "SkrRenderGraph/phases_v2/pass_dependency_analysis.hpp"
#include "SkrRenderGraph/phases_v2/pass_info_analysis.hpp"
#include "SkrRenderGraph/phases_v2/queue_schedule.hpp"
#include "SkrRenderGraph/phases_v2/cross_queue_sync_analysis.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrCore/log.hpp"

namespace skr
{
namespace RG
{

PassDependencyAnalysis::PassDependencyAnalysis(const PassInfoAnalysis& pass_info_analysis)
    : pass_info_analysis(pass_info_analysis)
{
    in_degrees_.reserve(64);
    topo_queue_.reserve(64);
    topo_levels_.reserve(64);
}

bool PassDependencies::has_dependency_on(PassNode* pass) const
{
    for (const auto& dep : resource_dependencies)
    {
        if (dep.dependent_pass == pass)
            return true;
    }
    return false;
}

// IRenderGraphPhase 接口实现
void PassDependencyAnalysis::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("PassDependencyAnalysis");

    analyze_pass_dependencies(graph);
    perform_logical_topological_sort_optimized();
    identify_logical_critical_path();
}

void PassDependencyAnalysis::analyze_pass_dependencies(RenderGraph* graph)
{
    SkrZoneScopedN("AnalyzePassDependencies");

    // 优化版本：O(n) 复杂度，为每个资源维护最后访问者索引
    auto& all_passes = get_passes(graph);

    // 为每个资源维护最后访问的Pass和访问信息
    StackHashMap<ResourceNode*, LastResourceAccess> resource_last_access_;
    resource_last_access_.reserve(graph->get_resources().size()); // 预分配避免rehash
    for (PassNode* current_pass : all_passes)
    {
        PassDependencies& current_deps = pass_dependencies_.try_add_default(current_pass).value();

        // Get pre-computed resource access info from PassInfoAnalysis
        const auto* current_resource_info = pass_info_analysis.get_resource_info(current_pass);
        if (!current_resource_info)
            continue;

        // For each resource, check last accessor directly
        for (const auto& current_access : current_resource_info->resource_accesses)
        {
            auto resource = current_access.resource;
            auto& last_access = resource_last_access_[resource];

            // If this resource was accessed before, create dependency
            if (last_access.last_pass != nullptr && last_access.last_pass != current_pass)
            {
                ResourceDependency dep;
                dep.dependent_pass = last_access.last_pass;
                dep.resource = resource;
                dep.current_access = current_access.access_type;
                dep.previous_access = last_access.last_access_type;
                dep.current_state = current_access.resource_state;
                dep.previous_state = last_access.last_state;
                current_deps.resource_dependencies.add(dep);

                current_deps.dependent_passes.add_unique(dep.dependent_pass);
            }

            // Update last access info for this resource
            last_access.last_pass = current_pass;
            last_access.last_access_type = current_access.access_type;
            last_access.last_state = current_access.resource_state;
        }
    }

    // Build dependent_by_passes (reverse dependencies)
    {
        SkrZoneScopedN("DoInsert");
        for (auto& [pass, deps] : pass_dependencies_)
        {
            for (PassNode* dep_pass : deps.dependent_passes)
            {
                pass_dependencies_.try_add_default(dep_pass).value().dependent_by_passes.add(pass);
            }
        }
    }
}

void PassDependencyAnalysis::perform_logical_topological_sort_optimized()
{
    SkrZoneScopedN("PerformLogicalTopologicalSortOptimized");

    const size_t num_passes = pass_dependencies_.size();
    if (num_passes == 0) return;

    // 预分配容器，避免动态扩容
    in_degrees_.reserve(num_passes);
    topo_queue_.reserve(num_passes);
    topo_levels_.reserve(num_passes);
    logical_topology_.logical_topological_order.reserve(num_passes);
    logical_topology_.logical_levels.reserve(num_passes);

    // 第一步：计算所有节点的入度，同时初始化依赖级别为0
    for (auto& [pass, deps] : pass_dependencies_)
    {
        in_degrees_.add(pass, static_cast<uint32_t>(deps.dependent_passes.size()));
        deps.logical_dependency_level = 0;           // 初始化级别
        deps.logical_topological_order = UINT32_MAX; // 临时标记为未处理
    }

    // 第二步：找到所有入度为0的节点（没有依赖的节点）
    for (const auto& [pass, degree] : in_degrees_)
    {
        if (degree == 0)
        {
            topo_queue_.add(pass);
            topo_levels_.add(0); // 起始节点的级别为0
        }
    }

    // 第三步：Kahn算法 + 同时计算依赖级别
    size_t queue_idx = 0;
    while (queue_idx < topo_queue_.size())
    {
        PassNode* current = topo_queue_[queue_idx];
        uint32_t current_level = topo_levels_[queue_idx];
        ++queue_idx;

        // 添加到拓扑排序结果
        logical_topology_.logical_topological_order.add(current);

        // 设置当前节点的拓扑索引和依赖级别
        if (auto current_it = pass_dependencies_.find(current))
        {
            current_it.value().logical_topological_order = static_cast<uint32_t>(logical_topology_.logical_topological_order.size() - 1);
            current_it.value().logical_dependency_level = current_level;

            // 处理所有依赖于当前节点的节点
            for (auto* dependent : current_it.value().dependent_by_passes)
            {
                // 减少入度
                --in_degrees_.find(dependent).value();

                // 更新依赖节点的级别（取所有前驱的最大级别+1）
                if (auto dep_it = pass_dependencies_.find(dependent))
                {
                    dep_it.value().logical_dependency_level = std::max(
                        dep_it.value().logical_dependency_level,
                        current_level + 1);

                    // 如果入度变为0，加入队列（复用已找到的iterator）
                    if (in_degrees_.find(dependent).value() == 0)
                    {
                        topo_queue_.add(dependent);
                        topo_levels_.add(dep_it.value().logical_dependency_level);
                    }
                }
                else if (in_degrees_.find(dependent).value() == 0)
                {
                    // 备用路径，正常情况下不应该执行到这里
                    topo_queue_.add(dependent);
                    topo_levels_.add(0);
                }
            }
        }
    }

    // 第五步：计算最大依赖深度并构建级别分组
    logical_topology_.max_logical_dependency_depth = 0;
    for (const auto& [pass, deps] : pass_dependencies_)
    {
        logical_topology_.max_logical_dependency_depth = std::max(
            logical_topology_.max_logical_dependency_depth,
            deps.logical_dependency_level);
    }

    // 构建级别分组
    logical_topology_.logical_levels.resize_default(logical_topology_.max_logical_dependency_depth + 1);

    for (uint32_t i = 0; i <= logical_topology_.max_logical_dependency_depth; ++i)
    {
        logical_topology_.logical_levels[i].level = i;
        logical_topology_.logical_levels[i].passes.clear();
    }

    // 将节点分配到对应级别
    for (const auto& [pass, deps] : pass_dependencies_)
    {
        logical_topology_.logical_levels[deps.logical_dependency_level].passes.add(pass);
    }
}

void PassDependencyAnalysis::identify_logical_critical_path()
{
    SkrZoneScopedN("IdentifyLogicalCriticalPath");
    
    // Step 1: 计算每个节点的高度（到DAG末尾的最长距离）
    // 按逆拓扑序处理，确保处理节点时其所有后继都已处理
    for (int i = logical_topology_.logical_topological_order.size() - 1; i >= 0; --i)
    {
        auto* pass = logical_topology_.logical_topological_order[i];
        auto pass_it = pass_dependencies_.find(pass);
        if (!pass_it)
            continue; // 如果没有依赖信息，跳过

        // 如果没有后继节点，高度为0
        if (pass_it.value().dependent_by_passes.is_empty())
        {
            pass_it.value().logical_critical_path_length = 0;
        }
        else
        {
            // 高度 = 1 + 所有后继节点的最大高度
            uint32_t max_height = 0;
            for (auto* dependent : pass_it.value().dependent_by_passes)
            {
                if (auto dep_it = pass_dependencies_.find(dependent))
                {
                    max_height = std::max(max_height, dep_it.value().logical_critical_path_length);
                }
            }
            pass_it.value().logical_critical_path_length = 1 + max_height;
        }
    }

    // Step 2: 找到高度最大的起始节点（没有前驱的节点）
    PassNode* critical_start = nullptr;
    uint32_t max_height = 0;

    for (const auto& [pass, deps] : pass_dependencies_)
    {
        // 起始节点：没有依赖其他Pass
        if (deps.dependent_passes.is_empty() && deps.logical_critical_path_length > max_height)
        {
            max_height = deps.logical_critical_path_length;
            critical_start = pass;
        }
    }

    // Step 3: 沿着高度递减的路径追踪关键路径
    if (critical_start)
    {
        PassNode* current = critical_start;

        while (current)
        {
            logical_topology_.logical_critical_path.add(current);

            // 在所有后继中选择高度最大的
            PassNode* next = nullptr;
            uint32_t next_height = 0;

            if (auto it = pass_dependencies_.find(current))
            {
                for (auto* dependent : it.value().dependent_by_passes)
                {
                    if (auto dep_it = pass_dependencies_.find(dependent))
                    {
                        // 选择高度最大的后继（如果有多个相同高度的，选择第一个）
                        if (dep_it.value().logical_critical_path_length > next_height ||
                            (dep_it.value().logical_critical_path_length == next_height && !next))
                        {
                            next_height = dep_it.value().logical_critical_path_length;
                            next = dependent;
                        }
                    }
                }
            }

            current = next;
        }
    }
}

uint32_t PassDependencyAnalysis::get_dependency_level(PassNode* pass) const
{
    auto it = pass_dependencies_.find(pass);
    return it ? it.value().logical_dependency_level : 0;
}

uint32_t PassDependencyAnalysis::get_topological_order(PassNode* pass) const
{
    auto it = pass_dependencies_.find(pass);
    return it ? it.value().logical_topological_order : UINT32_MAX;
}

uint32_t PassDependencyAnalysis::get_logical_critical_path_length(PassNode* pass) const
{
    auto it = pass_dependencies_.find(pass);
    return it ? it.value().logical_critical_path_length : 0;
}

bool PassDependencyAnalysis::can_execute_in_parallel_logically(PassNode* pass1, PassNode* pass2) const
{
    if (!pass1 || !pass2 || pass1 == pass2) return false;

    auto it1 = pass_dependencies_.find(pass1);
    auto it2 = pass_dependencies_.find(pass2);

    if (!it1 || !it2) return false;

    // Passes can execute in parallel logically if they are at the same logical dependency level
    // and neither depends on the other
    if (it1.value().logical_dependency_level != it2.value().logical_dependency_level) return false;

    // Check if pass1 depends on pass2
    for (auto* dep : it1.value().dependent_passes)
    {
        if (dep == pass2) return false;
    }

    // Check if pass2 depends on pass1
    for (auto* dep : it2.value().dependent_passes)
    {
        if (dep == pass1) return false;
    }

    return true;
}

const PassDependencies* PassDependencyAnalysis::get_pass_dependencies(PassNode* pass) const
{
    auto it = pass_dependencies_.find(pass);
    return it ? &it.value() : nullptr;
}

bool PassDependencyAnalysis::has_dependencies(PassNode* pass) const
{
    const auto* deps = get_pass_dependencies(pass);
    return deps != nullptr && !deps->resource_dependencies.is_empty();
}

// For ScheduleTimeline - get pass-level dependencies directly
const StackVector<PassNode*>& PassDependencyAnalysis::get_dependent_passes(PassNode* pass) const
{
    static const StackVector<PassNode*> empty_vector;
    const auto* deps = get_pass_dependencies(pass);
    return deps ? deps->dependent_passes : empty_vector;
}

const StackVector<PassNode*>& PassDependencyAnalysis::get_dependent_by_passes(PassNode* pass) const
{
    static const StackVector<PassNode*> empty_vector;
    const auto* deps = get_pass_dependencies(pass);
    return deps ? deps->dependent_by_passes : empty_vector;
}

void PassDependencyAnalysis::dump_dependencies() const
{
    SKR_LOG_INFO(u8"========== Pass Dependency Analysis Results ==========");

    for (const auto& [pass, deps] : pass_dependencies_)
    {
        if (deps.resource_dependencies.is_empty())
            continue;

        SKR_LOG_INFO(u8"Pass: %s depends on:", pass->get_name());

        for (const auto& dep : deps.resource_dependencies)
        {
            const char8_t* dep_type_str = u8"Unknown";
            const char8_t* current_access_str = u8"Unknown";
            const char8_t* previous_access_str = u8"Unknown";

            switch (dep.current_access)
            {
            case EResourceAccessType::Read:
                current_access_str = u8"Read";
                break;
            case EResourceAccessType::Write:
                current_access_str = u8"Write";
                break;
            case EResourceAccessType::ReadWrite:
                current_access_str = u8"ReadWrite";
                break;
            }

            switch (dep.previous_access)
            {
            case EResourceAccessType::Read:
                previous_access_str = u8"Read";
                break;
            case EResourceAccessType::Write:
                previous_access_str = u8"Write";
                break;
            case EResourceAccessType::ReadWrite:
                previous_access_str = u8"ReadWrite";
                break;
            }

            SKR_LOG_INFO(u8"  -> Pass: %s, Resource: %s, %s->%s, State: %d->%d",
                dep.dependent_pass->get_name(),
                dep.resource->get_name(),
                previous_access_str,
                current_access_str,
                (int)dep.previous_state,
                (int)dep.current_state);
        }
    }

    SKR_LOG_INFO(u8"==========================================");
}

void PassDependencyAnalysis::dump_logical_topology() const
{
    SKR_LOG_INFO(u8"========== Logical Topology Analysis ==========");
    SKR_LOG_INFO(u8"Max logical dependency depth: %u", logical_topology_.max_logical_dependency_depth);
    SKR_LOG_INFO(u8"Logical topological order:");

    for (size_t i = 0; i < logical_topology_.logical_topological_order.size(); ++i)
    {
        auto* pass = logical_topology_.logical_topological_order[i];
        uint32_t level = get_dependency_level(pass);

        SKR_LOG_INFO(u8"  [%zu] %s (logical level: %u)", i, pass->get_name(), level);
    }

    SKR_LOG_INFO(u8"\nLogical dependency levels:");
    for (const auto& level : logical_topology_.logical_levels)
    {
        SKR_LOG_INFO(u8"Level %u (%u passes):",
            level.level,
            static_cast<uint32_t>(level.passes.size()));

        for (auto* pass : level.passes)
        {
            uint32_t critical_length = get_logical_critical_path_length(pass);
            SKR_LOG_INFO(u8"  - %s (critical path length: %u)",
                pass->get_name(),
                critical_length);
        }
    }

    SKR_LOG_INFO(u8"=============================================");
}

void PassDependencyAnalysis::dump_logical_critical_path() const
{
    SKR_LOG_INFO(u8"========== Logical Critical Path ==========");
    SKR_LOG_INFO(u8"Logical critical path length: %u", static_cast<uint32_t>(logical_topology_.logical_critical_path.size()));

    for (size_t i = 0; i < logical_topology_.logical_critical_path.size(); ++i)
    {
        auto* pass = logical_topology_.logical_critical_path[i];
        uint32_t level = get_dependency_level(pass);

        SKR_LOG_INFO(u8"[%zu] %s (logical level: %u)",
            i,
            pass->get_name(),
            level);
    }

    SKR_LOG_INFO(u8"=========================================");
}

// 跨队列同步点生成方法
void PassDependencyAnalysis::generate_cross_queue_sync_points(const QueueSchedule& queue_schedule, StackVector<CrossQueueSyncPoint>& sync_points) const
{
    const auto& queue_result = queue_schedule.get_schedule_result();
    sync_points.clear();
    
    // 为每个Pass检查其依赖关系
    for (const auto& [consumer_pass, deps] : pass_dependencies_)
    {
        uint32_t consumer_queue = queue_result.pass_queue_assignments.find(consumer_pass).value();
        
        // 检查每个资源依赖
        for (const auto& resource_dep : deps.resource_dependencies)
        {
            PassNode* producer_pass = resource_dep.dependent_pass;
            uint32_t producer_queue = queue_result.pass_queue_assignments.find(producer_pass).value();
            
            // 如果生产者和消费者在不同队列上，创建跨队列同步点
            if (producer_queue != consumer_queue)
            {
                CrossQueueSyncPoint sync_point;
                sync_point.type = ESyncPointType::Signal;
                sync_point.producer_queue_index = producer_queue;
                sync_point.consumer_queue_index = consumer_queue;
                sync_point.producer_pass = producer_pass;
                sync_point.consumer_pass = consumer_pass;
                sync_point.resource = resource_dep.resource;
                sync_point.from_state = resource_dep.previous_state;
                sync_point.to_state = resource_dep.current_state;
                sync_point.sync_value = 0;
                
                sync_points.add(sync_point);
            }
        }
    }
}

} // namespace RG
} // namespace skr