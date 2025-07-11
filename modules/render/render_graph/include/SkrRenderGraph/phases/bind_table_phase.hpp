#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrGraphics/cgpux.hpp"

namespace skr {
namespace render_graph {

class RenderGraphFrameExecutor;
class ResourceAllocationPhase;

struct SKR_RENDER_GRAPH_API BindTablePhase : public IRenderGraphPhase
{
    void on_compile(RenderGraph* graph) SKR_NOEXCEPT final;
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

    // Bind table allocation and update for a specific pass
    CGPUXBindTableId alloc_update_pass_bind_table(RenderGraph* graph, RenderGraphFrameExecutor& executor, 
                                                  PassNode* pass, CGPURootSignatureId root_sig) SKR_NOEXCEPT;

private:
    ResourceAllocationPhase* resource_allocation_phase = nullptr;
    void find_resource_allocation_phase(RenderGraph* graph) SKR_NOEXCEPT;
};

} // namespace render_graph
} // namespace skr