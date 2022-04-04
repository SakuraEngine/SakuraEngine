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

struct RenderPassStack {
    CGpuRenderPassEncoderId encoder;
    gsl::span<CGpuDescriptorSetId> desc_sets;
};

using RenderPassExecuteFunction = eastl::function<
    void(class RenderGraph&, RenderPassStack&)>;

class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const bool before(const PassNode* other) const;
    const bool after(const PassNode* other) const;
    const PassHandle get_handle() const;
    inline gsl::span<TextureReadEdge*> read_edges();
    inline gsl::span<TextureRenderEdge*> write_edges();

    const EPassType pass_type = EPassType::None;
    const uint32_t order;

protected:
    PassNode(EPassType pass_type, uint32_t order);
    eastl::vector<TextureReadEdge*> in_edges;
    eastl::vector<TextureRenderEdge*> out_edges;
};

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
    CGpuRenderPipelineId pipeline;
    ECGpuLoadAction load_actions[MAX_MRT_COUNT + 1];
    ECGpuStoreAction store_actions[MAX_MRT_COUNT + 1];
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
    CGpuQueuePresentDescriptor descriptor;
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
inline gsl::span<TextureReadEdge*> PassNode::read_edges()
{
    return gsl::span<TextureReadEdge*>(in_edges.data(), in_edges.size());
}
inline gsl::span<TextureRenderEdge*> PassNode::write_edges()
{
    return gsl::span<TextureRenderEdge*>(out_edges.data(), out_edges.size());
}

} // namespace render_graph
} // namespace sakura