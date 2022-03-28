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
public:
    friend class RenderGraphViz;
    class RenderGraphBuilder
    {
    public:
        friend class RenderGraph;
        RenderGraphBuilder& frontend_only();
        RenderGraphBuilder& backend_api(ECGpuBackend backend);
        RenderGraphBuilder& with_device(CGpuDeviceId device);
        RenderGraphBuilder& with_gfx_queue(CGpuQueueId queue);

    protected:
        bool no_backend;
        ECGpuBackend api;
        CGpuDeviceId device;
        CGpuQueueId gfx_queue;
    };
    using RenderGraphSetupFunction = eastl::function<void(class RenderGraph::RenderGraphBuilder&)>;
    static RenderGraph* create(const RenderGraphSetupFunction& setup);
    class RenderPassBuilder
    {
    public:
        friend class RenderGraph;
        RenderPassBuilder& set_name(const char* name);
        RenderPassBuilder& read(uint32_t set, uint32_t binding, TextureHandle handle);
        RenderPassBuilder& write(uint32_t mrt_index, TextureHandle handle);
        RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& set_pipeline(CGpuRenderPipelineId pipeline);

    protected:
        inline void Apply()
        {
        }
        RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass)
            : graph(graph)
            , node(pass)
        {
        }
        RenderGraph& graph;
        RenderPassNode& node;
    };
    using RenderPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::RenderPassBuilder&)>;
    inline PassHandle add_pass(const RenderPassSetupFunction& setup, const PassExecuteFunction& executor)
    {
        auto newPass = new RenderPassNode();
        passes.emplace_back(newPass);
        graph->insert(newPass);
        // build up
        RenderPassBuilder builder(*this, *newPass);
        setup(*this, builder);
        builder.Apply();
        newPass->executor = executor;
        return newPass->get_handle();
    }
    class TextureBuilder
    {
    public:
        friend class RenderGraph;
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
    using TextureSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::TextureBuilder&)>;
    inline TextureHandle create_texture(const TextureSetupFunction& setup)
    {
        auto newTex = new TextureNode();
        resources.emplace_back(newTex);
        graph->insert(newTex);
        TextureBuilder builder(*this, *newTex);
        setup(*this, builder);
        builder.Apply();
        return newTex->get_handle();
    }
    inline TextureHandle get_texture(const char* name)
    {
        return -1;
    }
    bool compile();
    virtual uint64_t execute();

protected:
    uint64_t frame_index = 0;
    Blackboard blackboard;
    eastl::unique_ptr<DependencyGraph> graph =
        eastl::unique_ptr<DependencyGraph>(DependencyGraph::Create());
    eastl::vector<PassNode*> passes;
    eastl::vector<ResourceNode*> resources;
};
using RenderGraphSetupFunction = RenderGraph::RenderGraphSetupFunction;
using RenderGraphBuilder = RenderGraph::RenderGraphBuilder;
using RenderPassSetupFunction = RenderGraph::RenderPassSetupFunction;
using RenderPassBuilder = RenderGraph::RenderPassBuilder;
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
// graph builder
inline RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_device(CGpuDeviceId device_)
{
    device = device_;
    return *this;
}
inline RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_gfx_queue(CGpuQueueId queue)
{
    gfx_queue = queue;
    return *this;
}
inline RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::backend_api(ECGpuBackend backend)
{
    api = backend;
    return *this;
}
inline RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::frontend_only()
{
    no_backend = true;
    return *this;
}
// pass builder
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, TextureHandle handle)
{
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(set, binding, handle));
    graph.graph->link(graph.graph->access_node(handle.handle), &node, edge);
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(uint32_t mrt_index, TextureHandle handle)
{
    auto&& edge = node.out_edges.emplace_back(
        new TextureRenderEdge(mrt_index, handle));
    graph.graph->link(&node, graph.graph->access_node(handle.handle), edge);
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_pipeline(CGpuRenderPipelineId pipeline)
{
    node.pipeline = pipeline;
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
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::set_name(const char* name)
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