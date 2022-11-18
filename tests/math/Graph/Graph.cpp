#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunknown-pragmas"
    #pragma clang diagnostic ignored "-Wuninitialized-const-reference"
#endif

#include "utils/DAG.boost.hpp"
#include <boost/graph/graphviz.hpp>
#include "gtest/gtest.h"
#include <EASTL/string.h>
#include <fstream>
#include <iostream>

class GraphTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};
namespace DAG = skr::DAG;

struct vertex_prop_map_key_t {
    using kind = boost::vertex_property_tag;
};

template <typename EdgeIter, typename Graph>
void who_owes_who(EdgeIter first, EdgeIter last, const Graph& G)
{
    // Access the propety acessor type for this graph
    using VertexPropMap = typename boost::property_map<Graph, vertex_prop_map_key_t>::const_type;
    VertexPropMap name = get(vertex_prop_map_key_t(), G);

    using NameType = typename boost::property_traits<VertexPropMap>::value_type;
    NameType src_name, targ_name;

    while (first != last)
    {
        src_name = boost::get(name, source(*first, G));
        targ_name = boost::get(name, target(*first, G));
        std::cout << src_name << " -prop-> " << targ_name << std::endl;
        ++first;
    }
}

template <class EdgeProps>
class EdgeWriter
{
public:
    EdgeWriter(EdgeProps _name)
        : prop(_name)
    {
    }
    template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge& v) const
    {
        out << "[label=\"" << prop[v].name.c_str() << "\"]";
    }

private:
    EdgeProps prop;
};

TEST(GraphTest, GraphTest0)
{
    using vertexProp = boost::property<vertex_prop_map_key_t, std::string>;
    struct edgeProp {
        eastl::string name;
    };
    using Graph = DAG::Graph<vertexProp, edgeProp>;
    using GraphVertex = DAG::GraphVertex<vertexProp, edgeProp>;

    Graph g(5);

    auto E20 = boost::add_edge(2, 0, g).first;
    g[E20] = { "E20" };
    auto E10 = boost::add_edge(1, 0, g).first;
    g[E10] = { "E10" };
    auto E21 = boost::add_edge(2, 1, g).first;
    g[E21] = { "E21" };
    auto E12 = boost::add_edge(1, 2, g).first;
    g[E12] = { "E12" };
    auto E42 = boost::add_edge(4, 2, g).first;
    g[E42] = { "E42" };
    std::cout << "num vertices: " << boost::num_vertices(g) << std::endl;

    GraphVertex from(2);
    GraphVertex to(0);
    auto out_edges = DAG::out_edges(from, g);
    auto iter = out_edges.first;
    while (iter != out_edges.second)
    {
        if (iter->m_target == to)
        {
            auto&& prop = g[*iter];
            std::cout << prop.name.c_str() << std::endl;
        }
        iter++;
    }

    using Graph2 = DAG::Graph<int, edgeProp>;
    using GraphVertex2 = DAG::GraphVertex<int, edgeProp>;
    GraphVertex2 from2(2);
    Graph2 g2;
    boost::add_vertex(12, g2);
    boost::add_vertex(12, g2);
    boost::add_vertex(32, g2);
    auto nd = g2[from2];
    std::cout << "Node ID: " << nd << std::endl;

    boost::property_map<Graph, vertex_prop_map_key_t>::type val = boost::get(vertex_prop_map_key_t(), g);
    boost::put(val, 0, "0");
    boost::put(val, 1, "1");
    boost::put(val, 2, "2");
    boost::put(val, 3, "3");
    boost::put(val, 4, "4");

    GraphVertex vert(2);

    std::cout << "out degrees of vertex " << vert << " is: " << DAG::out_degree(vert, g) << std::endl;
    std::cout << "in degrees of vertex " << vert << " is: " << DAG::in_degree(vert, g) << std::endl;
    std::cout << "out degrees of vertex " << DAG::vertex(4, g) << " is: " << DAG::out_degree(DAG::vertex(4, g), g) << std::endl;

    using IndexMap = boost::property_map<Graph, boost::vertex_index_t>::type;
    IndexMap index_map = get(boost::vertex_index, g);

    auto inedges = DAG::in_edges(vert, g);
    std::cout << "list of in edges of vertex " << vert << ": " << std::endl;
    for (auto iter = inedges.first; iter != inedges.second; iter++)
    {
        std::cout << "    " << DAG::source(*iter, g) << "->" << vert << std::endl;
    }
    auto neigs = DAG::adjacent_vertices(vert, g);
    std::cout << "neigs of vertex " << vert << ": ";
    for (auto iter = neigs.first; iter != neigs.second; iter++)
    {
        std::cout << DAG::vertex_number(*iter, g) << ", ";
    }
    auto inv_neigs = DAG::inv_adjacent_vertices(vert, g);
    std::cout << std::endl
              << "inv neigs of vertex " << vert << ": ";
    for (auto iter = inv_neigs.first; iter != inv_neigs.second; iter++)
    {
        std::cout << DAG::get_vertex_property<vertex_prop_map_key_t>(*iter, g) << ", ";
    }
    std::cout << std::endl;

    who_owes_who(boost::edges(g).first, boost::edges(g).second, g);

    EdgeWriter<Graph> w(g);
    std::ofstream outf("net.gv");
    boost::write_graphviz(outf, g, boost::default_writer(), w);
}

#include "utils/dependency_graph.hpp"

class TestRDGNode : public skr::DependencyGraphNode
{
public:
    TestRDGNode(const char* n)
        : DependencyGraphNode()
        , name(n)
    {
    }

    eastl::string name;
};

TEST(GraphTest, DependencyGraph)
{
    skr::DependencyGraphEdge edge;
    TestRDGNode node0("node0");
    TestRDGNode node1("node1");
    TestRDGNode node2("node2");
    auto rdg = skr::DependencyGraph::Create();
    rdg->insert(&node0);
    rdg->insert(&node1);
    rdg->insert(&node2);
    rdg->link(&node0, &node1, &edge);
    rdg->link(&node0, &node2, &edge);
    rdg->link(&node1, &node2, &edge);
    std::cout << rdg->outgoing_edges(&node0) << std::endl;
    std::cout << rdg->incoming_edges(&node2) << std::endl;
    using Node = skr::DependencyGraphNode;
    using Edge = skr::DependencyGraphEdge;
    rdg->foreach_incoming_edges(&node1, [](Node* from, Node* to, Edge* e) {
        std::cout << "edge: " << ((TestRDGNode*)from)->name.c_str()
                  << " -> " << ((TestRDGNode*)to)->name.c_str() << std::endl;
    });
    skr::DependencyGraph::Destroy(rdg);
}

#include "SkrRenderGraph/frontend/render_graph.hpp"

TEST(GraphTest, RenderGraphFrontEnd)
{
    namespace render_graph = skr::render_graph;
    auto graph = render_graph::RenderGraph::create(
    [](render_graph::RenderGraphBuilder& builder) {
        builder.frontend_only();
    });
    CGPUTextureId to_import = (CGPUTextureId)1;
    CGPUTextureViewId to_import_view = (CGPUTextureViewId)1;
    auto back_buffer = graph->create_texture(
    [=](render_graph::RenderGraph&, render_graph::TextureBuilder& builder) {
        builder.set_name("backbuffer");
    });
    auto gbuffer0 = graph->create_texture(
    [](render_graph::RenderGraph&, render_graph::TextureBuilder& builder) {
        builder.set_name("gbuffer0")
        .allow_render_target()
        .format(CGPU_FORMAT_B8G8R8A8_UNORM);
    });
    auto gbuffer1 = graph->create_texture(
    [](render_graph::RenderGraph&, render_graph::TextureBuilder& builder) {
        builder.set_name("gbuffer1")
        .allow_render_target()
        .format(CGPU_FORMAT_B8G8R8A8_UNORM);
    });
    graph->add_render_pass(
    [=](render_graph::RenderGraph&, render_graph::RenderPassBuilder& builder) {
        builder.set_name("gbuffer_pass")
        .write(0, gbuffer0)
        .write(1, gbuffer1);
    },
    render_graph::RenderPassExecuteFunction());
    graph->add_render_pass(
    [=](render_graph::RenderGraph&, render_graph::RenderPassBuilder& builder) {
        builder.set_name("defer_lighting")
        .read(0, 0, gbuffer0)
        .read(0, 1, gbuffer1)
        .write(0, back_buffer);
    },
    render_graph::RenderPassExecuteFunction());
    render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph.gv");
    render_graph::RenderGraph::destroy(graph);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif