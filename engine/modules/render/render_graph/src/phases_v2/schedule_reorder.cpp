#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/phases_v2/schedule_reorder.hpp"
#include "SkrProfile/profile.h"
#include "SkrContainersDef/set.hpp"
#include <cstdint>

namespace skr {
namespace render_graph {

// Constructor
ExecutionReorderPhase::ExecutionReorderPhase(
    const PassInfoAnalysis& pass_info,
    const PassDependencyAnalysis& dependency_analysis,
    const QueueSchedule& timeline,
    const ExecutionReorderConfig& config)
    : config(config)
    , pass_info_analysis(pass_info)
    , dependency_analysis(dependency_analysis)
    , timeline_schedule(timeline)
{
}

void ExecutionReorderPhase::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("ExecutionReorderPhase");
    
    render_graph = graph;
    if (false)
    {
        // Step 1: Create a copy of the timeline
        working_timeline = timeline_schedule.get_schedule_result().queue_schedules;
        
        // Step 2: Run graph-based optimization (no need to rebuild resource chains!)
        run_graph_based_optimization();
        
        // Step 3: Store optimized results
        result.optimized_timeline = std::move(working_timeline);
    }
    else
    {
        result.optimized_timeline = timeline_schedule.get_schedule_result().queue_schedules;
    }
}

// Run graph-based optimization - much simpler!
void ExecutionReorderPhase::run_graph_based_optimization() SKR_NOEXCEPT
{
    // Optimize each queue independently
    for (size_t i = 0; i < working_timeline.size(); ++i)
    {
        optimize_queue_with_graph(i);
    }
}

// Optimize a single queue using RenderGraph DAG - simplified single-pass algorithm
void ExecutionReorderPhase::optimize_queue_with_graph(uint32_t queue_idx) SKR_NOEXCEPT
{
    if (working_timeline[queue_idx].size() < 2) 
        return;
    
    auto& passes = working_timeline[queue_idx];
    
    // Single forward pass: for each position, find the best candidate to move there
    // This avoids complex multi-iteration logic and index invalidation issues
    for (size_t current_pos = 0; current_pos < passes.size() - 1; ++current_pos)
    {
        PassNode* current_pass = passes[current_pos];
        
        // Find the best candidate to move to position current_pos + 1
        PassNode* best_candidate = nullptr;
        size_t best_candidate_pos = SIZE_MAX;
        float best_affinity = config.min_affinity_score;
        
        // Scan subsequent positions within attraction distance
        size_t max_scan_pos = std::min((uint64_t)passes.size(), (uint64_t)current_pos + 1 + config.max_attraction_distance);
        for (size_t candidate_pos = current_pos + 1; candidate_pos < max_scan_pos; ++candidate_pos)
        {
            PassNode* candidate_pass = passes[candidate_pos];
            
            // Check if this candidate should be attracted to current_pass
            if (should_attract_passes(current_pass, candidate_pass, config.enable_cache_opt, config.enable_lifetime_opt))
            {
                // Check if it's safe to move the candidate
                if (can_attract_pass_safely(queue_idx, current_pos, candidate_pos))
                {
                    // Calculate affinity to find the best candidate
                    auto shared_resources = get_shared_resources(current_pass, candidate_pass);
                    float affinity = calculate_resource_affinity_from_shared(current_pass, candidate_pass, shared_resources);
                    
                    if (affinity > best_affinity)
                    {
                        best_candidate = candidate_pass;
                        best_candidate_pos = candidate_pos;
                        best_affinity = affinity;
                    }
                }
            }
        }
        
        // If we found a good candidate and it's not already in the target position, move it
        if (best_candidate && best_candidate_pos != SIZE_MAX && best_candidate_pos != current_pos + 1)
        {
            attract_pass(queue_idx, best_candidate_pos, current_pos + 1);
        }
    }
}

// Graph-based safety check - much simpler than resource chain analysis!
bool ExecutionReorderPhase::can_attract_pass_safely(uint32_t queue_idx, size_t current_pos, size_t target_pos) SKR_NOEXCEPT
{
    if (current_pos >= target_pos) 
        return false;
    
    const auto& passes = working_timeline[queue_idx];
    PassNode* target_pass = passes[target_pos];
    
    // Check if moving target_pass to position current_pos+1 would violate dependencies
    // We only need to check the passes that will be "jumped over" by the move
    
    for (size_t i = current_pos + 1; i < target_pos; ++i)
    {
        PassNode* intermediate_pass = passes[i];
        
        // 1. target_pass cannot depend on any pass it will jump over
        if (has_path_between_passes(target_pass, intermediate_pass))
        {
            return false; // target depends on intermediate, cannot move before it
        }
        
        // 2. no pass being jumped over can depend on target_pass  
        if (has_path_between_passes(intermediate_pass, target_pass))
        {
            return false; // intermediate depends on target, target cannot move before it
        }
    }
    
    return true;
}

// Unified attraction logic - scans shared resources only once
bool ExecutionReorderPhase::should_attract_passes(PassNode* current_pass, PassNode* target_pass, bool check_cache, bool check_lifetime) const SKR_NOEXCEPT
{
    // Early exit if no checks enabled
    if (!check_cache && !check_lifetime) 
        return false;
    
    // Single shared resource scan - expensive operation done once
    auto shared_resources = get_shared_resources(current_pass, target_pass);
    
    // Early exit if no shared resources
    if (shared_resources.is_empty()) 
        return false;
    
    // Check both optimizations properly
    bool should_attract_for_lifetime = false;
    bool should_attract_for_cache = false;
    
    // Lifetime optimization: any shared resources mean potential for compression
    if (check_lifetime)
    {
        should_attract_for_lifetime = true;
    }
    
    // Cache optimization: check affinity threshold using already computed shared resources
    if (check_cache)
    {
        float affinity = calculate_resource_affinity_from_shared(current_pass, target_pass, shared_resources);
        should_attract_for_cache = (affinity >= config.min_affinity_score);
    }
    
    // Attract if either optimization thinks it's beneficial
    return should_attract_for_lifetime || should_attract_for_cache;
}

// Graph traversal: check if there's a directed path from from_pass to to_pass
bool ExecutionReorderPhase::has_path_between_passes(PassNode* from_pass, PassNode* to_pass) const SKR_NOEXCEPT
{
    if (!from_pass || !to_pass) return false;
    if (from_pass == to_pass) return false; // No self-loops in this context
    
    // Check for direct dependency first
    const auto* deps = dependency_analysis.get_pass_dependencies(to_pass);
    if (!deps) return false;
    
    // Direct dependency
    if (deps->has_dependency_on(from_pass)) return true;
    
    // Check transitive dependencies using BFS to avoid deep recursion
    StackSet<PassNode*> visited;
    StackVector<PassNode*> queue;

    // Start with direct dependencies of to_pass
    for (auto* dep_pass : deps->dependent_passes)
    {
        if (dep_pass == from_pass) return true;
        auto ref = visited.add(dep_pass);
        if (!ref.already_exist())
        {
            queue.add(dep_pass);
        }
    }
    
    // BFS through dependency chain
    for (size_t i = 0; i < queue.size(); ++i)
    {
        PassNode* current = queue[i];
        const auto* current_deps = dependency_analysis.get_pass_dependencies(current);
        if (!current_deps) continue;
        
        for (auto* next_dep : current_deps->dependent_passes)
        {
            if (next_dep == from_pass) return true;
            auto ref = visited.add(next_dep);
            if (!ref.already_exist())
            {
                queue.add(next_dep);
            }
        }
    }
    
    return false;
}

// Get resources shared between two passes using graph
StackVector<ResourceNode*> ExecutionReorderPhase::get_shared_resources(PassNode* pass1, PassNode* pass2) const SKR_NOEXCEPT
{
    StackVector<ResourceNode*> shared;
    StackSet<ResourceNode*> resources1;

    // Use pass info analysis to get resource accesses efficiently
    const auto* info1 = pass_info_analysis.get_pass_info(pass1);
    const auto* info2 = pass_info_analysis.get_pass_info(pass2);

    for (const auto& access : info1->resource_info.resource_accesses)
    {
        resources1.add(access.resource);
    }
    
    for (const auto& access : info2->resource_info.resource_accesses)
    {
        if (resources1.contains(access.resource))
        {
            shared.add(access.resource);
        }
    }
    
    return shared;
}

// Perform the attraction (move pass from from_pos to to_pos)
void ExecutionReorderPhase::attract_pass(uint32_t queue_idx, size_t from_pos, size_t to_pos) SKR_NOEXCEPT
{
    auto& passes = working_timeline[queue_idx];
    
    // Validate indices
    if (from_pos >= passes.size() || to_pos >= passes.size() || from_pos == to_pos) 
        return;
    
    // Move the pass: save the element, remove it, then insert at correct position
    PassNode* pass_to_move = passes[from_pos];
    passes.remove_at(from_pos);
    
    // Since we removed an element before to_pos, we need to adjust the insertion position
    // No adjustment needed because we're moving from back to front (from_pos > to_pos)
    passes.add_at(to_pos, pass_to_move);
    
    // No need to update complex data structures - we use the graph directly!
}

float ExecutionReorderPhase::calculate_resource_affinity(PassNode* pass1, PassNode* pass2) const SKR_NOEXCEPT
{
    // Legacy function - calls optimized version
    auto shared_resources = get_shared_resources(pass1, pass2);
    return calculate_resource_affinity_from_shared(pass1, pass2, shared_resources);
}

float ExecutionReorderPhase::calculate_resource_affinity_from_shared(PassNode* pass1, PassNode* pass2, const StackVector<ResourceNode*>& shared_resources) const SKR_NOEXCEPT
{
    const auto* info1 = pass_info_analysis.get_pass_info(pass1);
    const auto* info2 = pass_info_analysis.get_pass_info(pass2);
    
    if (!info1 || !info2) return 0.0f;
    
    uint32_t shared_count = static_cast<uint32_t>(shared_resources.size());
    uint32_t total_resources = info1->resource_info.total_resource_count + info2->resource_info.total_resource_count;
    
    if (total_resources == 0) return 0.0f;
    
    // Jaccard similarity: |intersection| / |union|
    // union = total - shared (since shared was counted twice)
    uint32_t union_size = total_resources - shared_count;
    if (union_size == 0) return 0.0f;
    
    return static_cast<float>(shared_count) / static_cast<float>(union_size);
}


} // namespace render_graph
} // namespace skr