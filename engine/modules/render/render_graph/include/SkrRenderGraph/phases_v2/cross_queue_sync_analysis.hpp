#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "pass_dependency_analysis.hpp"
#include "queue_schedule.hpp"
#include "schedule_reorder.hpp"

namespace skr {
namespace RG {

// 同步点类型
enum class ESyncPointType : uint8_t
{
    Signal,      // 信号量发送
    Wait,        // 信号量等待
    Barrier      // 内存屏障
};

// 跨队列同步点
struct CrossQueueSyncPoint
{
    ESyncPointType type;
    uint32_t producer_queue_index;    // 生产者队列索引
    uint32_t consumer_queue_index;    // 消费者队列索引
    PassNode* producer_pass;          // 生产者Pass
    PassNode* consumer_pass;          // 消费者Pass
    ResourceNode* resource;           // 相关资源
    ECGPUResourceState from_state;    // 源状态
    ECGPUResourceState to_state;      // 目标状态
    uint32_t sync_value;              // 同步值（用于时间线信号量）
};

// SSIS (Sufficient Synchronization Index Set) 分析结果
struct SSISAnalysisResult
{
    // 原始同步需求（所有资源冲突点）
    StackVector<CrossQueueSyncPoint> raw_sync_points;
    
    // 优化后的同步点集合（SSIS算法结果）
    StackVector<CrossQueueSyncPoint> optimized_sync_points;
    
    // 统计信息
    uint32_t total_raw_syncs = 0;
    uint32_t total_optimized_syncs = 0;
    uint32_t sync_reduction_count = 0;
    float optimization_ratio = 0.0f;
};

// SSIS分析配置
struct CrossQueueSyncConfig
{
    bool enable_ssis_optimization = true;    // 启用SSIS优化
    uint32_t max_sync_distance = 16;         // 最大同步距离（Pass数量）
    bool enable_debug_output = false;        // 启用调试输出
};

// 跨队列同步分析阶段 - 实现SSIS算法
class SKR_RENDER_GRAPH_API CrossQueueSyncAnalysis : public IRenderGraphPhase
{
public:
    CrossQueueSyncAnalysis(
        const PassDependencyAnalysis& dependency_analysis,
        const QueueSchedule& queue_schedule,
        const CrossQueueSyncConfig& config = {});
    ~CrossQueueSyncAnalysis() override = default;

    // IRenderGraphPhase 接口
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    // 获取分析结果
    uint32_t get_pass_queue_index(PassNode* pass) const SKR_NOEXCEPT;
    const SSISAnalysisResult& get_ssis_result() const { return ssis_result_; }
    const StackVector<CrossQueueSyncPoint>& get_optimized_sync_points() const { return ssis_result_.optimized_sync_points; }
    const PassDependencyAnalysis& get_dependency_analysis() const { return dependency_analysis_; }

    // 调试接口
    void dump_ssis_analysis() const SKR_NOEXCEPT;
    void dump_sync_points() const SKR_NOEXCEPT;

private:
    void apply_ssis_optimization(RenderGraph* graph) SKR_NOEXCEPT;
    void calculate_optimization_statistics() SKR_NOEXCEPT;
    
    // SSIS算法辅助结构
    struct SyncCoverage
    {
        PassNode* node_to_sync_with;
        uint32_t node_index;
        StackVector<uint32_t> synced_queue_indices;
    };
    
private:
    // 配置
    CrossQueueSyncConfig config_;

    // 输入Phase引用
    const PassDependencyAnalysis& dependency_analysis_;
    const QueueSchedule& queue_schedule_;

    // 分析结果
    SSISAnalysisResult ssis_result_;

    // 工作数据（移除了 cross_queue_dependencies_，现在由 PassDependencyAnalysis 生成同步点）
    
    // SSIS算法数据
    static constexpr uint32_t InvalidSyncIndex = UINT32_MAX;
    StackMap<PassNode*, StackVector<uint32_t>> pass_ssis_;  // 每个Pass的SSIS数组
    StackMap<PassNode*, uint32_t> pass_local_to_queue_indices_; // Pass在队列内的本地执行索引
    StackMap<PassNode*, StackVector<PassNode*>> pass_nodes_to_sync_with_; // 每个Pass需要同步的节点（会被优化）
    uint32_t total_queue_count_ = 0;
};

} // namespace RG
} // namespace skr