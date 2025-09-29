#include "SkrRenderGraph/phases_v2/barrier_generation_phase.hpp"
#include "SkrRenderGraph/phases_v2/cross_queue_sync_analysis.hpp"
#include "SkrRenderGraph/phases_v2/memory_aliasing_phase.hpp"
#include "SkrRenderGraph/phases_v2/detail/hardware_constriants.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrCore/log.hpp"
#include "SkrContainersDef/set.hpp"
#include <algorithm>

#define BARRIER_GENERATION_LOG(...)

namespace skr
{
namespace RG
{

inline static float estimate_barrier_cost(const GPUBarrier& barrier) SKR_NOEXCEPT
{
    using namespace HardwareConstraints;
    using namespace BarrierCosts;

    // 使用硬件约束常量估算屏障成本
    switch (barrier.type)
    {
    case EBarrierType::CrossQueueSync:
        return CROSS_QUEUE_SYNC; // 35.0μs - 跨队列同步成本最高

    case EBarrierType::MemoryAliasing:
        return L2_CACHE_FLUSH; // 15.0μs - 内存别名需要缓存刷新

    case EBarrierType::ResourceTransition: {
        // 根据状态转换类型估算成本
        bool is_format_change = (barrier.transition.before_state & CGPU_RESOURCE_STATE_RENDER_TARGET) &&
            (barrier.transition.after_state & CGPU_RESOURCE_STATE_SHADER_RESOURCE);

        if (is_format_change)
            return FORMAT_CONVERSION; // 100.0μs - 格式转换
        else if (barrier.is_cross_queue())
            return L1_CACHE_FLUSH; // 7.5μs - 跨队列资源转换
        else
            return SIMPLE_BARRIER; // 2.5μs - 简单资源屏障
    }

    case EBarrierType::ExecutionDependency:
        return SIMPLE_BARRIER; // 2.5μs - 执行依赖

    default:
        return SIMPLE_BARRIER;
    }
}

inline static bool is_state_transition_supported_on_queue(uint32_t queue_index, ECGPUResourceState before_state, ECGPUResourceState after_state) SKR_NOEXCEPT
{
    // 简化的队列能力检查
    if (queue_index == 0) // 图形队列支持所有状态
        return true;

    // 计算队列不支持图形专用状态
    bool has_graphics_before = (before_state & (CGPU_RESOURCE_STATE_RENDER_TARGET | CGPU_RESOURCE_STATE_DEPTH_WRITE)) != 0;
    bool has_graphics_after = (after_state & (CGPU_RESOURCE_STATE_RENDER_TARGET | CGPU_RESOURCE_STATE_DEPTH_WRITE)) != 0;

    return !(has_graphics_before || has_graphics_after);
}

inline static bool can_use_split_barriers(uint32_t transmitting_queue, uint32_t receiving_queue, ECGPUResourceState before_state, ECGPUResourceState after_state) SKR_NOEXCEPT
{
    // 分离屏障要求两个队列都支持相关状态转换
    if (!is_state_transition_supported_on_queue(transmitting_queue, before_state, after_state) ||
        !is_state_transition_supported_on_queue(receiving_queue, before_state, after_state))
    {
        return false;
    }

    // 只有重量级的屏障才值得使用分离屏障优化
    // 创建临时屏障对象来估算成本
    GPUBarrier temp_barrier = {};
    temp_barrier.type = EBarrierType::ResourceTransition;
    temp_barrier.transition.before_state = before_state;
    temp_barrier.transition.after_state = after_state;
    temp_barrier.source_queue = transmitting_queue;
    temp_barrier.target_queue = receiving_queue;

    const float split_barrier_threshold = 10.f;
    float barrier_cost = estimate_barrier_cost(temp_barrier);
    return barrier_cost >= split_barrier_threshold;
}

inline static bool are_passes_adjacent_or_synchronized(const ExecutionReorderPhase& reorder_phase_, const CrossQueueSyncAnalysis& sync_analysis_, PassNode* source_pass, PassNode* target_pass) SKR_NOEXCEPT
{
    // 检查两个Pass是否紧挨着或者已经有同步屏障
    const uint32_t source_queue = sync_analysis_.get_pass_queue_index(source_pass);
    const uint32_t target_queue = sync_analysis_.get_pass_queue_index(target_pass);
    const auto same_queue = (source_queue == target_queue);

    // 1. 检查是否在同一队列上紧挨着执行
    if (same_queue)
    {
        const auto& timeline = reorder_phase_.get_optimized_timeline()[source_queue];
        const auto source_pass_index = timeline.find(source_pass).index();
        const auto target_pass_index = timeline.find(target_pass).index();
        if (source_pass_index == target_pass_index + 1 || source_pass_index == target_pass_index - 1)
        {
            return true;
        }
    }

    // 2. 检查是否已经有跨队列同步屏障
    const auto& ssis_result = sync_analysis_.get_ssis_result();
    for (const auto& sync_point : ssis_result.optimized_sync_points)
    {
        if (sync_point.producer_pass == source_pass && sync_point.consumer_pass == target_pass)
        {
            // 已经有专门的同步点，不需要分离屏障
            return true;
        }
    }

    return false;
}

BarrierGenerationPhase::BarrierGenerationPhase(
    const CrossQueueSyncAnalysis& sync_analysis,
    const MemoryAliasingPhase& aliasing_phase,
    const PassInfoAnalysis& pass_info_analysis,
    const ExecutionReorderPhase& reorder_phase,
    const BarrierGenerationConfig& config
)
    : config_(config)
    , sync_analysis_(sync_analysis)
    , aliasing_phase_(aliasing_phase)
    , pass_info_analysis_(pass_info_analysis)
    , reorder_phase_(reorder_phase)
{
}

void BarrierGenerationPhase::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("BarrierGenerationPhase");
    BARRIER_GENERATION_LOG(u8"BarrierGenerationPhase: Starting barrier generation");
    barrier_result_ = {};

    generate_barriers(graph);
    batch_barriers();

    BARRIER_GENERATION_LOG(u8"BarrierGenerationPhase: Generated {} barriers ({} sync, {} aliasing, {} transition, {} execution)", get_total_barriers(), barrier_result_.total_sync_barriers, barrier_result_.total_aliasing_barriers, barrier_result_.total_resource_barriers, barrier_result_.total_execution_barriers);
}

void BarrierGenerationPhase::generate_barriers(RenderGraph* graph) SKR_NOEXCEPT
{
    // 1. 从SSIS分析生成跨队列同步屏障
    generate_cross_queue_sync_barriers(graph);

    // 2. 从内存别名分析生成别名屏障
    generate_memory_aliasing_barriers(graph);

    // 3. 生成资源状态转换屏障
    generate_resource_transition_barriers(graph);

    // Count total barriers generated
    uint32_t total_barriers = 0;
    for (const auto& [pass, batches] : pass_barriers_)
    {
        for (const auto& batch : batches)
        {
            total_barriers += static_cast<uint32_t>(batch.barriers.size());
        }
    }
    BARRIER_GENERATION_LOG(u8"BarrierGenerationPhase: Generated {} preliminary barriers", total_barriers);
}

void BarrierGenerationPhase::generate_cross_queue_sync_barriers(RenderGraph* graph) SKR_NOEXCEPT
{
    const auto& ssis_result = sync_analysis_.get_ssis_result();

    for (const auto& sync_point : ssis_result.optimized_sync_points)
    {
        GPUBarrier barrier = create_cross_queue_barrier(sync_point);
        // Add barrier directly to pass_barriers_
        auto* execution_pass = barrier.source_pass; // Cross-queue sync barriers execute at source pass
        if (execution_pass)
        {
            auto& batch = get_or_create_barrier_batch(execution_pass, EBarrierType::CrossQueueSync);
            batch.barriers.add(barrier);
        }
    }

    BARRIER_GENERATION_LOG(u8"BarrierGenerationPhase: Generated {} cross-queue sync barriers", ssis_result.optimized_sync_points.size());
}

void BarrierGenerationPhase::generate_memory_aliasing_barriers(RenderGraph* graph) SKR_NOEXCEPT
{
    const auto& aliasing_result = aliasing_phase_.get_result();
    for (const auto& transition : aliasing_result.alias_transitions)
    {
        if (!transition.transition_pass || !transition.to_resource)
            continue;

        // 创建别名屏障
        GPUBarrier barrier = create_aliasing_barrier(transition);

        // Add barrier directly to pass_barriers_
        auto* execution_pass = transition.transition_pass;
        if (execution_pass)
        {
            auto& batch = get_or_create_barrier_batch(execution_pass, EBarrierType::MemoryAliasing);
            batch.barriers.add(barrier);
        }
    }

    BARRIER_GENERATION_LOG(u8"BarrierGenerationPhase: Generated %u aliasing barriers from pre-computed transitions", static_cast<uint32_t>(aliasing_result.alias_transitions.size()));
}

struct ResourceStateTracker
{
public:
    ResourceStateTracker(const ResourceAccessInfo& init)
    {
        auto init_state = init.resource->is_imported() ? init.resource->get_init_state() : init.resource_state;
        if (init.resource->get_type() == EObjectType::Buffer)
        {
            as_buffer = (BufferNode*)init.resource;
            states.resize(1, init_state);
        }
        else if (init.resource->get_type() == EObjectType::Texture)
        {
            as_texture = (TextureNode*)init.resource;
            const auto subresource_count = as_texture->get_desc().array_size * as_texture->get_desc().mip_levels;
            states.resize(subresource_count, init_state);
        }
    }

    void update(BarrierGenerationPhase& barrier_phase, const ExecutionReorderPhase& reorder_phase_, const CrossQueueSyncAnalysis& sync_analysis_, const ResourceAccessInfo& access)
    {
        last_pass = last_pass ? last_pass : access.pass;

        auto emplace_barrier = [&](GPUBarrier barrier) {
            bool should_use_split_barrier = false;
            if (barrier.source_pass && can_use_split_barriers(barrier.source_queue, barrier.target_queue, barrier.transition.before_state, barrier.transition.after_state))
            {
                // 进一步检查是否值得使用分离屏障
                if (!are_passes_adjacent_or_synchronized(reorder_phase_, sync_analysis_, barrier.source_pass, barrier.target_pass))
                {
                    should_use_split_barrier = true;
                }
            }
            if (should_use_split_barrier)
            {
                auto& begin_batch = barrier_phase.get_or_create_barrier_batch(barrier.target_pass, EBarrierType::ResourceTransition);
                barrier.transition.is_begin = true;
                barrier.transition.is_end = false;
                begin_batch.barriers.add(barrier);

                auto& end_batch = barrier_phase.get_or_create_barrier_batch(barrier.target_pass, EBarrierType::ResourceTransition);
                barrier.transition.is_begin = false;
                barrier.transition.is_end = true;
                begin_batch.barriers.add(barrier);
            }
            else
            {
                auto& batch = barrier_phase.get_or_create_barrier_batch(barrier.target_pass, EBarrierType::ResourceTransition);
                batch.barriers.add(barrier);
            }
        };

        GPUBarrier barrier = {};
        barrier.type = EBarrierType::ResourceTransition;
        barrier.resource = access.resource;
        barrier.source_pass = last_pass;
        barrier.target_pass = access.pass;
        barrier.source_queue = sync_analysis_.get_pass_queue_index(last_pass);
        barrier.target_queue = sync_analysis_.get_pass_queue_index(access.pass);
        if (access.resource->get_type() == EObjectType::Buffer)
        {
            barrier.transition.before_state = states[0];
            barrier.transition.after_state = access.resource_state;
            if (should_barrier(barrier.transition.before_state, barrier.transition.after_state, barrier.source_queue, barrier.target_queue))
            {
                emplace_barrier(barrier);
                states[0] = access.resource_state;
            }
        }
        else if (access.resource->get_type() == EObjectType::Texture)
        {
            for (uint32_t i = 0; i < access.array_count; i++)
            {
                for (uint32_t j = 0; j < access.mip_count; j++)
                {
                    const auto array_level = i + access.array_base;
                    const auto mip_level = j + access.mip_base;
                    const auto state_index = array_level * as_texture->get_desc().mip_levels + mip_level;
                    barrier.transition.is_subresource = true;
                    barrier.transition.array_level = array_level;
                    barrier.transition.mip_level = mip_level;
                    barrier.transition.before_state = states[state_index];
                    barrier.transition.after_state = access.resource_state;
                    if (should_barrier(barrier.transition.before_state, barrier.transition.after_state, barrier.source_queue, barrier.target_queue))
                    {
                        emplace_barrier(barrier);
                        states[state_index] = access.resource_state;
                    }
                }
            }
        }
        last_pass = access.pass;
    }

    bool should_barrier(ECGPUResourceState from, ECGPUResourceState to, uint32_t from_queue, uint32_t to_queue)
    {
        if (from_queue != to_queue)
            return true;
        if (from != to)
            return true;
        if (from == CGPU_RESOURCE_STATE_UNORDERED_ACCESS && to == CGPU_RESOURCE_STATE_UNORDERED_ACCESS)
            return true;
        return false;
    }

private:
    friend struct BarrierGenerationPhase;
    BufferNode* as_buffer = nullptr;
    TextureNode* as_texture = nullptr;
    PassNode* last_pass = nullptr;
    skr::InlineVector<ECGPUResourceState, 1> states;
};

void BarrierGenerationPhase::generate_resource_transition_barriers(RenderGraph* graph) SKR_NOEXCEPT
{
    const auto& all_resources = get_resources(graph);
    const auto& all_passes = get_passes(graph);
    const auto& dependency_analysis = sync_analysis_.get_dependency_analysis();
    StackMap<ResourceNode*, ResourceStateTracker> resource_trackers;
    resource_trackers.reserve(graph->get_resources().size());
    for (auto* pass : dependency_analysis.get_topological_order())
    {
        if (const auto* pass_info = pass_info_analysis_.get_pass_info(pass))
        {
            for (const auto& access : pass_info->resource_info.resource_accesses)
            {
                auto iter = resource_trackers.try_emplace(access.resource, access);
                if (access.resource->is_imported() || iter.already_exist())
                {
                    auto& [resource, tracker] = iter.ref();
                    tracker.update(*this, reorder_phase_, sync_analysis_, access);
                }
            }
        }
    }
    for (auto resource : graph->get_resources())
    {
        if (auto tracker = resource_trackers.find(resource))
        {
            for (auto t : resource->trackers)
            {
                t->last_state = tracker.value().states[0];
            }
        }
    }
    BARRIER_GENERATION_LOG(u8"BarrierGenerationPhase: Generated resource transitions for %zu resources", all_resources.size());
}

void BarrierGenerationPhase::batch_barriers() SKR_NOEXCEPT
{
    barrier_result_.pass_barrier_batches = pass_barriers_;
}

GPUBarrier BarrierGenerationPhase::create_cross_queue_barrier(const CrossQueueSyncPoint& sync_point) const SKR_NOEXCEPT
{
    GPUBarrier barrier = {};
    barrier.resource = sync_point.resource;
    barrier.type = EBarrierType::CrossQueueSync;
    barrier.source_pass = sync_point.producer_pass;
    barrier.target_pass = sync_point.consumer_pass;
    barrier.source_queue = sync_point.producer_queue_index;
    barrier.target_queue = sync_point.consumer_queue_index;
    return barrier;
}

GPUBarrier BarrierGenerationPhase::create_aliasing_barrier(const MemoryAliasTransition& transition) const SKR_NOEXCEPT
{
    GPUBarrier barrier = {};
    barrier.resource = transition.to_resource;
    barrier.type = EBarrierType::MemoryAliasing;
    barrier.source_pass = transition.source_pass;
    barrier.target_pass = transition.transition_pass;

    barrier.aliasing = transition;

    return barrier;
}

const StackVector<BarrierBatch>& BarrierGenerationPhase::get_pass_barrier_batches(PassNode* pass) const
{
    static const StackVector<BarrierBatch> empty_batches;

    auto it = barrier_result_.pass_barrier_batches.find(pass);
    return it ? it.value() : empty_batches;
}

uint32_t BarrierGenerationPhase::get_total_barriers() const
{
    uint32_t total = 0;
    for (const auto& [pass, batches] : barrier_result_.pass_barrier_batches)
    {
        for (const auto& batch : batches)
        {
            total += static_cast<uint32_t>(batch.barriers.size());
        }
    }
    return total;
}

uint32_t BarrierGenerationPhase::get_total_batches() const
{
    uint32_t total = 0;
    for (const auto& [pass, batches] : barrier_result_.pass_barrier_batches)
    {
        total += static_cast<uint32_t>(batches.size());
    }
    return total;
}

ECGPUResourceState BarrierGenerationPhase::calculate_combined_read_state(const StackVector<ECGPUResourceState>& read_states) const SKR_NOEXCEPT
{
    // 合并多个读取状态
    ECGPUResourceState combined = CGPU_RESOURCE_STATE_UNDEFINED;

    for (auto state : read_states)
    {
        combined = static_cast<ECGPUResourceState>(combined | state);
    }

    return combined;
}

ECGPUResourceState BarrierGenerationPhase::get_resource_state_for_usage(ResourceNode* resource, PassNode* pass, bool is_write) const SKR_NOEXCEPT
{
    // 使用PassInfoAnalysis的状态信息
    return pass_info_analysis_.get_resource_state(pass, resource);
}

BarrierBatch& BarrierGenerationPhase::get_or_create_barrier_batch(PassNode* pass, EBarrierType batch_type) SKR_NOEXCEPT
{
    // Get or create the vector of batches for this pass
    auto& pass_batches = pass_barriers_.try_add_default(pass).value();

    // Find existing batch of the same type
    for (auto& batch : pass_batches)
    {
        if (batch.batch_type == batch_type)
        {
            return batch;
        }
    }

    // Create new batch if not found
    pass_batches.add(BarrierBatch{});
    auto& new_batch = pass_batches.back();
    new_batch.batch_type = batch_type;
    return new_batch;
}

void BarrierGenerationPhase::dump_barrier_analysis() const SKR_NOEXCEPT
{
    SKR_LOG_INFO(u8"========== Barrier Generation Analysis ==========");
    SKR_LOG_INFO(u8"Total barriers: %u", get_total_barriers());
    SKR_LOG_INFO(u8"Total batches: %u", get_total_batches());
    SKR_LOG_INFO(u8"  - Resource transition barriers: %u", barrier_result_.total_resource_barriers);
    SKR_LOG_INFO(u8"  - Cross-queue sync barriers: %u", barrier_result_.total_sync_barriers);
    SKR_LOG_INFO(u8"  - Memory aliasing barriers: %u", barrier_result_.total_aliasing_barriers);
    SKR_LOG_INFO(u8"  - Execution dependency barriers: %u", barrier_result_.total_execution_barriers);
    SKR_LOG_INFO(u8"Optimized away barriers: %u", barrier_result_.optimized_away_barriers);
    SKR_LOG_INFO(u8"Estimated barrier cost: {:.2f}", barrier_result_.estimated_barrier_cost);

    SKR_LOG_INFO(u8"Pass barrier batch distribution:");
    for (const auto& [pass, batches] : barrier_result_.pass_barrier_batches)
    {
        uint32_t total_barriers_in_pass = 0;
        for (const auto& batch : batches)
        {
            total_barriers_in_pass += static_cast<uint32_t>(batch.barriers.size());
        }
        SKR_LOG_INFO(u8"  Pass %s: %zu batches, %u barriers", pass->get_name(), batches.size(), total_barriers_in_pass);
    }

    SKR_LOG_INFO(u8"===============================================");
}

} // namespace RG
} // namespace skr