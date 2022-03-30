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
    static void destroy(RenderGraph* g);
    class RenderPassBuilder
    {
    public:
        friend class RenderGraph;
        RenderPassBuilder& set_name(const char* name);
        RenderPassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle);
        RenderPassBuilder& write(uint32_t mrt_index, TextureRTVHandle handle);
        RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& set_pipeline(CGpuRenderPipelineId pipeline);

    protected:
        inline void Apply() {}
        RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass)
            : graph(graph)
            , node(pass)
        {
        }
        RenderGraph& graph;
        RenderPassNode& node;
    };
    using RenderPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::RenderPassBuilder&)>;
    inline PassHandle add_render_pass(const RenderPassSetupFunction& setup, const RenderPassExecuteFunction& executor)
    {
        auto newPass = new RenderPassNode(passes.size());
        passes.emplace_back(newPass);
        graph->insert(newPass);
        // build up
        RenderPassBuilder builder(*this, *newPass);
        setup(*this, builder);
        builder.Apply();
        newPass->executor = executor;
        return newPass->get_handle();
    }
    class PresentPassBuilder
    {
    public:
        friend class RenderGraph;

        PresentPassBuilder& set_name(const char* name);
        PresentPassBuilder& swapchain(CGpuSwapChainId chain, uint32_t idnex);
        PresentPassBuilder& texture(TextureHandle texture, bool is_backbuffer = true);

    protected:
        inline void Apply() {}
        PresentPassBuilder(RenderGraph& graph, PresentPassNode& present)
            : graph(graph)
            , node(present)
        {
        }
        RenderGraph& graph;
        PresentPassNode& node;
    };
    using PresentPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::PresentPassBuilder&)>;
    inline PassHandle add_present_pass(const PresentPassSetupFunction& setup)
    {
        auto newPass = new PresentPassNode(passes.size());
        passes.emplace_back(newPass);
        graph->insert(newPass);
        // build up
        PresentPassBuilder builder(*this, *newPass);
        setup(*this, builder);
        builder.Apply();
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
    const ECGpuResourceState get_lastest_state(TextureHandle texture, PassHandle pending_pass) const;

    bool compile();
    virtual uint64_t execute();

protected:
    virtual void initialize();
    virtual void finalize();
    virtual ~RenderGraph() = default;

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
using PresentPassSetupFunction = RenderGraph::PresentPassSetupFunction;
using PresentPassBuilder = RenderGraph::PresentPassBuilder;
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
// present pass builder
inline RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
inline RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::swapchain(CGpuSwapChainId chain, uint32_t index)
{
    node.descriptor.swapchain = chain;
    node.descriptor.index = index;
    return *this;
}
inline RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::texture(TextureHandle handle, bool is_backbuffer)
{
    assert(is_backbuffer && "blit to screen mode not supported!");
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(0, 0, handle, 0, 1, 0, 1,
            RESOURCE_STATE_PRESENT));
    graph.graph->link(graph.graph->access_node(handle.handle), &node, edge);
    return *this;
}
// render pass builder
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
// textures read/write
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, TextureSRVHandle handle)
{
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(set, binding, handle._this,
            handle.mip_base, handle.mip_count,
            handle.array_base, handle.array_count));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(
    uint32_t mrt_index, TextureRTVHandle handle)
{
    auto&& edge = node.out_edges.emplace_back(
        new TextureRenderEdge(mrt_index, handle._this));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.load_actions[mrt_index] = handle.load_act;
    node.store_actions[mrt_index] = handle.store_act;
    return *this;
}
// buffers read/write
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
    tex_desc.width = texture->width;
    tex_desc.height = texture->height;
    tex_desc.depth = texture->depth;
    tex_desc.format = (ECGpuFormat)texture->format;
    tex_desc.array_size = texture->array_size_minus_one + 1;
    tex_desc.sample_count = texture->sample_count;
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
    tex_desc.descriptors |= RT_RENDER_TARGET;
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