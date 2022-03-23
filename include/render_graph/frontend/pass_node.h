#pragma once
#include <EASTL/vector.h>
#include "render_graph/frontend/base_types.hpp"
#include "render_graph/frontend/resource_node.h"

namespace sakura
{
namespace render_graph
{
using PassExecuteFunction = eastl::function<void()>;

class PassNode : public RenderGraphNode
{
    friend class RenderGraph;

public:
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
    eastl::vector<TextureReferenceEdge*> in_edges;
    eastl::vector<TextureAccessEdge*> out_edges;
};

class RenderPassNode : public PassNode
{
public:
protected:
    CGpuRenderPassDescriptor descriptor;
};
} // namespace render_graph
} // namespace sakura