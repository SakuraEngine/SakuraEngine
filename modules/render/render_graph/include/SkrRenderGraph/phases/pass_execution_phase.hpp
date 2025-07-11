#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/phases/barrier_calculation_phase.hpp"

namespace skr {
namespace render_graph {

class BarrierCalculationPhase;
class BindTablePhase;

struct SKR_RENDER_GRAPH_API PassExecutionPhase : public IRenderGraphPhase
{
    void on_compile(RenderGraph* graph) SKR_NOEXCEPT final;
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

private:
    void execute_compute_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, ComputePassNode* pass) SKR_NOEXCEPT;
    void execute_render_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, RenderPassNode* pass) SKR_NOEXCEPT;
    void execute_copy_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, CopyPassNode* pass) SKR_NOEXCEPT;
    void execute_present_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, PresentPassNode* pass) SKR_NOEXCEPT;
    
    void find_dependent_phases(RenderGraph* graph) SKR_NOEXCEPT;

    BarrierCalculationPhase* barrier_calculation_phase = nullptr;
    BindTablePhase* bind_table_phase = nullptr;
};

} // namespace render_graph
} // namespace skr