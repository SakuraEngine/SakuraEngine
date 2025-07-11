#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrContainers/hashmap.hpp"

namespace skr {
namespace render_graph {

class RenderGraphFrameExecutor;

struct SKR_RENDER_GRAPH_API ResourceAllocationPhase : public IRenderGraphPhase
{
    void on_compile(RenderGraph* graph) SKR_NOEXCEPT final;
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

    // Resource resolution methods
    CGPUTextureId resolve_texture(RenderGraph* graph, RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT;
    CGPUTextureId try_aliasing_allocate(RenderGraph* graph, RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT;
    CGPUBufferId resolve_buffer(RenderGraph* graph, RenderGraphFrameExecutor& executor, const BufferNode& node) SKR_NOEXCEPT;

private:
    // Cache resolved resources to avoid duplicate allocation
    skr::FlatHashMap<ResourceHandle, CGPUTextureId> resolved_textures_cache;
    skr::FlatHashMap<ResourceHandle, CGPUBufferId> resolved_buffers_cache;
};

} // namespace render_graph
} // namespace skr