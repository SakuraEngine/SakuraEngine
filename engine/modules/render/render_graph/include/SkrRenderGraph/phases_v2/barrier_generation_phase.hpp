#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "cross_queue_sync_analysis.hpp"
#include "memory_aliasing_phase.hpp"
#include "pass_info_analysis.hpp"
#include "schedule_reorder.hpp"

namespace skr {
namespace RG {

// 屏障类型
enum class EBarrierType : uint8_t
{
    ResourceTransition,     // 资源状态转换屏障
    CrossQueueSync,        // 跨队列同步屏障
    MemoryAliasing,        // 内存别名屏障
    ExecutionDependency    // 执行依赖屏障
};

// GPU屏障描述
struct GPUBarrier
{
    ResourceNode* resource = nullptr;       // 相关资源
    EBarrierType type;
    PassNode* source_pass = nullptr;        // 源Pass
    PassNode* target_pass = nullptr;        // 目标Pass
    uint32_t source_queue = UINT32_MAX;
    uint32_t target_queue = UINT32_MAX;
    
    union
    {
        struct {
            ECGPUResourceState before_state = CGPU_RESOURCE_STATE_UNDEFINED;
            ECGPUResourceState after_state = CGPU_RESOURCE_STATE_UNDEFINED;
            bool is_begin = false;
            bool is_end = false;
            uint32_t mip_level = 0;
            uint32_t array_level = 0;
            bool is_subresource = false;
        } transition;
        MemoryAliasTransition aliasing;
    };
    
    bool is_cross_queue() const { return source_queue != target_queue && source_queue != UINT32_MAX && target_queue != UINT32_MAX; }
    bool involves_memory_aliasing() const { return type == EBarrierType::MemoryAliasing; }
};

// 屏障批次
struct BarrierBatch
{
    StackVector<GPUBarrier> barriers;       // 批次中的屏障列表
    EBarrierType batch_type;                // 批次类型（批次中所有屏障应该是同一类型）
};

// 屏障生成结果
struct BarrierGenerationResult
{
    // 按Pass组织的屏障批次 - 主要的使用接口
    StackMap<PassNode*, StackVector<BarrierBatch>> pass_barrier_batches;
    
    // 统计信息
    uint32_t total_resource_barriers = 0;
    uint32_t total_sync_barriers = 0;
    uint32_t total_aliasing_barriers = 0;
    uint32_t total_execution_barriers = 0;
    uint32_t optimized_away_barriers = 0;   // 被优化掉的屏障数
    
    // 性能估算
    float estimated_barrier_cost = 0.0f;    // 估算的屏障性能开销
};

// 屏障生成配置
struct BarrierGenerationConfig
{
    
};

// 屏障生成Phase（整合SSIS和内存别名结果）
class SKR_RENDER_GRAPH_API BarrierGenerationPhase : public IRenderGraphPhase
{
public:
    BarrierGenerationPhase(
        const CrossQueueSyncAnalysis& sync_analysis,
        const MemoryAliasingPhase& aliasing_phase,
        const PassInfoAnalysis& pass_info_analysis,
        const ExecutionReorderPhase& reorder_phase,
        const BarrierGenerationConfig& config = {});
    ~BarrierGenerationPhase() override = default;

    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    const BarrierGenerationResult& get_result() const { return barrier_result_; }
    const StackVector<BarrierBatch>& get_pass_barrier_batches(PassNode* pass) const;
    
    uint32_t get_total_barriers() const;
    uint32_t get_total_batches() const;
    float get_estimated_barrier_cost() const { return barrier_result_.estimated_barrier_cost; }
    uint32_t get_optimized_barriers_count() const { return barrier_result_.optimized_away_barriers; }
    BarrierBatch& get_or_create_barrier_batch(PassNode* pass, EBarrierType batch_type) SKR_NOEXCEPT;
    
    void dump_barrier_analysis() const SKR_NOEXCEPT;

private:
    // 核心屏障生成算法
    void generate_barriers(RenderGraph* graph) SKR_NOEXCEPT;
    void generate_cross_queue_sync_barriers(RenderGraph* graph) SKR_NOEXCEPT;
    void generate_memory_aliasing_barriers(RenderGraph* graph) SKR_NOEXCEPT;
    void generate_resource_transition_barriers(RenderGraph* graph) SKR_NOEXCEPT;
    void batch_barriers() SKR_NOEXCEPT;
    
    // 屏障创建辅助
    GPUBarrier create_cross_queue_barrier(const CrossQueueSyncPoint& sync_point) const SKR_NOEXCEPT;
    GPUBarrier create_aliasing_barrier(const MemoryAliasTransition& transition) const SKR_NOEXCEPT;
    
    // 状态转换计算
    ECGPUResourceState calculate_combined_read_state(const StackVector<ECGPUResourceState>& read_states) const SKR_NOEXCEPT;
    ECGPUResourceState get_resource_state_for_usage(ResourceNode* resource, PassNode* pass, bool is_write) const SKR_NOEXCEPT;
    
private:
    // 配置
    BarrierGenerationConfig config_;

    // 输入Phase引用
    const CrossQueueSyncAnalysis& sync_analysis_;
    const MemoryAliasingPhase& aliasing_phase_;
    const PassInfoAnalysis& pass_info_analysis_;
    const ExecutionReorderPhase& reorder_phase_;

    // 分析结果
    BarrierGenerationResult barrier_result_;
    
    // 工作数据
    StackMap<PassNode*, StackVector<BarrierBatch>> pass_barriers_;
};

} // namespace RG
} // namespace skr