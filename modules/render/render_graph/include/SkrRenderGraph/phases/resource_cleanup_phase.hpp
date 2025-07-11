#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"

namespace skr {
namespace render_graph {

struct SKR_RENDER_GRAPH_API ResourceCleanupPhase : public IRenderGraphPhase
{
    void on_compile(RenderGraph* graph) SKR_NOEXCEPT final;
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

private:
    void deallocate_resources(RenderGraph* graph, PassNode* pass) SKR_NOEXCEPT;
};

} // namespace render_graph
} // namespace skr