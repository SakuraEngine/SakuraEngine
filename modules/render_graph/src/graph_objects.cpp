#include "platform/debug.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "utils/log.h"

namespace skr
{
namespace render_graph
{
struct SKR_RENDER_GRAPH_API NodeAndEdgeFactoryImpl final : public NodeAndEdgeFactory
{
    ~NodeAndEdgeFactoryImpl() SKR_NOEXCEPT = default;
};

NodeAndEdgeFactory* NodeAndEdgeFactory::Create()
{
    return SkrNew<NodeAndEdgeFactoryImpl>();
}

void NodeAndEdgeFactory::Destroy(NodeAndEdgeFactory* factory)
{
    SkrDelete(factory);
}

// 0.node
RenderGraphNode::RenderGraphNode(EObjectType type)
    : type(type)
{
}

void RenderGraphNode::set_name(const char* n)
{
    name = n;
}

const char* RenderGraphNode::get_name() const
{
    return name.c_str();
}

RenderGraphEdge::RenderGraphEdge(ERelationshipType type)
    : type(type)
{
}

// 1.resource nodes

ResourceNode::ResourceNode(EObjectType type) SKR_NOEXCEPT
    : RenderGraphNode(type),
        imported(false)
{
}

TextureNode::TextureNode() SKR_NOEXCEPT
    : ResourceNode(EObjectType::Texture)
{
}

BufferNode::BufferNode() SKR_NOEXCEPT
    : ResourceNode(EObjectType::Buffer)
{
}

// 2.pass nodes

PassNode::PassNode(EPassType pass_type, uint32_t order)
    : RenderGraphNode(EObjectType::Pass)
    , pass_type(pass_type)
    , order(order)
{
}

const PassHandle PassNode::get_handle() const
{
    return PassHandle(get_id());
}

const bool PassNode::before(const PassNode* other) const
{
    if (other == nullptr) return false;
    const bool _ = order < other->order;
    return _;
}

const bool PassNode::after(const PassNode* other) const
{
    if (other == nullptr) return true;
    const bool _ = order > other->order;
    return _;
}

skr::span<TextureReadEdge*> PassNode::tex_read_edges()
{
    return skr::span<TextureReadEdge*>(in_texture_edges.data(), in_texture_edges.size());
}

skr::span<TextureRenderEdge*> PassNode::tex_write_edges()
{
    return skr::span<TextureRenderEdge*>(out_texture_edges.data(), out_texture_edges.size());
}

skr::span<TextureReadWriteEdge*> PassNode::tex_readwrite_edges()
{
    return skr::span<TextureReadWriteEdge*>(inout_texture_edges.data(), inout_texture_edges.size());
}

void PassNode::foreach_textures(eastl::function<void(TextureNode*, TextureEdge*)> f)
{
    for (auto&& e : tex_read_edges())
        f(e->get_texture_node(), e);
    for (auto&& e : tex_write_edges())
        f(e->get_texture_node(), e);
    for (auto&& e : tex_readwrite_edges())
        f(e->get_texture_node(), e);
}

skr::span<BufferReadEdge*> PassNode::buf_read_edges()
{
    return skr::span<BufferReadEdge*>(in_buffer_edges.data(), in_buffer_edges.size());
}

skr::span<BufferReadWriteEdge*> PassNode::buf_readwrite_edges()
{
    return skr::span<BufferReadWriteEdge*>(out_buffer_edges.data(), out_buffer_edges.size());
}

skr::span<PipelineBufferEdge*> PassNode::buf_ppl_edges()
{
    return skr::span<PipelineBufferEdge*>(ppl_buffer_edges.data(), ppl_buffer_edges.size());
}

void PassNode::foreach_buffers(eastl::function<void(BufferNode*, BufferEdge*)> f)
{
    for (auto&& e : buf_read_edges())
        f(e->get_buffer_node(), e);
    for (auto&& e : buf_readwrite_edges())
        f(e->get_buffer_node(), e);
    for (auto&& e : buf_ppl_edges())
        f(e->get_buffer_node(), e);
}

RenderPassNode::RenderPassNode(uint32_t order)
    : PassNode(EPassType::Render, order)
{
}

ComputePassNode::ComputePassNode(uint32_t order)
    : PassNode(EPassType::Compute, order)
{
}

CopyPassNode::CopyPassNode(uint32_t order)
    : PassNode(EPassType::Copy, order)
{
}

PresentPassNode::PresentPassNode(uint32_t order)
    : PassNode(EPassType::Present, order)
{
}

// 3.edges

TextureEdge::TextureEdge(ERelationshipType type, ECGPUResourceState requested_state) SKR_NOEXCEPT
    : RenderGraphEdge(type)
    , requested_state(requested_state)
{
}

// 3.1 Texture Read Edge

TextureReadEdge::TextureReadEdge(uint32_t set, uint32_t binding, TextureSRVHandle handle, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureRead, state)
    , set(set)
    , binding(binding)
    , handle(handle)

{
}
TextureReadEdge::TextureReadEdge(const char8_t* name, TextureSRVHandle handle, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureRead, state)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name(name)
    , handle(handle)
{
}

TextureNode* TextureReadEdge::get_texture_node()
{
    return static_cast<TextureNode*>(from());
}

PassNode* TextureReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

// 3.2 Texture RTV Edge

TextureRenderEdge::TextureRenderEdge(uint32_t mrt_index, TextureRTVHandle handle, CGPUClearValue clear_value, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureWrite, state)
    , mrt_index(mrt_index)
    , handle(handle)
    , clear_value(clear_value)
{
}

TextureNode* TextureRenderEdge::get_texture_node()
{
    return static_cast<TextureNode*>(to());
}

PassNode* TextureRenderEdge::get_pass_node()
{
    return (PassNode*)from();
}

// 3.3 Texture UAV edge

TextureReadWriteEdge::TextureReadWriteEdge(uint32_t set, uint32_t binding, TextureUAVHandle handle, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureReadWrite, state)
    , set(set)
    , binding(binding)
    , handle(handle)

{
}

TextureReadWriteEdge::TextureReadWriteEdge(const char8_t* name, TextureUAVHandle handle, ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureReadWrite, state)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name(name)
    , handle(handle)

{
}

TextureNode* TextureReadWriteEdge::get_texture_node()
{
    return static_cast<TextureNode*>(to());
}

PassNode* TextureReadWriteEdge::get_pass_node()
{
    return (PassNode*)from();
}

// 3.4 Pipeline buffer edge

PipelineBufferEdge::PipelineBufferEdge(PipelineBufferHandle handle, ECGPUResourceState state)
    : BufferEdge(ERelationshipType::PipelineBuffer, state)
    , handle(handle)
{
}

BufferNode* PipelineBufferEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(from());
}

PassNode* PipelineBufferEdge::get_pass_node()
{
    return (PassNode*)to();
}

// 3.5 Buffer read edge

BufferReadEdge::BufferReadEdge(const char8_t* name, BufferRangeHandle handle, ECGPUResourceState state)
    : BufferEdge(ERelationshipType::BufferRead, state)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name (name)
    , handle(handle)
{
}

BufferNode* BufferReadEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(from());
}

PassNode* BufferReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

// 3.6 Buffer UAV edge

BufferReadWriteEdge::BufferReadWriteEdge(BufferRangeHandle handle, ECGPUResourceState state)
    : BufferEdge(ERelationshipType::BufferReadWrite, state)
    , handle(handle)
{
}

BufferNode* BufferReadWriteEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(to());
}

PassNode* BufferReadWriteEdge::get_pass_node()
{
    return (PassNode*)from();
}

} // namespace render_graph
} // namespace skr