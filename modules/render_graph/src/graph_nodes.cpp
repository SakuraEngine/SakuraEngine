#include "platform/debug.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
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
} // namespace render_graph
} // namespace skr