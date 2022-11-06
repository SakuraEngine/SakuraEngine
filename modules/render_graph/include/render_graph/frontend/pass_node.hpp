#pragma once
#include <EASTL/vector.h>
#include "containers/span.hpp"
#include "render_graph/frontend/base_types.hpp"
#include "render_graph/frontend/resource_node.hpp"
#include "render_graph/frontend/resource_edge.hpp"

namespace skr
{
namespace render_graph
{

struct SKR_RENDER_GRAPH_API PassContext {
    CGPUCommandBufferId cmd;
    skr::span<eastl::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    skr::span<eastl::pair<TextureHandle, CGPUTextureId>> resolved_textures;

    inline CGPUBufferId resolve(BufferHandle buffer_handle) const
    {
        for (auto iter : resolved_buffers)
        {
            if (iter.first == buffer_handle) return iter.second;
        }
        return nullptr;
    }
    inline CGPUTextureId resolve(TextureHandle tex_handle) const
    {
        for (auto iter : resolved_textures)
        {
            if (iter.first == tex_handle) return iter.second;
        }
        return nullptr;
    }
};
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
    PassNode(EPassType pass_type, uint32_t order);
    eastl::vector<TextureReadEdge*> in_texture_edges;
    eastl::vector<TextureRenderEdge*> out_texture_edges;
    eastl::vector<TextureReadWriteEdge*> inout_texture_edges;

    eastl::vector<BufferReadEdge*> in_buffer_edges;
    eastl::vector<BufferReadWriteEdge*> out_buffer_edges;
    eastl::vector<PipelineBufferEdge*> ppl_buffer_edges;
};

struct RenderPassContext : public PassContext {
    CGPURenderPassEncoderId encoder;
    skr::span<CGPUDescriptorSetId> desc_sets;
};
using RenderPassExecuteFunction = eastl::function<void(class RenderGraph&, RenderPassContext&)>;
class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

protected:
    RenderPassNode(uint32_t order)
        : PassNode(EPassType::Render, order)
    {
    }
    RenderPassExecuteFunction executor;
    CGPURenderPipelineId pipeline = nullptr;
    CGPURootSignatureId root_signature = nullptr;
    ECGPULoadAction load_actions[CGPU_MAX_MRT_COUNT + 1];
    ECGPUStoreAction store_actions[CGPU_MAX_MRT_COUNT + 1];
    ECGPULoadAction depth_load_action;
    ECGPUStoreAction depth_store_action;
    ECGPULoadAction stencil_load_action;
    ECGPUStoreAction stencil_store_action;
};

struct SKR_RENDER_GRAPH_API ComputePassContext : public PassContext {
    CGPUComputePassEncoderId encoder;
    skr::span<CGPUDescriptorSetId> desc_sets;
};
using ComputePassExecuteFunction = eastl::function<void(class RenderGraph&, ComputePassContext&)>;
class ComputePassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

protected:
    ComputePassNode(uint32_t order)
        : PassNode(EPassType::Compute, order)
    {
    }
    ComputePassExecuteFunction executor;
    CGPUComputePipelineId pipeline;
    CGPURootSignatureId root_signature;
};

struct CopyPassContext : public PassContext {
    CGPUCommandBufferId cmd;
};
using CopyPassExecuteFunction = eastl::function<void(class RenderGraph&, CopyPassContext&)>;
class CopyPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

protected:
    CopyPassNode(uint32_t order)
        : PassNode(EPassType::Copy, order)
    {
    }
    CopyPassExecuteFunction executor;
    eastl::vector<eastl::pair<TextureSubresourceHandle, TextureSubresourceHandle>> t2ts;
    eastl::vector<eastl::pair<BufferRangeHandle, BufferRangeHandle>> b2bs;
};

class PresentPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

protected:
    PresentPassNode(uint32_t order)
        : PassNode(EPassType::Present, order)
    {
    }
    CGPUQueuePresentDescriptor descriptor;
};

// pass node impl
inline PassNode::PassNode(EPassType pass_type, uint32_t order)
    : RenderGraphNode(EObjectType::Pass)
    , pass_type(pass_type)
    , order(order)
{
}
inline const PassHandle PassNode::get_handle() const
{
    return PassHandle(get_id());
}
inline const bool PassNode::before(const PassNode* other) const
{
    if (other == nullptr) return false;
    const bool _ = order < other->order;
    return _;
}
inline const bool PassNode::after(const PassNode* other) const
{
    if (other == nullptr) return true;
    const bool _ = order > other->order;
    return _;
}
inline skr::span<TextureReadEdge*> PassNode::tex_read_edges()
{
    return skr::span<TextureReadEdge*>(in_texture_edges.data(), in_texture_edges.size());
}
inline skr::span<TextureRenderEdge*> PassNode::tex_write_edges()
{
    return skr::span<TextureRenderEdge*>(out_texture_edges.data(), out_texture_edges.size());
}
inline skr::span<TextureReadWriteEdge*> PassNode::tex_readwrite_edges()
{
    return skr::span<TextureReadWriteEdge*>(inout_texture_edges.data(), inout_texture_edges.size());
}
inline void PassNode::foreach_textures(eastl::function<void(TextureNode*, TextureEdge*)> f)
{
    for (auto&& e : tex_read_edges())
        f(e->get_texture_node(), e);
    for (auto&& e : tex_write_edges())
        f(e->get_texture_node(), e);
    for (auto&& e : tex_readwrite_edges())
        f(e->get_texture_node(), e);
}
inline skr::span<BufferReadEdge*> PassNode::buf_read_edges()
{
    return skr::span<BufferReadEdge*>(in_buffer_edges.data(), in_buffer_edges.size());
}
inline skr::span<BufferReadWriteEdge*> PassNode::buf_readwrite_edges()
{
    return skr::span<BufferReadWriteEdge*>(out_buffer_edges.data(), out_buffer_edges.size());
}
inline skr::span<PipelineBufferEdge*> PassNode::buf_ppl_edges()
{
    return skr::span<PipelineBufferEdge*>(ppl_buffer_edges.data(), ppl_buffer_edges.size());
}
inline void PassNode::foreach_buffers(eastl::function<void(BufferNode*, BufferEdge*)> f)
{
    for (auto&& e : buf_read_edges())
        f(e->get_buffer_node(), e);
    for (auto&& e : buf_readwrite_edges())
        f(e->get_buffer_node(), e);
    for (auto&& e : buf_ppl_edges())
        f(e->get_buffer_node(), e);
}
} // namespace render_graph
} // namespace skr