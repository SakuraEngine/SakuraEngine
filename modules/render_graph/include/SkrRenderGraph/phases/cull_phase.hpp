#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"

namespace skr {
namespace render_graph {

struct SKR_RENDER_GRAPH_API CullPhase : public IRenderGraphPhase
{
    void on_compile(RenderGraph* graph) SKR_NOEXCEPT final;
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

    skr::Array<PassNode*> culled_passes;
    skr::Array<ResourceNode*> culled_resources;
};

} // namespace render_graph
} // namespace skr