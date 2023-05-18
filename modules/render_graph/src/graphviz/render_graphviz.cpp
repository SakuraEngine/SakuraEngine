#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunknown-pragmas"
    #pragma clang diagnostic ignored "-Wuninitialized-const-reference"
#endif
#include "utils/DAG.boost.hpp"
#include <boost/graph/graphviz.hpp>
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"

namespace skr
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
        skr::string label;
        switch (rg_edge->type)
        {
            case ERelationshipType::TextureRead: {
                auto SRV = (TextureReadEdge*)rg_edge;
                if (auto name = SRV->get_name())
                {
                    label = u8"SRV: ";
                    label.append(name);
                }
                else
                {
                    label = u8"SRV";
                    //label = "SRV:s";
                    //label.append(skr::to_string(SRV->set))
                    //.append("b")
                    //.append(skr::to_string(SRV->binding));
                }
            }
            break;
            case ERelationshipType::TextureReadWrite: {
                auto UAV = (TextureReadWriteEdge*)rg_edge;
                if (auto name = UAV->get_name())
                {
                    label = u8"UAV: ";
                    label.append(name);
                }
                else
                {
                    label = u8"UAV";
                    //label = "UAV:s";
                    //label.append(skr::to_string(UAV->set))
                    //.append("b")
                    //.append(skr::to_string(UAV->binding));
                }
            }
            break;
            case ERelationshipType::TextureWrite: {
                auto RTV = (TextureRenderEdge*)rg_edge;
                label = u8"RTV:";
                label.append(skr::format(u8"{}", RTV->mrt_index));
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
        skr::string label;
        skr::string color = u8"lavenderblush";
        skr::string shape = u8"none";
        switch (rg_node->type)
        {
            case EObjectType::Texture: {
                TextureNode* tex_node = (TextureNode*)rg_node;
                const bool is_imported = tex_node->is_imported();
                color = is_imported ? u8"grey35" : u8"grey70";
                label = u8"texture: ";
                label.append(rg_node->get_name());
                label.append(u8"\\nrefs: ")
                    .append(is_imported ? u8"imported" : skr::format(u8"{}", tex_node->outgoing_edges()).u8_str());
                if (auto aliasing_parent = tex_node->get_aliasing_parent(); aliasing_parent)
                {
                    label.append(u8"\\naliasing: ").append(aliasing_parent->get_name());
                }
                shape = u8"box";
            }
            break;
            case EObjectType::Buffer: {
                BufferNode* buf_node = (BufferNode*)rg_node;
                const bool is_imported = buf_node->is_imported();
                color = is_imported ? u8"limegreen" : u8"lightgreen";
                label = u8"buffer: ";
                label.append(rg_node->get_name());
                label.append(u8"\\nrefs: ")
                    .append(is_imported ? u8"imported" : skr::format(u8"{}", buf_node->outgoing_edges()).u8_str());
                shape = u8"box";
            }
            break;
            case EObjectType::Pass: {
                PassNode* pass_node = (PassNode*)rg_node;
                shape = u8"ellipse";
                switch (pass_node->pass_type)
                {
                    case EPassType::Compute: {
                        label = u8"compute: ";
                        color = u8"lemonchiffon";
                    }
                    break;
                    case EPassType::Copy: {
                        label = u8"copy: ";
                        color = u8"lightblue1";
                    }
                    break;
                    case EPassType::Render: {
                        label = u8"render(geom): ";
                    }
                    break;
                    default:
                        break;
                }
                label.append(rg_node->get_name());
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
void RenderGraphViz::write_graphviz(RenderGraph& graph, const char* outf) SKR_NOEXCEPT
{
    DependencyGraphBase& g = *DependencyGraphBase::as(graph.graph);
    EdgeWriter<DependencyGraphBase> w(g);
    VertWriter<DependencyGraphBase> v(g);
    std::ofstream ofstm(outf);
    boost::write_graphviz(ofstm, g, v, w);
}
} // namespace render_graph
} // namespace skr

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif