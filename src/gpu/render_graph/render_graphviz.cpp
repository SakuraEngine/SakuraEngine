#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunknown-pragmas"
    #pragma clang diagnostic ignored "-Wuninitialized-const-reference"
#endif

#include "utils/DAG.boost.hpp"
#include <boost/graph/graphviz.hpp>
#include "render_graph/frontend/render_graph.hpp"

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
                auto SRV = (TextureReadEdge*)rg_edge;
                label = "SRV:s";
                label.append(eastl::to_string(SRV->set))
                    .append("b")
                    .append(eastl::to_string(SRV->binding));
            }
            break;
            case ERelationshipType::TextureWrite: {
                auto RTV = (TextureRenderEdge*)rg_edge;
                label = "RTV:";
                label.append(eastl::to_string(RTV->mrt_index));
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
        eastl::string color = "lavenderblush";
        eastl::string shape = "none";
        switch (rg_node->type)
        {
            case EObjectType::Texture: {
                TextureNode* tex_node = (TextureNode*)rg_node;
                const bool is_imported = tex_node->is_imported();
                color = is_imported ? "grey35" : "grey70";
                label = "texture: ";
                label.append(rg_node->get_name());
                label.append("\nrefs: ")
                    .append(is_imported ? "imported" : eastl::to_string(tex_node->outgoing_edges().size()));
                shape = "box";
            }
            break;
            case EObjectType::Pass: {
                label = "pass: ";
                label.append(rg_node->get_name());
                shape = "ellipse";
            }
            break;
            default:
                break;
        }
        out << "[label=\"" << label.c_str() << "\"]";
        out << "[shape=\"" << shape.c_str() << "\"]";
        out << "[fillcolor=\"" << color.c_str() << "\"]";
        out << "[style=\"filled\"]";
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

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif