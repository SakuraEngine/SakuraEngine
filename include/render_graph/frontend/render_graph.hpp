#pragma once
#include <EASTL/unique_ptr.h>
#include "dependency_graph.hpp"

namespace sakura
{
class RenderGraph
{
public:
protected:
    eastl::unique_ptr<RenderDependencyGraph> graph;
};
} // namespace sakura
