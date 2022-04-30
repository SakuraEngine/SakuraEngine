#pragma once
#include <EASTL/vector.h>
#include <gsl/span>
#include "render_graph/frontend/base_types.hpp"
#include "render_graph/frontend/resource_node.hpp"
#include "render_graph/frontend/resource_edge.hpp"

namespace sakura
{
namespace render_graph
{

struct PassContext {
    CGPUCommandBufferId cmd;
    gsl::span<eastl::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    gsl::span<eastl::pair<TextureHandle, CGPUTextureId>> resolved_textures;

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

    const bool before(const PassNode* other) const;
    const bool after(const PassNode* other) const;
    const PassHandle get_handle() const;

    gsl::span<TextureReadEdge*> tex_read_edges();
    gsl::span<TextureRenderEdge*> tex_write_edges();
    gsl::span<TextureReadWriteEdge*> tex_readwrite_edges();
    void foreach_textures(eastl::function<void(TextureNode*, TextureEdge*)>);
    inline uint32_t textures_count() const
    {
        return (uint32_t)(in_texture_edges.size() + out_texture_edges.size() + inout_texture_edges.size());
    }

    gsl::span<BufferReadEdge*> buf_read_edges();
    gsl::span<BufferReadWriteEdge*> buf_readwrite_edges();
    gsl::span<PipelineBufferEdge*> buf_ppl_edges();
    void foreach_buffers(eastl::function<void(BufferNode*, BufferEdge*)>);
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
    gsl::span<CGPUDescriptorSetId> desc_sets;
};
using RenderPassExecuteFunction = eastl::function<
void(class RenderGraph&, RenderPassContext&)>;
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
    CGPURenderPipelineId pipeline;
    ECGPULoadAction load_actions[MAX_MRT_COUNT + 1];
    ECGPUStoreAction store_actions[MAX_MRT_COUNT + 1];
    ECGPULoadAction depth_load_action;
    ECGPUStoreAction depth_store_action;
    ECGPULoadAction stencil_load_action;
    ECGPUStoreAction stencil_store_action;
};

struct ComputePassContext : public PassContext {
    CGPUComputePassEncoderId encoder;
    gsl::span<CGPUDescriptorSetId> desc_sets;
};
using ComputePassExecuteFunction = eastl::function<
void(class RenderGraph&, ComputePassContext&)>;
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
};

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
inline gsl::span<TextureReadEdge*> PassNode::tex_read_edges()
{
    return gsl::span<TextureReadEdge*>(in_texture_edges.data(), in_texture_edges.size());
}
inline gsl::span<TextureRenderEdge*> PassNode::tex_write_edges()
{
    return gsl::span<TextureRenderEdge*>(out_texture_edges.data(), out_texture_edges.size());
}
inline gsl::span<TextureReadWriteEdge*> PassNode::tex_readwrite_edges()
{
    return gsl::span<TextureReadWriteEdge*>(inout_texture_edges.data(), inout_texture_edges.size());
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
inline gsl::span<BufferReadEdge*> PassNode::buf_read_edges()
{
    return gsl::span<BufferReadEdge*>(in_buffer_edges.data(), in_buffer_edges.size());
}
inline gsl::span<BufferReadWriteEdge*> PassNode::buf_readwrite_edges()
{
    return gsl::span<BufferReadWriteEdge*>(out_buffer_edges.data(), out_buffer_edges.size());
}
inline gsl::span<PipelineBufferEdge*> PassNode::buf_ppl_edges()
{
    return gsl::span<PipelineBufferEdge*>(ppl_buffer_edges.data(), ppl_buffer_edges.size());
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
} // namespace sakura