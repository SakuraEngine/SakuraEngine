#include "render_graph/frontend/base_types.hpp"
#include "render_graph/frontend/resource_node.h"
#include "render_graph/rg_config.h"
#include "utils/DAG.boost.hpp"
#include <boost/graph/graphviz.hpp>
#include "render_graph/frontend/render_graph.hpp"
#include "utils/dependency_graph.hpp"

namespace sakura
{
namespace render_graph
{
template <class EdgeProps>
class EdgeWriter
{
public:
    EdgeWriter(EdgeProps& edge)
        : prop(edge)
    {
    }
    template <class Edge>
    void operator()(std::ostream& out, const Edge& e) const
    {
        RenderGraphEdge* rg_edge = (RenderGraphEdge*)prop[e];
        eastl::string label;
        switch (rg_edge->type)
        {
            case ERelationshipType::TextureRead: {
                auto SRV = (TextureReferenceEdge*)rg_edge;
                label = "SRV";
            }
            break;
            case ERelationshipType::TextureWrite: {
                auto RTV = (TextureReferenceEdge*)rg_edge;
                label = "RTV";
            }
            break;
            default:
                break;
        }
        out << "[label=\"" << label.c_str() << "\"]";
    }

private:
    EdgeProps prop;
};

template <class VertProps>
class VertWriter
{
public:
    VertWriter(VertProps& vert)
        : prop(vert)
    {
    }
    template <class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const
    {
        RenderGraphNode* rg_node = (RenderGraphNode*)prop[v];
        eastl::string label;
        eastl::string shape = "none";
        switch (rg_node->type)
        {
            case EObjectType::Texture:
                label = "texture:\n";
                shape = "box";
                break;
            case EObjectType::Pass:
                label = "pass:\n";
                shape = "ellipse";
                break;
            default:
                break;
        }
        label.append(rg_node->get_name());
        out << "[label=\"" << label.c_str() << "\"]";
        out << "[shape=\"" << shape.c_str() << "\"]";
    }

private:
    VertProps prop;
};

#include <fstream>
void RenderGraphViz::write_graphviz(RenderGraph& graph, const char* outf)
{
    DependencyGraphBase& g = *DependencyGraphBase::as(graph.graph.get());
    EdgeWriter<DependencyGraphBase> w(g);
    VertWriter<DependencyGraphBase> v(g);
    std::ofstream ofstm(outf);
    boost::write_graphviz(ofstm, g, v, w);
}
} // namespace render_graph
} // namespace sakura