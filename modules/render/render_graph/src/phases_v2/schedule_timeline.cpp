#include "SkrGraphics/api.h"
#include "SkrRenderGraph/phases_v2/schedule_timeline.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"

namespace skr {
namespace render_graph {

ScheduleTimeline::ScheduleTimeline(const ScheduleTimelineConfig& cfg)
    : config(cfg)
{
}

ScheduleTimeline::~ScheduleTimeline() = default;

void ScheduleTimeline::on_initialize(RenderGraph* graph) SKR_NOEXCEPT
{
    // 查询队列能力
    query_queue_capabilities(graph);
    
    // 初始化调度结果
    schedule_result.queue_schedules.clear();
    schedule_result.sync_requirements.clear(); 
    schedule_result.pass_queue_assignments.clear();
}

void ScheduleTimeline::on_finalize(RenderGraph* graph) SKR_NOEXCEPT
{
    // 清理分配的Fence资源
    for (auto& requirement : schedule_result.sync_requirements) {
        if (requirement.fence) {
            cgpu_free_fence(requirement.fence);
            requirement.fence = nullptr;
        }
    }
    
    // 清理所有状态
    clear_frame_data();
}

void ScheduleTimeline::clear_frame_data() SKR_NOEXCEPT
{
    // 清理调度结果
    schedule_result.queue_schedules.clear();
    schedule_result.sync_requirements.clear();
    schedule_result.pass_queue_assignments.clear();
    
    // 清理每帧重建的数据
    dependency_info.clear();
    pass_to_queue.clear();
    pass_preferred_queue.clear();
}

void ScheduleTimeline::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SKR_LOG_DEBUG(u8"ScheduleTimeline: Starting timeline scheduling and fence allocation");
    
    // 0. 清空上一帧的数据 - RG中的资源和Pass每帧都不稳定
    clear_frame_data();
    
    // 1. 分析Pass依赖关系
    analyze_dependencies(graph);
    
    // 2. 对Pass进行分类
    classify_passes(graph);
    
    // 3. 分配Pass到队列
    assign_passes_to_queues(graph);
    
    // 4. 计算同步需求
    calculate_sync_requirements(graph);
    
    // 5. 分配Fence对象
    allocate_fences(graph);
    
    SKR_LOG_DEBUG(u8"ScheduleTimeline: Scheduling completed. Queues: %d, SyncRequirements: %d", 
                  (int)schedule_result.queue_schedules.size(), (int)schedule_result.sync_requirements.size());
}

void ScheduleTimeline::query_queue_capabilities(RenderGraph* graph) SKR_NOEXCEPT
{
    // 清空队列信息
    available_queues.clear();
    graphics_queue_index = UINT32_MAX;
    compute_queue_indices.clear();
    copy_queue_indices.clear();
    
    uint32_t queue_index = 0;
    
    // 添加Graphics队列（总是存在）
    if (auto gfx_queue = graph->get_gfx_queue()) {
        available_queues.emplace(QueueCapabilities{
            .supports_graphics = true, .supports_compute = true, .supports_copy = true, .supports_present = true,
            .family_index = 0, .queue_handle = gfx_queue
        });
        graphics_queue_index = queue_index++;
    }
    
    // 添加AsyncCompute队列（多个）
    if (config.enable_async_compute) {
        const auto& cmpt_queues = graph->get_cmpt_queues();
        uint32_t max_compute = skr::min((uint32_t)cmpt_queues.size(), config.max_async_compute_queues);
        for (uint32_t i = 0; i < max_compute; ++i) {
            available_queues.emplace(QueueCapabilities{
                .supports_compute = true, .supports_copy = true,
                .family_index = 1, .queue_handle = cmpt_queues[i]
            });
            compute_queue_indices.add(queue_index++);
        }
    }
    
    // 添加Copy队列（多个）
    if (config.enable_copy_queue) {
        const auto& cpy_queues = graph->get_cpy_queues();
        uint32_t max_copy = skr::min((uint32_t)cpy_queues.size(), config.max_copy_queues);
        for (uint32_t i = 0; i < max_copy; ++i) {
            available_queues.emplace(QueueCapabilities{
                .supports_copy = true,
                .family_index = 2, .queue_handle = cpy_queues[i]
            });
            copy_queue_indices.add(queue_index++);
        }
    }
    
    SKR_LOG_DEBUG(u8"ScheduleTimeline: Queue capabilities - Graphics: %s, AsyncCompute: %d queues, Copy: %d queues",
                  graphics_queue_index != UINT32_MAX ? u8"✓" : u8"✗",
                  (int)compute_queue_indices.size(), (int)copy_queue_indices.size());
}

void ScheduleTimeline::analyze_dependencies(RenderGraph* graph) SKR_NOEXCEPT
{
    dependency_info.clear();
    
    auto& passes = get_passes(graph);
    
    // 构建Pass级别的依赖图
    for (auto* pass : passes) {
        auto& info = dependency_info[pass];
        
        // TODO: 从RenderGraph的依赖图中提取Pass之间的直接依赖关系
        // 当前简化实现：通过拓扑顺序推导简单的串行依赖
        
        // 设置队列亲和性 - 根据Pass类型确定可运行的队列
        for (uint32_t i = 0; i < available_queues.size(); ++i) {
            if (can_run_on_queue_index(pass, i)) {
                // 注意：这里我们使用实际队列索引，而不是枚举类型
                info.queue_affinities.set(i);
            }
        }
        
        // 如果Pass可以在多个队列上运行，就有跨队列调度的潜力
        if (info.queue_affinities.count() > 1) {
            info.has_cross_queue_dependency = true;
        }
    }
    
    // 建立简化的Pass依赖关系 - 基于Pass添加顺序的串行依赖
    for (size_t i = 1; i < passes.size(); ++i) {
        auto* current_pass = passes[i];
        auto* previous_pass = passes[i - 1];
        
        // 当前简化处理：每个Pass依赖前一个Pass（严格串行）
        // TODO: 从RenderGraph的真实依赖图中获取更精确的依赖关系
        dependency_info[current_pass].dependencies.add(previous_pass);
        dependency_info[previous_pass].dependents.add(current_pass);
    }
}

void ScheduleTimeline::classify_passes(RenderGraph* graph) SKR_NOEXCEPT
{
    auto& passes = get_passes(graph);
    
    for (auto* pass : passes) {
        // 分类Pass并记录到Map中
        auto queue_type = classify_pass(pass);
        pass_preferred_queue[pass] = queue_type;
    }
}

ERenderGraphQueueType ScheduleTimeline::classify_pass(PassNode* pass) SKR_NOEXCEPT
{
    // 1. Present Pass必须在Graphics队列
    if (pass->pass_type == EPassType::Present) {
        return ERenderGraphQueueType::Graphics;
    }
    
    // 2. Render Pass必须在Graphics队列
    if (pass->pass_type == EPassType::Render) {
        return ERenderGraphQueueType::Graphics;
    }
    
    // 3. Copy Pass优先Copy队列
    if (pass->pass_type == EPassType::Copy && config.enable_copy_queue) {
        auto* copy_pass = static_cast<CopyPassNode*>(pass);
        // 检查是否标记为可以独立执行
        if (copy_pass->get_can_be_lone()) {
            // 检查是否真的没有紧密依赖
            auto& info = dependency_info[pass];
            bool is_isolated = true;
            
            for (auto* dep : info.dependencies) {
                if (dep->pass_type == EPassType::Render) {
                    is_isolated = false;
                    break;
                }
            }
            
            if (is_isolated) {
                return ERenderGraphQueueType::Copy;
            }
        }
    }
    
    // 4. Compute Pass检查异步提示  
    if (pass->pass_type == EPassType::Compute && config.enable_async_compute) {
        // 简化处理：如果没有图形资源依赖，可以异步执行
        if (!has_graphics_resource_dependency(pass)) {
            return ERenderGraphQueueType::AsyncCompute;
        }
    }
    
    // 默认分配到Graphics队列
    return ERenderGraphQueueType::Graphics;
}

bool ScheduleTimeline::can_run_on_queue(PassNode* pass, ERenderGraphQueueType queue) SKR_NOEXCEPT
{
    switch (queue) {
        case ERenderGraphQueueType::Graphics:
            return graphics_queue_index != UINT32_MAX && can_run_on_queue_index(pass, graphics_queue_index);
        case ERenderGraphQueueType::AsyncCompute:
            return !compute_queue_indices.is_empty() && can_run_on_queue_index(pass, compute_queue_indices[0]);
        case ERenderGraphQueueType::Copy:
            return !copy_queue_indices.is_empty() && can_run_on_queue_index(pass, copy_queue_indices[0]);
        default:
            return false;
    }
}

bool ScheduleTimeline::can_run_on_queue_index(PassNode* pass, uint32_t queue_index) SKR_NOEXCEPT
{
    if (queue_index >= available_queues.size()) return false;
    
    const auto& caps = available_queues[queue_index];
    switch (pass->pass_type) {
        case EPassType::Render:   return caps.supports_graphics;
        case EPassType::Compute:  return caps.supports_compute;
        case EPassType::Copy:     return caps.supports_copy;
        case EPassType::Present:  return caps.supports_present;
        default:                  return false;
    }
}

bool ScheduleTimeline::has_graphics_resource_dependency(PassNode* pass) SKR_NOEXCEPT
{
    bool has_graphics_dep = false;
    
    // 检查纹理资源
    pass->foreach_textures([&](TextureNode* texture, TextureEdge* edge) {
        if (texture) {
            // 检查是否是渲染目标或深度缓冲
            if (texture->get_desc().descriptors & CGPU_RESOURCE_TYPE_RENDER_TARGET ||
                texture->get_desc().descriptors & CGPU_RESOURCE_TYPE_DEPTH_STENCIL) {
                has_graphics_dep = true;
            }
        }
    });
    
    return has_graphics_dep;
}

void ScheduleTimeline::assign_passes_to_queues(RenderGraph* graph) SKR_NOEXCEPT
{
    // 初始化队列调度结果
    schedule_result.queue_schedules.clear();
    schedule_result.queue_schedules.reserve(available_queues.size());
    for (uint32_t i = 0; i < available_queues.size(); ++i) {
        const auto& caps = available_queues[i];
        auto queue_type = caps.supports_graphics ? ERenderGraphQueueType::Graphics :
                         caps.supports_compute ? ERenderGraphQueueType::AsyncCompute : ERenderGraphQueueType::Copy;
        schedule_result.queue_schedules.emplace(QueueScheduleInfo{queue_type, i});
    }
    
    // 拓扑排序并分配Pass
    auto sorted_passes = topological_sort_passes(get_passes(graph));
    for (auto* pass : sorted_passes) {
        uint32_t queue_idx = select_best_queue_for_pass(pass);
        schedule_result.queue_schedules[queue_idx].scheduled_passes.add(pass);
        pass_to_queue[pass] = queue_idx;
        schedule_result.pass_queue_assignments[pass] = queue_idx;
    }
}

skr::Vector<PassNode*> ScheduleTimeline::topological_sort_passes(const skr::Vector<PassNode*>& passes) SKR_NOEXCEPT
{
    skr::Vector<PassNode*> sorted_passes;
    skr::FlatHashMap<PassNode*, uint32_t> in_degree;
    skr::Vector<PassNode*> queue;
    
    // 计算入度并收集起始节点
    for (auto* pass : passes) {
        uint32_t degree = dependency_info[pass].dependencies.size();
        in_degree[pass] = degree;
        if (degree == 0) queue.add(pass);
    }
    
    // 拓扑排序
    while (!queue.is_empty()) {
        auto* pass = queue.back();
        queue.pop_back();
        sorted_passes.add(pass);
        
        for (auto* dependent : dependency_info[pass].dependents) {
            if (--in_degree[dependent] == 0) {
                queue.add(dependent);
            }
        }
    }
    
    return sorted_passes;
}


uint32_t ScheduleTimeline::select_best_queue_for_pass(PassNode* pass) SKR_NOEXCEPT
{
    auto preferred_type = pass_preferred_queue[pass];
    
    // 根据偏好类型选择队列
    switch (preferred_type) {
        case ERenderGraphQueueType::AsyncCompute:
            if (!compute_queue_indices.is_empty()) {
                return find_least_loaded_compute_queue();
            }
            break;
        case ERenderGraphQueueType::Copy:
            if (!copy_queue_indices.is_empty()) {
                return copy_queue_indices[0];
            }
            break;
        default:
            break;
    }
    
    // 默认回退到Graphics队列
    return graphics_queue_index;
}


uint32_t ScheduleTimeline::find_least_loaded_compute_queue() SKR_NOEXCEPT
{
    if (compute_queue_indices.is_empty()) {
        return graphics_queue_index; // 回退到Graphics队列
    }
    
    // 找到负载最小的计算队列
    uint32_t best_queue = compute_queue_indices[0];
    size_t min_load = SIZE_MAX;
    
    for (uint32_t queue_idx : compute_queue_indices) {
        if (queue_idx < schedule_result.queue_schedules.size()) {
            size_t current_load = schedule_result.queue_schedules[queue_idx].scheduled_passes.size();
            if (current_load < min_load) {
                min_load = current_load;
                best_queue = queue_idx;
            }
        }
    }
    
    return best_queue;
}

bool ScheduleTimeline::is_compute_queue(uint32_t queue_index) const SKR_NOEXCEPT
{
    for (uint32_t compute_idx : compute_queue_indices) {
        if (compute_idx == queue_index) {
            return true;
        }
    }
    return false;
}

void ScheduleTimeline::calculate_sync_requirements(RenderGraph* graph) SKR_NOEXCEPT
{
    schedule_result.sync_requirements.clear();
    
    // 收集跨队列依赖并直接生成同步需求
    skr::FlatHashMap<uint64_t, SyncRequirement> sync_map;
    
    for (auto& [pass, queue] : pass_to_queue) {
        for (auto* dep : dependency_info[pass].dependencies) {
            auto dep_queue = pass_to_queue[dep];
            if (dep_queue != queue) {
                uint64_t key = ((uint64_t)dep_queue << 32) | queue;
                auto& requirement = sync_map[key];
                requirement.signal_queue_index = dep_queue;
                requirement.wait_queue_index = queue;
                requirement.signal_after_pass = dep;
                requirement.wait_before_pass = pass;
                requirement.fence = nullptr;
            }
        }
    }
    
    // 转换为vector
    schedule_result.sync_requirements.reserve(sync_map.size());
    for (auto& [key, requirement] : sync_map) {
        schedule_result.sync_requirements.add(requirement);
    }
}


void ScheduleTimeline::allocate_fences(RenderGraph* graph) SKR_NOEXCEPT
{
    auto device = graph->get_backend_device();
    if (!device) {
        SKR_LOG_WARN(u8"ScheduleTimeline: No backend device available for fence allocation");
        return;
    }
    
    // 为每个同步需求分配CGPUFence对象
    for (auto& requirement : schedule_result.sync_requirements) {
        if (!requirement.fence) {
            requirement.fence = cgpu_create_fence(device);
            
            SKR_LOG_DEBUG(u8"ScheduleTimeline: Allocated fence for sync between queue %d->%d (pass %p->%p)", 
                         requirement.signal_queue_index, requirement.wait_queue_index,
                         requirement.signal_after_pass, requirement.wait_before_pass);
        }
    }
    
    SKR_LOG_DEBUG(u8"ScheduleTimeline: Allocated %d fences for cross-queue synchronization", 
                  (int)schedule_result.sync_requirements.size());
}

void ScheduleTimeline::dump_timeline_result(const char8_t* title) const SKR_NOEXCEPT
{
    SKR_LOG_INFO(u8"═══════════════════════════════════════");
    SKR_LOG_INFO(u8"%s", title);
    SKR_LOG_INFO(u8"═══════════════════════════════════════");
    
    // 打印队列调度信息
    SKR_LOG_INFO(u8"📅 Queue Schedules (%d queues):", (int)schedule_result.queue_schedules.size());
    for (size_t i = 0; i < schedule_result.queue_schedules.size(); ++i) {
        const auto& queue_schedule = schedule_result.queue_schedules[i];
        const char8_t* queue_name = get_queue_type_name(queue_schedule.queue_type);
        
        // 为多队列类型添加索引标识
        skr::String queue_display_name;
        if (queue_schedule.queue_type == ERenderGraphQueueType::AsyncCompute && compute_queue_indices.size() > 1) {
            uint32_t index = compute_queue_indices.find(i).index();
            queue_display_name = skr::format(u8"{}#{}", queue_name, index);
        } else {
            queue_display_name = skr::String(queue_name);
        }
        
        SKR_LOG_INFO(u8"  [%zu] %s Queue (index=%d, %d passes):", 
                     i, queue_display_name.c_str(), queue_schedule.queue_index, 
                     (int)queue_schedule.scheduled_passes.size());
        
        for (size_t j = 0; j < queue_schedule.scheduled_passes.size(); ++j) {
            auto* pass = queue_schedule.scheduled_passes[j];
            const char8_t* pass_type_name = u8"Unknown";
            
            switch (pass->pass_type) {
                case EPassType::Render: pass_type_name = u8"Render"; break;
                case EPassType::Compute: pass_type_name = u8"Compute"; break;
                case EPassType::Copy: pass_type_name = u8"Copy"; break;
                case EPassType::Present: pass_type_name = u8"Present"; break;
            }
            
            SKR_LOG_INFO(u8"    [%zu] %s Pass (name=%s)", j, pass_type_name, pass->get_name());
        }
    }
    
    // 打印同步需求
    SKR_LOG_INFO(u8"");
    SKR_LOG_INFO(u8"🔄 Sync Requirements (%d requirements):", (int)schedule_result.sync_requirements.size());
    
    if (schedule_result.sync_requirements.is_empty()) {
        SKR_LOG_INFO(u8"  ✅ No cross-queue synchronization needed");
    } else {
        for (size_t i = 0; i < schedule_result.sync_requirements.size(); ++i) {
            const auto& requirement = schedule_result.sync_requirements[i];
            
            const char8_t* signal_queue_name = u8"Unknown";
            const char8_t* wait_queue_name = u8"Unknown";
            
            if (requirement.signal_queue_index < schedule_result.queue_schedules.size()) {
                signal_queue_name = get_queue_type_name(schedule_result.queue_schedules[requirement.signal_queue_index].queue_type);
            }
            if (requirement.wait_queue_index < schedule_result.queue_schedules.size()) {
                wait_queue_name = get_queue_type_name(schedule_result.queue_schedules[requirement.wait_queue_index].queue_type);
            }
            
            SKR_LOG_INFO(u8"  [%zu] %s[%d] --signal--> %s[%d]", 
                         i, signal_queue_name, requirement.signal_queue_index,
                         wait_queue_name, requirement.wait_queue_index);
            SKR_LOG_INFO(u8"       Signal after: %s, Wait before: %s, Fence: %p",
                         requirement.signal_after_pass->get_name(), requirement.wait_before_pass->get_name(), requirement.fence);
        }
    }
    
    // 打印Pass映射统计
    SKR_LOG_INFO(u8"");
    SKR_LOG_INFO(u8"📊 Pass Assignment Summary:");
    
    uint32_t graphics_count = 0, compute_count = 0, copy_count = 0;
    for (const auto& [pass, queue_idx] : schedule_result.pass_queue_assignments) {
        if (queue_idx < schedule_result.queue_schedules.size()) {
            auto queue_type = schedule_result.queue_schedules[queue_idx].queue_type;
            switch (queue_type) {
                case ERenderGraphQueueType::Graphics: graphics_count++; break;
                case ERenderGraphQueueType::AsyncCompute: compute_count++; break;
                case ERenderGraphQueueType::Copy: copy_count++; break;
            }
        }
    }
    
    SKR_LOG_INFO(u8"  📈 Graphics Queue: %d passes", graphics_count);
    SKR_LOG_INFO(u8"  ⚡ AsyncCompute Queue: %d passes", compute_count);
    SKR_LOG_INFO(u8"  📁 Copy Queue: %d passes", copy_count);
    SKR_LOG_INFO(u8"  📝 Total Passes: %d", (int)schedule_result.pass_queue_assignments.size());
    
    SKR_LOG_INFO(u8"═══════════════════════════════════════");
}

} // namespace render_graph
} // namespace skr