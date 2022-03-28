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
using RenderPassExecuteFunction = eastl::function<
    void(class RenderGraph&, CGpuRenderPassEncoderId)>;

class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const PassHandle get_handle() const
    {
        return PassHandle(get_id());
    }
    inline gsl::span<TextureReadEdge*> read_edges()
    {
        return gsl::span<TextureReadEdge*>(in_edges.data(), in_edges.size());
    }
    inline gsl::span<TextureRenderEdge*> write_edges()
    {
        return gsl::span<TextureRenderEdge*>(out_edges.data(), out_edges.size());
    }

protected:
    PassNode(EPassType pass_type)
        : RenderGraphNode(EObjectType::Pass)
        , pass_type(pass_type)
    {
    }
    const EPassType pass_type = EPassType::None;
    eastl::vector<TextureReadEdge*> in_edges;
    eastl::vector<TextureRenderEdge*> out_edges;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

protected:
    RenderPassNode()
        : PassNode(EPassType::Render)
    {
    }
    RenderPassExecuteFunction executor;
    CGpuRenderPipelineId pipeline;
    CGpuRenderPassDescriptor descriptor;
};

class PresentPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

protected:
    PresentPassNode()
        : PassNode(EPassType::Present)
    {
    }
    CGpuQueuePresentDescriptor descriptor;
};

} // namespace render_graph
} // namespace sakura