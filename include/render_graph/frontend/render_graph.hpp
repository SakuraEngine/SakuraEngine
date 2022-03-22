#pragma once
#include <EASTL/unique_ptr.h>
#include "utils/dependency_graph.hpp"

namespace sakura
{
namespace render_graph
{
class RenderGraph
{
public:
protected:
    eastl::unique_ptr<DependencyGraph> graph;
};
} // namespace render_graph
} // namespace sakura
