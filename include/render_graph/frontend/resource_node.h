#pragma once
#include "cgpu/api.h"
#include "cgpu/flags.h"
#include "render_graph/rg_config.h"
#include "render_graph/frontend/dependency_graph.hpp"

namespace sakura
{
namespace render_graph
{
class ResourceNode : public DependencyGraphNode
{
};

class TextureNode : public ResourceNode
{
public:
protected:
    // resource
    CGpuTextureId texture;
};

class TextureReferenceEdge : public DependencyGraphEdge
{
protected:
    ECGpuResourceState requested_state;
    CGpuTextureViewId texture_view;
};
} // namespace render_graph
} // namespace sakura