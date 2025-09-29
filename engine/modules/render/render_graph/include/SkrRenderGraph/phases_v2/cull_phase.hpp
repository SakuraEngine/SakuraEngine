#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/stack_allocator.hpp"

namespace skr {
namespace RG {

struct SKR_RENDER_GRAPH_API CullPhase : public IRenderGraphPhase
{
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

    StackVector<PassNode*> culled_passes;
    StackVector<ResourceNode*> culled_resources;
};

} // namespace RG
} // namespace skr