#include "gtest/gtest.h"
#include <containers/string.hpp>
#include "SkrRT/misc/dependency_graph.hpp"
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


class TestRDGNode : public skr::DependencyGraphNode
{
public:
    TestRDGNode(const char8_t* n)
        : DependencyGraphNode()
        , name(n)
    {
    }

    skr::string name;
};

TEST(GraphTest, DependencyGraph)
{
    skr::DependencyGraphEdge edge;
    TestRDGNode node0(u8"node0");
    TestRDGNode node1(u8"node1");
    TestRDGNode node2(u8"node2");
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
        builder.set_name(u8"backbuffer");
    });
    auto gbuffer0 = graph->create_texture(
    [](render_graph::RenderGraph&, render_graph::TextureBuilder& builder) {
        builder.set_name(u8"gbuffer0")
        .allow_render_target()
        .format(CGPU_FORMAT_B8G8R8A8_UNORM);
    });
    auto gbuffer1 = graph->create_texture(
    [](render_graph::RenderGraph&, render_graph::TextureBuilder& builder) {
        builder.set_name(u8"gbuffer1")
        .allow_render_target()
        .format(CGPU_FORMAT_B8G8R8A8_UNORM);
    });
    graph->add_render_pass(
    [=](render_graph::RenderGraph&, render_graph::RenderPassBuilder& builder) {
        builder.set_name(u8"gbuffer_pass")
        .write(0, gbuffer0)
        .write(1, gbuffer1);
    },
    render_graph::RenderPassExecuteFunction());
    graph->add_render_pass(
    [=](render_graph::RenderGraph&, render_graph::RenderPassBuilder& builder) {
        builder.set_name(u8"defer_lighting")
        .read(u8"GBuffer0", gbuffer0)
        .read(u8"GBuffer1", gbuffer1)
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