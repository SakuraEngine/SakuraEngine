#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "barrier_generation_phase.hpp"
#include "cross_queue_sync_analysis.hpp"
#include "bind_table_phase.hpp"
#include "queue_schedule.hpp"

namespace skr {
namespace RG {

// Command recording phase configuration
struct CommandRecordingConfig {
    bool enable_debug_markers = true;
    bool enable_profiler_events = true;
    bool enable_debug_output = false;
};

// Command recording phase result
struct CommandRecordingResult {
    uint32_t total_passes_executed = 0;
    uint32_t total_barriers_inserted = 0;
    uint32_t total_sync_points_processed = 0;
    float total_execution_time_ms = 0.0f;
};

// Command recording phase - the final execution layer
class SKR_RENDER_GRAPH_API PassExecutionPhase : public IRenderGraphPhase {
public:
    PassExecutionPhase(
        const QueueSchedule& queue_schedule,
        const ExecutionReorderPhase& reorder_phase,
        const CrossQueueSyncAnalysis& sync_analysis,
        const BarrierGenerationPhase& barrier_generation_phase,
        const ResourceAllocationPhase& resource_allocation_phase,
        const BindTablePhase& bind_table_phase,
        const CommandRecordingConfig& config = {});
    ~PassExecutionPhase() override = default;
    
    // IRenderGraphPhase interface
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;
    
    // Results access
    const CommandRecordingResult& get_result() const { return recording_result_; }
    
private:
    // Core execution workflow
    void execute_scheduled_passes(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT;
    
    // Pass execution by type
    void execute_render_pass(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderPassNode* pass) SKR_NOEXCEPT;
    void execute_compute_pass(RenderGraph* graph, RenderGraphFrameExecutor* executor, ComputePassNode* pass) SKR_NOEXCEPT;
    void execute_copy_pass(RenderGraph* graph, RenderGraphFrameExecutor* executor, CopyPassNode* pass) SKR_NOEXCEPT;
    void execute_present_pass(RenderGraph* graph, RenderGraphFrameExecutor* executor, PresentPassNode* pass) SKR_NOEXCEPT;
    
    // Barrier and sync management
    void insert_pass_barriers(RenderGraphFrameExecutor* executor, PassNode* pass) SKR_NOEXCEPT;
    void process_sync_points(RenderGraphFrameExecutor* executor, PassNode* pass) SKR_NOEXCEPT;
    
    // Utility functions
    void begin_debug_marker(RenderGraphFrameExecutor* executor, PassNode* pass) SKR_NOEXCEPT;
    void end_debug_marker(RenderGraphFrameExecutor* executor) SKR_NOEXCEPT;
    
private:
    // Configuration
    CommandRecordingConfig config_;
    
    // Input phase references
    const QueueSchedule& queue_schedule_;
    const ExecutionReorderPhase& reorder_phase_;
    const CrossQueueSyncAnalysis& sync_analysis_;
    const BarrierGenerationPhase& barrier_generation_phase_;
    const ResourceAllocationPhase& resource_allocation_phase_;
    const BindTablePhase& bind_table_phase_;
    
    // Results
    CommandRecordingResult recording_result_;
};

} // namespace RG
} // namespace skr