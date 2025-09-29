#include "SkrRenderGraph/phases_v2/cross_queue_sync_analysis.hpp"
#include "SkrRenderGraph/phases_v2/pass_info_analysis.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrCore/log.hpp"
#include <algorithm>

#define SSIS_LOG(...)

namespace skr {
namespace RG {

CrossQueueSyncAnalysis::CrossQueueSyncAnalysis(
    const PassDependencyAnalysis& dependency_analysis,
    const QueueSchedule& queue_schedule,
    const CrossQueueSyncConfig& config)
    : config_(config)
    , dependency_analysis_(dependency_analysis)
    , queue_schedule_(queue_schedule)
{
}

void CrossQueueSyncAnalysis::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("CrossQueueSyncAnalysis");
    SSIS_LOG(u8"CrossQueueSyncAnalysis: Starting SSIS analysis");

    // 构建Pass到队列的映射缓存
    const auto& queue_result = queue_schedule_.get_schedule_result();
    
    // Step 1: 从 PassDependencyAnalysis 生成原始同步点
    dependency_analysis_.generate_cross_queue_sync_points(queue_schedule_, ssis_result_.raw_sync_points);
    ssis_result_.total_raw_syncs = static_cast<uint32_t>(ssis_result_.raw_sync_points.size());
    SSIS_LOG(u8"CrossQueueSyncAnalysis: Received %u raw sync points from PassDependencyAnalysis", ssis_result_.total_raw_syncs);
    
    // Step 3: 应用SSIS优化算法
    if (config_.enable_ssis_optimization)
    {
        apply_ssis_optimization(graph);
    }
    else
    {
        // 如果不启用SSIS，直接复制原始同步点
        ssis_result_.optimized_sync_points = ssis_result_.raw_sync_points;
    }
    
    // Step 5: 计算优化统计信息
    calculate_optimization_statistics();
    
    // 可选：输出调试信息
    if (config_.enable_debug_output)
    {
        dump_ssis_analysis();
    }
    
    SSIS_LOG(u8"CrossQueueSyncAnalysis: SSIS analysis completed - reduced %u sync points to %u (%.1f%% reduction)",
                  ssis_result_.total_raw_syncs,
                  ssis_result_.total_optimized_syncs,
                  ssis_result_.optimization_ratio * 100.0f);
}


void CrossQueueSyncAnalysis::apply_ssis_optimization(RenderGraph* graph) SKR_NOEXCEPT
{
    SSIS_LOG(u8"CrossQueueSyncAnalysis: Applying SSIS optimization algorithm");
    
    const auto& queue_result = queue_schedule_.get_schedule_result();
    total_queue_count_ = static_cast<uint32_t>(queue_result.queue_schedules.size());
    
    // Step 1: 初始化SSIS值和本地队列索引
    for (auto* pass : get_passes(graph))
    {
        // 初始化SSIS为InvalidSyncIndex
        auto& ssis = pass_ssis_.try_add_default(pass).value();
        ssis.resize(total_queue_count_, InvalidSyncIndex);
        
        // 初始化nodes_to_sync_with为空
        pass_nodes_to_sync_with_.try_add_default(pass).value().clear();
    }
    
    // 设置每个Pass的本地队列索引
    for (uint32_t queue_idx = 0; queue_idx < queue_result.queue_schedules.size(); ++queue_idx)
    {
        const auto& queue_schedule = queue_result.queue_schedules[queue_idx];
        for (uint32_t local_idx = 0; local_idx < queue_schedule.size(); ++local_idx)
        {
            PassNode* pass = queue_schedule[local_idx];
            pass_local_to_queue_indices_.add(pass, local_idx);
        }
    }
    
    // Step 2: 第一阶段 - 构建初始SSIS值
    // 从原始同步点构建需要同步的节点列表
    for (const auto& sync_point : ssis_result_.raw_sync_points)
    {
        PassNode* consumer = sync_point.consumer_pass;
        PassNode* producer = sync_point.producer_pass;
        
        // 添加到需要同步的节点列表
        auto& nodes_to_sync = pass_nodes_to_sync_with_.try_add_default(consumer).value();
        
        // 避免重复添加
        bool already_exists = false;
        for (auto* node : nodes_to_sync)
        {
            if (node == producer)
            {
                already_exists = true;
                break;
            }
        }
        if (!already_exists)
        {
            nodes_to_sync.add(producer);
        }
    }
    
    // 为每个Pass计算初始SSIS
    for (auto* pass : get_passes(graph))
    {
        uint32_t pass_queue = get_pass_queue_index(pass);
        auto& ssis = pass_ssis_.find(pass).value();
        auto& nodes_to_sync = pass_nodes_to_sync_with_.try_add_default(pass).value();
        
        // 找到每个队列上最近的依赖节点
        StackVector<PassNode*> closest_nodes_per_queue;
        closest_nodes_per_queue.resize(total_queue_count_, nullptr);
        
        for (PassNode* dep_node : nodes_to_sync)
        {
            uint32_t dep_queue = get_pass_queue_index(dep_node);
            uint32_t dep_local_idx = pass_local_to_queue_indices_.find(dep_node).value();
            
            PassNode*& closest = closest_nodes_per_queue[dep_queue];
            if (!closest || dep_local_idx > pass_local_to_queue_indices_.find(closest).value())
            {
                closest = dep_node;
            }
        }
        
        // 更新SSIS值和nodes_to_sync_with
        nodes_to_sync.clear();
        for (uint32_t q = 0; q < total_queue_count_; ++q)
        {
            if (PassNode* closest = closest_nodes_per_queue[q])
            {
                // 更新SSIS
                if (q != pass_queue)
                {
                    ssis[q] = pass_local_to_queue_indices_.find(closest).value();
                }
                // 只保留最近的节点
                nodes_to_sync.add(closest);
            }
        }
        
        // 设置自己队列的SSIS值
        ssis[pass_queue] = pass_local_to_queue_indices_.find(pass).value();
        
        SSIS_LOG(u8"  Pass %s initial SSIS: [%u,%u], nodes_to_sync: %u", 
                pass->get_name(), ssis[0], ssis.size() > 1 ? ssis[1] : 0, 
                static_cast<uint32_t>(nodes_to_sync.size()));
    }
    
    // Step 3: 第二阶段 - 通过SSIS比较剔除冗余同步
    for (auto* pass : get_passes(graph))
    {
        uint32_t pass_queue = get_pass_queue_index(pass);
        auto& ssis = pass_ssis_.find(pass).value();
        auto& nodes_to_sync = pass_nodes_to_sync_with_.find(pass).value();
        
        if (nodes_to_sync.is_empty())
            continue;
        
        // 构建需要同步的队列集合
        StackSet<uint32_t> queues_to_sync_with;
        for (PassNode* node : nodes_to_sync)
        {
            uint32_t node_queue = get_pass_queue_index(node);
            queues_to_sync_with.add(node_queue);
        }
        
        // 优化后的节点列表
        StackVector<PassNode*> optimal_nodes_to_sync;
        
        // 迭代直到所有队列都被覆盖
        while (!queues_to_sync_with.is_empty())
        {
            StackVector<SyncCoverage> sync_coverage_array;
            uint32_t max_syncs_covered = 0;
            
            // 计算每个依赖节点的覆盖情况
            for (uint32_t dep_idx = 0; dep_idx < nodes_to_sync.size(); ++dep_idx)
            {
                PassNode* dep_node = nodes_to_sync[dep_idx];
                const auto& dep_ssis = pass_ssis_.find(dep_node).value();
                
                SyncCoverage coverage;
                coverage.node_to_sync_with = dep_node;
                coverage.node_index = dep_idx;
                
                // 检查这个节点能覆盖哪些队列的同步
                for (uint32_t queue_idx : queues_to_sync_with)
                {
                    uint32_t current_desired_sync = ssis[queue_idx];
                    uint32_t dep_sync_index = dep_ssis[queue_idx];
                    
                    // 特殊处理：如果是当前Pass所在的队列，减1
                    if (queue_idx == pass_queue && current_desired_sync != InvalidSyncIndex)
                    {
                        current_desired_sync = (current_desired_sync > 0) ? current_desired_sync - 1 : 0;
                    }
                    
                    // 检查是否覆盖
                    if (dep_sync_index != InvalidSyncIndex && dep_sync_index >= current_desired_sync)
                    {
                        coverage.synced_queue_indices.add(queue_idx);
                    }
                }
                
                if (!coverage.synced_queue_indices.is_empty())
                {
                    sync_coverage_array.add(coverage);
                    max_syncs_covered = std::max(max_syncs_covered, 
                                                static_cast<uint32_t>(coverage.synced_queue_indices.size()));
                }
            }
            
            // 找到覆盖最多队列的节点
            bool found_coverage = false;
            for (const auto& coverage : sync_coverage_array)
            {
                if (coverage.synced_queue_indices.size() == max_syncs_covered)
                {
                    // 只添加跨队列的同步（同队列是隐式的）
                    if (get_pass_queue_index(coverage.node_to_sync_with) != pass_queue)
                    {
                        optimal_nodes_to_sync.add(coverage.node_to_sync_with);
                    }
                    
                    // 更新SSIS值（可能比期望的更大）
                    const auto& node_ssis = pass_ssis_.find(coverage.node_to_sync_with).value();
                    for (uint32_t q : coverage.synced_queue_indices)
                    {
                        if (node_ssis[q] != InvalidSyncIndex)
                        {
                            ssis[q] = std::max(ssis[q], node_ssis[q]);
                        }
                    }
                    
                    // 移除已覆盖的队列
                    for (uint32_t q : coverage.synced_queue_indices)
                    {
                        queues_to_sync_with.remove(q);
                    }
                    
                    found_coverage = true;
                    break; // 每次迭代只选择一个节点
                }
            }
            
            if (!found_coverage)
            {
                // 无法覆盖剩余队列，退出
                SSIS_LOG(u8"    Warning: Pass %s cannot cover remaining queues", pass->get_name());
                break;
            }
            
            // 从nodes_to_sync中移除已选择的节点（反向迭代避免索引失效）
            for (auto& it : sync_coverage_array.range_inv())
            {
                if (it.synced_queue_indices.size() == max_syncs_covered)
                {
                    nodes_to_sync.erase(nodes_to_sync.begin() + it.node_index);
                    break;
                }
            }
        }
        
        // 更新优化后的同步点
        for (PassNode* optimal_node : optimal_nodes_to_sync)
        {
            // 在原始同步点中找到对应的同步点
            for (const auto& raw_sync : ssis_result_.raw_sync_points)
            {
                if (raw_sync.producer_pass == optimal_node && raw_sync.consumer_pass == pass)
                {
                    ssis_result_.optimized_sync_points.add(raw_sync);
                    SSIS_LOG(u8"    Optimized sync: %s -> %s", 
                            optimal_node->get_name(), pass->get_name());
                    break;
                }
            }
        }
    }
    
    SSIS_LOG(u8"CrossQueueSyncAnalysis: SSIS optimization completed - %u sync points after optimization",
            static_cast<uint32_t>(ssis_result_.optimized_sync_points.size()));
}

void CrossQueueSyncAnalysis::calculate_optimization_statistics() SKR_NOEXCEPT
{
    ssis_result_.total_optimized_syncs = static_cast<uint32_t>(ssis_result_.optimized_sync_points.size());
    ssis_result_.sync_reduction_count = ssis_result_.total_raw_syncs - ssis_result_.total_optimized_syncs;
    
    if (ssis_result_.total_raw_syncs > 0)
    {
        ssis_result_.optimization_ratio = static_cast<float>(ssis_result_.sync_reduction_count) / static_cast<float>(ssis_result_.total_raw_syncs);
    }
    else
    {
        ssis_result_.optimization_ratio = 0.0f;
    }
}

uint32_t CrossQueueSyncAnalysis::get_pass_queue_index(PassNode* pass) const SKR_NOEXCEPT
{
    return queue_schedule_.get_schedule_result().pass_queue_assignments.find(pass).value();
}

void CrossQueueSyncAnalysis::dump_ssis_analysis() const SKR_NOEXCEPT
{
    SKR_LOG_INFO(u8"========== SSIS Analysis Results ==========");
    SKR_LOG_INFO(u8"Raw sync points: %u", ssis_result_.total_raw_syncs);
    SKR_LOG_INFO(u8"Optimized sync points: %u", ssis_result_.total_optimized_syncs);
    SKR_LOG_INFO(u8"Sync reduction: %u (%.1f%%)", 
                 ssis_result_.sync_reduction_count,
                 ssis_result_.optimization_ratio * 100.0f);
    
    SKR_LOG_INFO(u8"\nOptimized Sync Points:");
    for (size_t i = 0; i < ssis_result_.optimized_sync_points.size(); ++i)
    {
        const auto& sync = ssis_result_.optimized_sync_points[i];
        const char8_t* type_str = (sync.type == ESyncPointType::Signal) ? u8"Signal" : u8"Wait";
        
        SKR_LOG_INFO(u8"  [%zu] %s: %s (queue %u) -> %s (queue %u) | Resource: %s | States: %d->%d",
                     i, type_str,
                     sync.producer_pass->get_name(), sync.producer_queue_index,
                     sync.consumer_pass->get_name(), sync.consumer_queue_index,
                     sync.resource->get_name(),
                     static_cast<int>(sync.from_state),
                     static_cast<int>(sync.to_state));
    }
    
    SKR_LOG_INFO(u8"==========================================");
}

void CrossQueueSyncAnalysis::dump_sync_points() const SKR_NOEXCEPT
{
    dump_ssis_analysis(); // 目前复用相同的输出
}

} // namespace RG
} // namespace skr