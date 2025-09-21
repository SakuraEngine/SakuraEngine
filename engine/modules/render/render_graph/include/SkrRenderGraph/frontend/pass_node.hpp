#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/resource_edge.hpp"

namespace skr {
namespace render_graph
{
    template<typename T, uint32_t N = 4>
    using graph_edges_vector = skr::InlineVector<T, N>;  
}
}

namespace skr
{
namespace render_graph
{
class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class PassExecutionPhase;

    SKR_RENDER_GRAPH_API const bool before(const PassNode* other) const;
    SKR_RENDER_GRAPH_API const bool after(const PassNode* other) const;
    SKR_RENDER_GRAPH_API const PassHandle get_handle() const;

    SKR_RENDER_GRAPH_API skr::Span<TextureReadEdge*> tex_read_edges();
    SKR_RENDER_GRAPH_API skr::Span<TextureRenderEdge*> tex_write_edges();
    SKR_RENDER_GRAPH_API skr::Span<TextureReadWriteEdge*> tex_readwrite_edges();
    SKR_RENDER_GRAPH_API void foreach_textures(skr::stl_function<void(TextureNode*, TextureEdge*)>);
    inline uint32_t textures_count() const
    {
        return (uint32_t)(in_texture_edges.size() + out_texture_edges.size() + inout_texture_edges.size());
    }

    SKR_RENDER_GRAPH_API skr::Span<BufferReadEdge*> buf_read_edges();
    SKR_RENDER_GRAPH_API skr::Span<BufferReadWriteEdge*> buf_readwrite_edges();
    SKR_RENDER_GRAPH_API skr::Span<PipelineBufferEdge*> buf_ppl_edges();
    SKR_RENDER_GRAPH_API void foreach_buffers(skr::stl_function<void(BufferNode*, BufferEdge*)>);
    inline uint32_t buffers_count() const
    {
        return (uint32_t)(in_buffer_edges.size() + out_buffer_edges.size() + ppl_buffer_edges.size());
    }

    SKR_RENDER_GRAPH_API skr::Span<AccelerationStructureReadEdge*> acceleration_structure_read_edges();
    SKR_RENDER_GRAPH_API void foreach_acceleration_structures(skr::stl_function<void(AccelerationStructureNode*, AccelerationStructureEdge*)>);
    inline uint32_t acceleration_structures_count() const
    {
        return (uint32_t)(in_acceleration_structure_edges.size());
    }
    const bool get_can_be_lone() const { return can_be_lone; }
    
    // Performance hint flags access
    void set_flags(EPassFlags flags) { hint_flags = flags; }
    void add_flags(EPassFlags flags) { hint_flags = static_cast<EPassFlags>(static_cast<uint32_t>(hint_flags) | static_cast<uint32_t>(flags)); }
    void remove_flags(EPassFlags flags) { hint_flags = static_cast<EPassFlags>(static_cast<uint32_t>(hint_flags) & ~static_cast<uint32_t>(flags)); }
    bool has_flags(EPassFlags flags) const { return (static_cast<uint32_t>(hint_flags) & static_cast<uint32_t>(flags)) != 0; }
    EPassFlags get_flags() const { return hint_flags; }

    const uint64_t frame_index;
    const EPassType pass_type = EPassType::None;
    const uint32_t order;

protected:
    bool can_be_lone = false;
    EPassFlags hint_flags = EPassFlags::None;
    PassNode(EPassType pass_type, uint32_t order, uint64_t frame_index);
    graph_edges_vector<TextureReadEdge*> in_texture_edges;
    graph_edges_vector<TextureRenderEdge*> out_texture_edges;
    graph_edges_vector<TextureReadWriteEdge*> inout_texture_edges;

    graph_edges_vector<BufferReadEdge*> in_buffer_edges;
    graph_edges_vector<BufferReadWriteEdge*> out_buffer_edges;
    graph_edges_vector<PipelineBufferEdge*> ppl_buffer_edges;

    graph_edges_vector<AccelerationStructureReadEdge*> in_acceleration_structure_edges;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class PassExecutionPhase;

    RenderPassNode(uint32_t order, uint64_t frame_index);
    CGPURootSignatureId get_root_signature() const { return root_signature; }
    
protected:
    RenderPassExecuteFunction executor;
    CGPURenderPipelineId pipeline = nullptr;
    CGPURootSignatureId root_signature = nullptr;
    ECGPULoadAction load_actions[CGPU_MAX_MRT_COUNT + 1];
    ECGPUStoreAction store_actions[CGPU_MAX_MRT_COUNT + 1];
    ECGPULoadAction depth_load_action;
    ECGPUStoreAction depth_store_action;
    ECGPULoadAction stencil_load_action;
    ECGPUStoreAction stencil_store_action;
    float clear_depth = 0.f;
};

class ComputePassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class PassExecutionPhase;

    ComputePassNode(uint32_t order, uint64_t frame_index);
    CGPURootSignatureId get_root_signature() const { return root_signature; }

protected:
    ComputePassExecuteFunction executor;
    CGPUComputePipelineId pipeline = nullptr;
    CGPURayPipelineId ray_pipeline = nullptr;
    CGPURootSignatureId root_signature = nullptr;
};

class CopyPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class PassExecutionPhase;

    CopyPassNode(uint32_t order, uint64_t frame_index);
protected:
    CopyPassExecuteFunction executor;
    graph_edges_vector<std::pair<TextureSubresourceHandle, TextureSubresourceHandle>, 2> t2ts;
    graph_edges_vector<std::pair<BufferRangeHandle, BufferRangeHandle>, 2> b2bs;
    graph_edges_vector<std::pair<BufferRangeHandle, TextureSubresourceHandle>, 2> b2ts;
    graph_edges_vector<std::pair<BufferHandle, ECGPUResourceState>, 2> bbarriers;
    graph_edges_vector<std::pair<TextureHandle, ECGPUResourceState>, 2> tbarriers;
};

class PresentPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class PassExecutionPhase;

    inline bool reimport(CGPUSwapChainId swapchain, uint32_t index)
    {
        if (!swapchain) return false;
        descriptor.swapchain = swapchain;
        descriptor.index = index;
        return true;
    }

    PresentPassNode(uint32_t order, uint64_t frame_index);
protected:
    CGPUQueuePresentDescriptor descriptor;
};
} // namespace render_graph
} // namespace skr