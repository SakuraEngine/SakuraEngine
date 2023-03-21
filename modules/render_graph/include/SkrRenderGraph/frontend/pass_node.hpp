#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/resource_edge.hpp"
#include <EASTL/vector.h>

#ifdef RG_USE_FIXED_VECTOR
#include <EASTL/fixed_vector.h>
#endif

namespace skr {
namespace render_graph
{
#ifdef RG_USE_FIXED_VECTOR
    template<typename T, uint32_t N = 4>
    using graph_edges_vector = eastl::fixed_vector<T, N>;  
#else
    template<typename T, uint32_t N = 4>
    using graph_edges_vector = eastl::vector<T>;
#endif
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

    SKR_RENDER_GRAPH_API const bool before(const PassNode* other) const;
    SKR_RENDER_GRAPH_API const bool after(const PassNode* other) const;
    SKR_RENDER_GRAPH_API const PassHandle get_handle() const;

    SKR_RENDER_GRAPH_API skr::span<TextureReadEdge*> tex_read_edges();
    SKR_RENDER_GRAPH_API skr::span<TextureRenderEdge*> tex_write_edges();
    SKR_RENDER_GRAPH_API skr::span<TextureReadWriteEdge*> tex_readwrite_edges();
    SKR_RENDER_GRAPH_API void foreach_textures(eastl::function<void(TextureNode*, TextureEdge*)>);
    inline uint32_t textures_count() const
    {
        return (uint32_t)(in_texture_edges.size() + out_texture_edges.size() + inout_texture_edges.size());
    }

    SKR_RENDER_GRAPH_API skr::span<BufferReadEdge*> buf_read_edges();
    SKR_RENDER_GRAPH_API skr::span<BufferReadWriteEdge*> buf_readwrite_edges();
    SKR_RENDER_GRAPH_API skr::span<PipelineBufferEdge*> buf_ppl_edges();
    SKR_RENDER_GRAPH_API void foreach_buffers(eastl::function<void(BufferNode*, BufferEdge*)>);
    inline uint32_t buffers_count() const
    {
        return (uint32_t)(in_buffer_edges.size() + out_buffer_edges.size() + ppl_buffer_edges.size());
    }

    const EPassType pass_type = EPassType::None;
    const uint32_t order;
protected:
    bool can_be_lone = false;
    PassNode(EPassType pass_type, uint32_t order);
    graph_edges_vector<TextureReadEdge*> in_texture_edges;
    graph_edges_vector<TextureRenderEdge*> out_texture_edges;
    graph_edges_vector<TextureReadWriteEdge*> inout_texture_edges;

    graph_edges_vector<BufferReadEdge*> in_buffer_edges;
    graph_edges_vector<BufferReadWriteEdge*> out_buffer_edges;
    graph_edges_vector<PipelineBufferEdge*> ppl_buffer_edges;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    RenderPassNode(uint32_t order);
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

    ComputePassNode(uint32_t order);
protected:
    ComputePassExecuteFunction executor;
    CGPUComputePipelineId pipeline;
    CGPURootSignatureId root_signature;
};

class CopyPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    CopyPassNode(uint32_t order);
protected:
    CopyPassExecuteFunction executor;
    graph_edges_vector<eastl::pair<TextureSubresourceHandle, TextureSubresourceHandle>, 2> t2ts;
    graph_edges_vector<eastl::pair<BufferRangeHandle, BufferRangeHandle>, 2> b2bs;
    graph_edges_vector<eastl::pair<BufferRangeHandle, TextureSubresourceHandle>, 2> b2ts;
};

class PresentPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    inline bool reimport(CGPUSwapChainId swapchain, uint32_t index)
    {
        if (!swapchain) return false;
        descriptor.swapchain = swapchain;
        descriptor.index = index;
        return true;
    }

    PresentPassNode(uint32_t order);
protected:
    CGPUQueuePresentDescriptor descriptor;
};
} // namespace render_graph
} // namespace skr