#include "SkrRenderGraph/frontend/render_graph.hpp"

namespace skr {
namespace render_graph {

IRenderGraphPhase::~IRenderGraphPhase() SKR_NOEXCEPT
{

}

void IRenderGraphPhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{

}

void IRenderGraphPhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{

}

uint32_t IRenderGraphPhase::on_collect_texture_garbage(RenderGraph* graph, uint64_t critical_frame, uint32_t with_tags, uint32_t without_flags) SKR_NOEXCEPT
{
    return 0;
}

uint32_t IRenderGraphPhase::on_collect_buffer_garbage(RenderGraph* graph, uint64_t critical_frame, uint32_t with_tags, uint32_t without_flags) SKR_NOEXCEPT
{
    return 0;
}

void IRenderGraphPhase::on_initialize(RenderGraph* graph) SKR_NOEXCEPT
{

}

void IRenderGraphPhase::on_finalize(RenderGraph* graph) SKR_NOEXCEPT
{

}

skr::vector<ResourceNode*>& IRenderGraphPhase::get_resources(RenderGraph* graph) SKR_NOEXCEPT
{
    return graph->resources;
}

skr::vector<PassNode*>& IRenderGraphPhase::get_passes(RenderGraph* graph) SKR_NOEXCEPT
{
    return graph->passes;
}

} // namespace render_graph
} // namespace skr