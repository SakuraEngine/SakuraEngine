#pragma once
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include "render_graph/frontend/blackboard.hpp"
#include "render_graph/frontend/resource_node.h"
#include "render_graph/frontend/pass_node.h"

namespace sakura
{
namespace render_graph
{

class RenderGraph
{
    friend class RenderGraphViz;

public:
    class PassBuilder
    {
        friend class RenderGraph;

    public:
        inline PassBuilder& set_name(const char* name)
        {
            if (name)
            {
                graph.blackboard.named_passes[name] = &pass;
                pass.set_name(name);
            }
            return *this;
        }
        inline PassBuilder& read(uint32_t set, uint32_t binding, TextureHandle handle)
        {
            auto&& edge = pass.in_edges.emplace_back(
                new TextureReferenceEdge(set, binding, handle));
            graph.graph->link(graph.graph->access_node(handle.handle), &pass, edge);
            return *this;
        }
        inline PassBuilder& write(uint32_t mrt_index, TextureHandle handle)
        {
            auto&& edge = pass.out_edges.emplace_back(
                new TextureAccessEdge(mrt_index, handle));
            graph.graph->link(&pass, graph.graph->access_node(handle.handle), edge);
            return *this;
        }
        inline PassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle)
        {
            return *this;
        }
        inline PassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle)
        {
            return *this;
        }

    protected:
        PassBuilder(RenderGraph& graph, RenderPassNode& pass)
            : graph(graph)
            , pass(pass)
        {
        }
        RenderGraph& graph;
        RenderPassNode& pass;
    };
    using PassSetupFunction = eastl::function<void(class RenderGraph::PassBuilder&)>;
    inline PassHandle add_pass(const PassSetupFunction& setup, const PassExecuteFunction& executor)
    {
        auto newPass = new RenderPassNode();
        passes.emplace_back(newPass);
        graph->insert(newPass);
        // build up
        PassBuilder builder(*this, *newPass);
        setup(builder);
        newPass->executor = executor;
        return newPass->get_handle();
    }
    inline TextureHandle create_texture(const char* name = nullptr)
    {
        auto newTex = new TextureNode();
        resources.emplace_back(newTex);
        graph->insert(newTex);
        // blackboard
        if (name)
        {
            blackboard.named_textures[name] = newTex;
            newTex->set_name(name);
        }
        return newTex->get_handle();
    }

protected:
    Blackboard blackboard;
    eastl::unique_ptr<DependencyGraph> graph =
        eastl::unique_ptr<DependencyGraph>(DependencyGraph::Create());
    eastl::vector<PassNode*> passes;
    eastl::vector<ResourceNode*> resources;
};

using PassSetupFunction = RenderGraph::PassSetupFunction;

class RenderGraphViz
{
public:
    static void write_graphviz(RenderGraph& graph, const char* outf);
};
} // namespace render_graph
} // namespace sakura
