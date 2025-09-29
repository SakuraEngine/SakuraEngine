#include <SkrContainers/string.hpp>
#include "SkrDependencyGraph/dependency_graph.hpp"
#include <fstream>

#include "SkrTestFramework/framework.hpp"

struct GraphTest
{

};

class TestRDGNode : public skr::DependencyGraphNode
{
public:
    TestRDGNode(const char8_t* n)
        : DependencyGraphNode()
        , name(n)
    {
    }

    skr::String name;
};

TEST_CASE_METHOD(GraphTest, "DependencyGraph")
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
    SKR_TEST_INFO(u8"{}", rdg->outgoing_edges(&node0))
    SKR_TEST_INFO(u8"{}", rdg->outgoing_edges(&node2))
    using Node = skr::DependencyGraphNode;
    using Edge = skr::DependencyGraphEdge;
    rdg->foreach_incoming_edges(&node1, [](Node* from, Node* to, Edge* e) {
        SKR_TEST_INFO(u8"edge: {} -> {}", 
            (const char8_t*)((TestRDGNode*)from)->name.c_str(), 
            (const char8_t*)((TestRDGNode*)to)->name.c_str()
        );
    });
    skr::DependencyGraph::Destroy(rdg);
}

#include "SkrRenderGraph/frontend/render_graph.hpp"

TEST_CASE_METHOD(GraphTest, "RenderGraphFrontEnd")
{
    namespace RG = skr::RG;
    auto graph = RG::RenderGraph::create(
    [](RG::RenderGraphBuilder& builder) {
        builder.frontend_only();
    });
    auto back_buffer = graph->create_texture(
    [=](RG::RenderGraph&, RG::TextureBuilder& builder) {
        builder.set_name(u8"backbuffer");
    });
    auto gbuffer0 = graph->create_texture(
    [](RG::RenderGraph&, RG::TextureBuilder& builder) {
        builder.set_name(u8"gbuffer0")
        .allow_render_target()
        .format(CGPU_FORMAT_B8G8R8A8_UNORM);
    });
    auto gbuffer1 = graph->create_texture(
    [](RG::RenderGraph&, RG::TextureBuilder& builder) {
        builder.set_name(u8"gbuffer1")
        .allow_render_target()
        .format(CGPU_FORMAT_B8G8R8A8_UNORM);
    });
    graph->add_render_pass(
    [=](RG::RenderGraph&, RG::RenderPassBuilder& builder) {
        builder.set_name(u8"gbuffer_pass")
        .write(0, gbuffer0)
        .write(1, gbuffer1);
    },
    RG::RenderPassExecuteFunction());
    graph->add_render_pass(
    [=](RG::RenderGraph&, RG::RenderPassBuilder& builder) {
        builder.set_name(u8"defer_lighting")
        .read(u8"GBuffer0", gbuffer0)
        .read(u8"GBuffer1", gbuffer1)
        .write(0, back_buffer);
    },
    RG::RenderPassExecuteFunction());
    RG::RenderGraph::destroy(graph);
}