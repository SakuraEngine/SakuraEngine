#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "SkrContainersDef/array.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrContainersDef/bitset.hpp"

namespace skr {
namespace render_graph {

// 队列类型定义
enum class ERenderGraphQueueType : uint8_t 
{
    Graphics = 0,      // 主图形队列
    AsyncCompute = 1,  // 异步计算队列
    Copy = 2,          // 专用拷贝队列
    Count = 3
};

// 队列能力描述
struct QueueCapabilities 
{
    bool supports_graphics = false;
    bool supports_compute = false;
    bool supports_copy = false;
    bool supports_present = false;
    uint32_t family_index = 0;
    CGPUQueueId queue_handle = nullptr;  // 队列句柄
};

// 同步需求 - ScheduleTimeline的调度输出
struct SyncRequirement 
{
    uint32_t signal_queue_index;    // 发信号的队列
    uint32_t wait_queue_index;      // 等待信号的队列
    PassNode* signal_after_pass;    // 在哪个Pass执行后发信号
    PassNode* wait_before_pass;     // 在哪个Pass执行前等待
    CGPUFenceId fence;              // 预分配的Fence对象
};

// 队列调度信息
struct QueueScheduleInfo 
{
    ERenderGraphQueueType queue_type;
    uint32_t queue_index;
    skr::Vector<PassNode*> scheduled_passes;    // 调度的Pass序列
};

// ScheduleTimeline的输出结果
struct TimelineScheduleResult 
{
    skr::Vector<QueueScheduleInfo> queue_schedules;    // 各队列的调度信息
    skr::Vector<SyncRequirement> sync_requirements;    // 同步需求
    skr::FlatHashMap<PassNode*, uint32_t> pass_queue_assignments; // Pass到队列的映射
};

// Pass依赖信息
struct PassDependencyInfo 
{
    skr::Vector<PassNode*> dependencies;            // 依赖的Pass列表
    skr::Vector<PassNode*> dependents;              // 被依赖的Pass列表
    skr::Bitset<64> queue_affinities;               // 可运行的队列（动态大小，最大64个队列）
    bool has_cross_queue_dependency = false;        // 是否有跨队列依赖
};

// 跨队列依赖
struct CrossQueueDependency 
{
    PassNode* from_pass;
    uint32_t from_queue;
    PassNode* to_pass;
    uint32_t to_queue;
};

// Timeline Phase 配置
struct ScheduleTimelineConfig 
{
    bool enable_async_compute = true;       // 启用异步计算
    bool enable_copy_queue = true;          // 启用拷贝队列
    uint32_t max_async_compute_queues = 4;  // 最大异步计算队列数量
    uint32_t max_copy_queues = 2;           // 最大拷贝队列数量
    uint32_t max_sync_points = 64;          // 最大同步点数量
};

// Timeline Phase - 负责多队列调度
class SKR_RENDER_GRAPH_API ScheduleTimeline : public IRenderGraphPhase 
{
public:
    ScheduleTimeline(const ScheduleTimelineConfig& config = {});
    ~ScheduleTimeline() override;

    // IRenderGraphPhase 接口
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;
    void on_initialize(RenderGraph* graph) SKR_NOEXCEPT override;
    void on_finalize(RenderGraph* graph) SKR_NOEXCEPT override;

    // 获取调度结果
    const TimelineScheduleResult& get_schedule_result() const { return schedule_result; }
    
    // 调试接口
    void dump_timeline_result(const char8_t* title = u8"Timeline Schedule Result") const SKR_NOEXCEPT;

private:
    // 初始化阶段
    void query_queue_capabilities(RenderGraph* graph) SKR_NOEXCEPT;
    void clear_frame_data() SKR_NOEXCEPT;

    // 编译阶段
    void analyze_dependencies(RenderGraph* graph) SKR_NOEXCEPT;
    void classify_passes(RenderGraph* graph) SKR_NOEXCEPT;
    void assign_passes_to_queues(RenderGraph* graph) SKR_NOEXCEPT;
    void calculate_sync_requirements(RenderGraph* graph) SKR_NOEXCEPT;
    void allocate_fences(RenderGraph* graph) SKR_NOEXCEPT;

    // Pass分类和调度
    ERenderGraphQueueType classify_pass(PassNode* pass) SKR_NOEXCEPT;
    bool can_run_on_queue(PassNode* pass, ERenderGraphQueueType queue) SKR_NOEXCEPT;
    bool has_graphics_resource_dependency(PassNode* pass) SKR_NOEXCEPT;


private:
    ScheduleTimelineConfig config;
    
    // 动态队列信息 - 支持多计算队列
    skr::Vector<QueueCapabilities> available_queues;  // 所有可用队列
    uint32_t graphics_queue_index = UINT32_MAX;       // Graphics队列索引
    skr::Vector<uint32_t> compute_queue_indices;      // 计算队列索引列表
    skr::Vector<uint32_t> copy_queue_indices;         // 拷贝队列索引列表
    
    // Pass调度信息 - 每帧重建
    skr::FlatHashMap<PassNode*, uint32_t> pass_to_queue;
    skr::FlatHashMap<PassNode*, PassDependencyInfo> dependency_info;
    skr::FlatHashMap<PassNode*, ERenderGraphQueueType> pass_preferred_queue;
    
    // 调度结果输出
    TimelineScheduleResult schedule_result;
    
    // 调度子功能
    skr::Vector<PassNode*> topological_sort_passes(const skr::Vector<PassNode*>& passes) SKR_NOEXCEPT;
    uint32_t select_best_queue_for_pass(PassNode* pass) SKR_NOEXCEPT;
    
    // 辅助方法
    uint32_t find_least_loaded_compute_queue() SKR_NOEXCEPT;
    bool is_compute_queue(uint32_t queue_index) const SKR_NOEXCEPT;
    bool can_run_on_queue_index(PassNode* pass, uint32_t queue_index) SKR_NOEXCEPT;
};

// 辅助函数
inline const char8_t* get_queue_type_name(ERenderGraphQueueType type) 
{
    switch (type) {
        case ERenderGraphQueueType::Graphics: return u8"Graphics";
        case ERenderGraphQueueType::AsyncCompute: return u8"AsyncCompute";
        case ERenderGraphQueueType::Copy: return u8"Copy";
        default: return u8"Unknown";
    }
}

} // namespace render_graph
} // namespace skr