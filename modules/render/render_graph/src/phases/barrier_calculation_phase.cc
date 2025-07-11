#include "SkrRenderGraph/phases/barrier_calculation_phase.hpp"
#include "SkrRenderGraph/phases/resource_allocation_phase.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

BarrierCalculationPhase::BarrierCalculationPhase(ResourceAllocationPhase* allocation_phase)
    : resource_allocation_phase(allocation_phase)
{

}

void BarrierCalculationPhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{

}

void BarrierCalculationPhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{

}

void BarrierCalculationPhase::calculate_barriers(RenderGraph* graph, RenderGraphFrameExecutor& executor, PassNode* pass,
                                                stack_vector<CGPUTextureBarrier>& tex_barriers, 
                                                stack_vector<std::pair<TextureHandle, CGPUTextureId>>& resolved_textures,
                                                stack_vector<CGPUBufferBarrier>& buf_barriers, 
                                                stack_vector<std::pair<BufferHandle, CGPUBufferId>>& resolved_buffers) SKR_NOEXCEPT
{
    SkrZoneScopedN("CalculateBarriers");
    
    if (!resource_allocation_phase) return;
    
    stack_set<TextureHandle> tex_resolve_set;
    stack_set<BufferHandle> buf_resolve_set;

    pass->foreach_textures([&](TextureNode* texture, TextureEdge* edge) {
        auto tex_resolved = resource_allocation_phase->resolve_texture(executor, *texture);
        if (!tex_resolve_set.contains(texture->get_handle()))
        {
            resolved_textures.emplace(texture->get_handle(), tex_resolved);
            tex_resolve_set.add(texture->get_handle());

            const auto current_state = graph->get_lastest_state(texture, pass);
            const auto dst_state = edge->requested_state;
            if (current_state == dst_state) return;

            CGPUTextureBarrier barrier = {};
            barrier.src_state = current_state;
            barrier.dst_state = dst_state;
            barrier.texture = tex_resolved;

            tex_barriers.emplace(barrier);
        }
    });
    
    pass->foreach_buffers([&](BufferNode* buffer, BufferEdge* edge) {
        auto buf_resolved = resource_allocation_phase->resolve_buffer(executor, *buffer);
        if (!buf_resolve_set.find(buffer->get_handle()))
        {
            resolved_buffers.emplace(buffer->get_handle(), buf_resolved);
            buf_resolve_set.add(buffer->get_handle());

            const auto current_state = graph->get_lastest_state(buffer, pass);
            const auto dst_state = edge->requested_state;
            if (current_state == dst_state) return;

            CGPUBufferBarrier barrier = {};
            barrier.src_state = current_state;
            barrier.dst_state = dst_state;
            barrier.buffer = buf_resolved;
            buf_barriers.emplace(barrier);
        }
    });
}

} // namespace render_graph
} // namespace skr