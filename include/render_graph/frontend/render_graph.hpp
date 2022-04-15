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
        // textures
        RenderPassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle);
        RenderPassBuilder& read(const char8_t* name, TextureSRVHandle handle);
        RenderPassBuilder& write(uint32_t mrt_index, TextureRTVHandle handle,
            ECGpuLoadAction load_action = LOAD_ACTION_CLEAR,
            ECGpuStoreAction store_action = STORE_ACTION_STORE);
        RenderPassBuilder& set_depth_stencil(TextureDSVHandle handle,
            ECGpuLoadAction dload_action = LOAD_ACTION_CLEAR,
            ECGpuStoreAction dstore_action = STORE_ACTION_STORE,
            ECGpuLoadAction sload_action = LOAD_ACTION_CLEAR,
            ECGpuStoreAction sstore_action = STORE_ACTION_STORE);
        // buffers
        RenderPassBuilder& read(const char8_t* name, BufferHandle handle);
        RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& write(const char8_t* name, BufferHandle handle);
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
    class ComputePassBuilder
    {
    public:
        friend class RenderGraph;
        ComputePassBuilder& set_name(const char* name);
        ComputePassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle);
        ComputePassBuilder& read(const char8_t* name, TextureSRVHandle handle);
        ComputePassBuilder& readwrite(uint32_t set, uint32_t binding, TextureUAVHandle handle);
        ComputePassBuilder& readwrite(const char8_t* name, TextureUAVHandle handle);
        ComputePassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle);
        ComputePassBuilder& read(const char8_t* name, BufferHandle handle);
        ComputePassBuilder& readwrite(uint32_t set, uint32_t binding, BufferHandle handle);
        ComputePassBuilder& readwrite(const char8_t* name, BufferHandle handle);
        ComputePassBuilder& set_pipeline(CGpuComputePipelineId pipeline);

    protected:
        inline void Apply() {}
        ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass)
            : graph(graph)
            , node(pass)
        {
        }
        RenderGraph& graph;
        ComputePassNode& node;
    };
    using ComputePassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::ComputePassBuilder&)>;
    inline PassHandle add_compute_pass(const ComputePassSetupFunction& setup, const ComputePassExecuteFunction& executor)
    {
        auto newPass = new ComputePassNode(passes.size());
        passes.emplace_back(newPass);
        graph->insert(newPass);
        // build up
        ComputePassBuilder builder(*this, *newPass);
        setup(*this, builder);
        builder.Apply();
        newPass->executor = executor;
        return newPass->get_handle();
    }
    class CopyPassBuilder
    {
    public:
        friend class RenderGraph;
        CopyPassBuilder& set_name(const char* name);
        CopyPassBuilder& texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst);

    protected:
        inline void Apply() {}
        CopyPassBuilder(RenderGraph& graph, CopyPassNode& pass)
            : graph(graph)
            , node(pass)
        {
        }
        RenderGraph& graph;
        CopyPassNode& node;
    };
    using CopyPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::CopyPassBuilder&)>;
    inline PassHandle add_copy_pass(const CopyPassSetupFunction& setup)
    {
        auto newPass = new CopyPassNode(passes.size());
        passes.emplace_back(newPass);
        graph->insert(newPass);
        // build up
        CopyPassBuilder builder(*this, *newPass);
        setup(*this, builder);
        builder.Apply();
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
        TextureBuilder& import(CGpuTextureId texture, ECGpuResourceState init_state);
        TextureBuilder& extent(uint32_t width, uint32_t height, uint32_t depth = 1);
        TextureBuilder& format(ECGpuFormat format);
        TextureBuilder& array(uint32_t size);
        TextureBuilder& sample_count(ECGpuSampleCount count);
        TextureBuilder& allow_render_target();
        TextureBuilder& allow_depth_stencil();
        TextureBuilder& allow_readwrite();
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
    const ECGpuResourceState get_lastest_state(const TextureNode* texture, const PassNode* pending_pass) const;

    bool compile();
    virtual uint64_t execute();
    virtual CGpuDeviceId get_backend_device() { return nullptr; }
    virtual CGpuQueueId get_gfx_queue() { return nullptr; }
    virtual void collect_grabage(uint64_t critical_frame)
    {
        collect_texture_grabage(critical_frame);
        collect_buffer_grabage(critical_frame);
    }
    virtual void collect_texture_grabage(uint64_t critical_frame) {}
    virtual void collect_buffer_grabage(uint64_t critical_frame) {}

    inline TextureNode* resolve(TextureHandle hdl) { return static_cast<TextureNode*>(graph->node_at(hdl)); }
    inline PassNode* resolve(PassHandle hdl) { return static_cast<PassNode*>(graph->node_at(hdl)); }
    uint32_t foreach_writer_passes(TextureHandle texture,
        eastl::function<void(PassNode* writer, TextureNode* tex, RenderGraphEdge* edge)>) const;
    uint32_t foreach_reader_passes(TextureHandle texture,
        eastl::function<void(PassNode* reader, TextureNode* tex, RenderGraphEdge* edge)>) const;
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
using ComputePassSetupFunction = RenderGraph::ComputePassSetupFunction;
using ComputePassBuilder = RenderGraph::ComputePassBuilder;
using CopyPassBuilder = RenderGraph::CopyPassBuilder;
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
        new TextureReadEdge(0, 0, handle, RESOURCE_STATE_PRESENT));
    graph.graph->link(graph.graph->access_node(handle), &node, edge);
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
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, TextureSRVHandle handle)
{
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(set, binding, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, TextureSRVHandle handle)
{
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(name, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(
    uint32_t mrt_index, TextureRTVHandle handle, ECGpuLoadAction load_action,
    ECGpuStoreAction store_action)
{
    auto&& edge = node.out_edges.emplace_back(
        new TextureRenderEdge(mrt_index, handle._this));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.load_actions[mrt_index] = load_action;
    node.store_actions[mrt_index] = store_action;
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_depth_stencil(TextureDSVHandle handle,
    ECGpuLoadAction dload_action, ECGpuStoreAction dstore_action,
    ECGpuLoadAction sload_action, ECGpuStoreAction sstore_action)
{
    auto&& edge = node.out_edges.emplace_back(
        new TextureRenderEdge(
            MAX_MRT_COUNT, handle._this,
            RESOURCE_STATE_DEPTH_WRITE));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.depth_load_action = dload_action;
    node.depth_store_action = dstore_action;
    node.stencil_load_action = sload_action;
    node.stencil_store_action = sstore_action;
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(const char8_t* name, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_pipeline(CGpuRenderPipelineId pipeline)
{
    node.pipeline = pipeline;
    return *this;
}

// compute pass builder
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, TextureSRVHandle handle)
{
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(set, binding, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, TextureSRVHandle handle)
{
    auto&& edge = node.in_edges.emplace_back(
        new TextureReadEdge(name, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, TextureUAVHandle handle)
{
    auto&& edge = node.inout_edges.emplace_back(
        new TextureReadWriteEdge(set, binding, handle));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, TextureUAVHandle handle)
{
    auto&& edge = node.inout_edges.emplace_back(
        new TextureReadWriteEdge(name, handle));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, BufferHandle handle)
{
    return *this;
}
inline RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_pipeline(CGpuComputePipelineId pipeline)
{
    node.pipeline = pipeline;
    return *this;
}

// copy pass
inline RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
inline RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst)
{
    auto&& in_edge = node.in_edges.emplace_back(
        new TextureReadEdge(0, 0, src._this,
            RESOURCE_STATE_COPY_SOURCE));
    auto&& out_edge = node.out_edges.emplace_back(
        new TextureRenderEdge(0, dst._this,
            RESOURCE_STATE_COPY_DEST));
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this), out_edge);
    node.t2ts.emplace_back(src, dst);
    return *this;
}

// texture builder
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::import(CGpuTextureId texture, ECGpuResourceState init_state)
{
    imported = texture;
    node.init_state = init_state;
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
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_readwrite()
{
    tex_desc.descriptors |= RT_RW_TEXTURE;
    tex_desc.start_state = RESOURCE_STATE_UNDEFINED;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_render_target()
{
    tex_desc.descriptors |= RT_RENDER_TARGET;
    tex_desc.start_state = RESOURCE_STATE_UNDEFINED;
    return *this;
}
inline RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_depth_stencil()
{
    tex_desc.descriptors |= RT_DEPTH_STENCIL;
    tex_desc.start_state = RESOURCE_STATE_UNDEFINED;
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