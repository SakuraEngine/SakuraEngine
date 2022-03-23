#pragma once
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include "render_graph/frontend/blackboard.hpp"
#include "render_graph/frontend/resource_node.hpp"
#include "render_graph/frontend/resource_edge.hpp"
#include "render_graph/frontend/pass_node.hpp"

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
        PassBuilder& set_name(const char* name);
        PassBuilder& read(uint32_t set, uint32_t binding, TextureHandle handle);
        PassBuilder& write(uint32_t mrt_index, TextureHandle handle);
        PassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle);
        PassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle);

    protected:
        inline void Apply()
        {
        }
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
        builder.Apply();
        newPass->executor = executor;
        return newPass->get_handle();
    }
    class TextureBuilder
    {
        friend class RenderGraph;

    public:
        TextureBuilder& import(CGpuTextureId texture);
        TextureBuilder& extent(uint32_t width, uint32_t height, uint32_t depth = 1);
        TextureBuilder& format(ECGpuFormat format);
        TextureBuilder& array(uint32_t size);
        TextureBuilder& sample_count(ECGpuSampleCount count);
        TextureBuilder& allow_render_target();
        TextureBuilder& set_name(const char* name);
        TextureBuilder& owns_memory();
        TextureBuilder& create_immediate();
        TextureBuilder& create_async();
        TextureBuilder& allow_lone();

    protected:
        TextureBuilder(RenderGraph& graph, TextureNode& node)
            : graph(graph)
            , node(node)
        {
            tex_desc.descriptors = RT_TEXTURE;
        }
        inline void Apply()
        {
            node.imported = imported;
            if (imported) node.frame_texture = imported;
            node.canbe_lone = canbe_lone;
            node.descriptor = tex_desc;
            // TODO: create underlying resource
        }
        RenderGraph& graph;
        TextureNode& node;
        CGpuTextureDescriptor tex_desc = {};
        CGpuTextureId imported = nullptr;
        bool create_at_once : 1;
        bool async : 1;
        bool canbe_lone : 1;
    };
    using TextureSetupFunction = eastl::function<void(class RenderGraph::TextureBuilder&)>;
    inline TextureHandle create_texture(const TextureSetupFunction& setup)
    {
        auto newTex = new TextureNode();
        resources.emplace_back(newTex);
        graph->insert(newTex);
        TextureBuilder builder(*this, *newTex);
        setup(builder);
        builder.Apply();
        return newTex->get_handle();
    }
    inline TextureHandle get_texture(const char* name)
    {
        return -1;
    }

protected:
    Blackboard blackboard;
    eastl::unique_ptr<DependencyGraph> graph =
        eastl::unique_ptr<DependencyGraph>(DependencyGraph::Create());
    eastl::vector<PassNode*> passes;
    eastl::vector<ResourceNode*> resources;
};

using PassSetupFunction = RenderGraph::PassSetupFunction;
using PassBuilder = RenderGraph::PassBuilder;
using TextureSetupFunction = RenderGraph::TextureSetupFunction;
using TextureBuilder = RenderGraph::TextureBuilder;

class RenderGraphViz
{
public:
    static void write_graphviz(RenderGraph& graph, const char* outf);
};
} // namespace render_graph
} // namespace sakura

// implementation
namespace sakura
{
namespace render_graph
{
// pass builder
inline RenderGraph::PassBuilder& RenderGraph::PassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &pass;
        pass.set_name(name);
    }
    return *this;
}
inline RenderGraph::PassBuilder& RenderGraph::PassBuilder::read(uint32_t set, uint32_t binding, TextureHandle handle)
{
    auto&& edge = pass.in_edges.emplace_back(
        new TextureReadEdge(set, binding, handle));
    graph.graph->link(graph.graph->access_node(handle.handle), &pass, edge);
    return *this;
}
inline RenderGraph::PassBuilder& RenderGraph::PassBuilder::write(uint32_t mrt_index, TextureHandle handle)
{
    auto&& edge = pass.out_edges.emplace_back(
        new TextureRenderEdge(mrt_index, handle));
    graph.graph->link(&pass, graph.graph->access_node(handle.handle), edge);
    return *this;
}
inline RenderGraph::PassBuilder& RenderGraph::PassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::PassBuilder& RenderGraph::PassBuilder::write(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}

// texture builder
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::import(CGpuTextureId texture)
{
    imported = texture;
    return *this;
}

inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::extent(
    uint32_t width, uint32_t height, uint32_t depth)
{
    tex_desc.width = width;
    tex_desc.height = height;
    tex_desc.depth = depth;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::format(
    ECGpuFormat format)
{
    tex_desc.format = format;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::array(uint32_t size)
{
    tex_desc.array_size = size;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::sample_count(
    ECGpuSampleCount count)
{
    tex_desc.sample_count = count;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_render_target()
{
    tex_desc.start_state = RESOURCE_STATE_RENDER_TARGET;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::set_name(
    const char* name)
{
    tex_desc.name = name;
    // blackboard
    graph.blackboard.named_textures[name] = &node;
    node.set_name(name);
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::owns_memory()
{
    tex_desc.flags |= TCF_OWN_MEMORY_BIT;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::create_immediate()
{
    create_at_once = true;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::create_async()
{
    async = 1;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_lone()
{
    canbe_lone = true;
    return *this;
}
} // namespace render_graph
} // namespace sakura