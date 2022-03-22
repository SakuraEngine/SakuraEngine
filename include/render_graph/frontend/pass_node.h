#pragma once
#include "cgpu/api.h"
#include "render_graph/rg_config.h"
#include "render_graph/frontend/dependency_graph.hpp"

namespace sakura
{
namespace render_graph
{
class PassNode : public DependencyGraphNode
{
};

class RenderPassNode : public PassNode
{
public:
protected:
    CGpuRenderPassDescriptor descriptor;
};
} // namespace render_graph
} // namespace sakura