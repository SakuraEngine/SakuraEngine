#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/set.hpp"
#include "SkrGraphics/api.h"

namespace skr {
namespace render_graph {

class RenderGraphFrameExecutor;
class ResourceAllocationPhase;

// TODO: optimize stack allocation
static constexpr size_t stack_vector_fixed_count = 8;
template <typename T>
using stack_vector = skr::InlineVector<T, stack_vector_fixed_count>;
template <typename T>
using stack_set = skr::Set<T>;

struct SKR_RENDER_GRAPH_API BarrierCalculationPhase : public IRenderGraphPhase
{
    BarrierCalculationPhase(ResourceAllocationPhase* allocation_phase);

    void on_compile(RenderGraph* graph) SKR_NOEXCEPT final;
    void on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT final;

    // Barrier calculation for a specific pass
    void calculate_barriers(RenderGraph* graph, RenderGraphFrameExecutor& executor, PassNode* pass,
                           stack_vector<CGPUTextureBarrier>& tex_barriers, 
                           stack_vector<std::pair<TextureHandle, CGPUTextureId>>& resolved_textures,
                           stack_vector<CGPUBufferBarrier>& buf_barriers, 
                           stack_vector<std::pair<BufferHandle, CGPUBufferId>>& resolved_buffers) SKR_NOEXCEPT;

private:
    ResourceAllocationPhase* resource_allocation_phase = nullptr;
};

} // namespace render_graph
} // namespace skr