#pragma once
#include <EASTL/vector.h>
#include "render_graph/frontend/base_types.hpp"
#include "render_graph/frontend/resource_node.hpp"
#include "render_graph/frontend/resource_edge.hpp"

namespace sakura
{
namespace render_graph
{
using PassExecuteFunction = eastl::function<
    void(RenderGraph&, CGpuRenderPassEncoderId)>;

class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    PassNode()
        : RenderGraphNode(EObjectType::Pass)
    {
    }
    const PassHandle get_handle() const
    {
        return PassHandle(get_id());
    }

protected:
    PassExecuteFunction executor;
    eastl::vector<TextureReadEdge*> in_edges;
    eastl::vector<TextureRenderEdge*> out_edges;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;

protected:
    CGpuRenderPipelineId pipeline;
    CGpuRenderPassDescriptor descriptor;
};
} // namespace render_graph
} // namespace sakura