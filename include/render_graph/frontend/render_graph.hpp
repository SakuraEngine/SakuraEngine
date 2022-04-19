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
        RenderPassBuilder& use_buffer(PipelineBufferHandle buffer, ECGpuResourceState requested_state);

        RenderPassBuilder& set_pipeline(CGpuRenderPipelineId pipeline);

    protected:
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
        newPass->executor = executor;
        return newPass->get_handle();
    }
    class CopyPassBuilder
    {
    public:
        friend class RenderGraph;
        CopyPassBuilder& set_name(const char* name);
        CopyPassBuilder& texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst);
        CopyPassBuilder& buffer_to_buffer(BufferRangeHandle src, BufferRangeHandle dst);

    protected:
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
        return newPass->get_handle();
    }
    class BufferBuilder
    {
    public:
        friend class RenderGraph;
        BufferBuilder& set_name(const char* name);
        BufferBuilder& import(CGpuBufferId buffer, ECGpuResourceState init_state);
        BufferBuilder& owns_memory();
        BufferBuilder& structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride);
        BufferBuilder& size(uint64_t size);
        BufferBuilder& memory_usage(ECGpuMemoryUsage mem_usage);
        BufferBuilder& allow_shader_readwrite();
        BufferBuilder& allow_shader_read();
        BufferBuilder& as_upload_buffer();
        BufferBuilder& as_vertex_buffer();
        BufferBuilder& as_index_buffer();

    protected:
        BufferBuilder(RenderGraph& graph, BufferNode& node)
            : graph(graph)
            , node(node)
        {
            node.descriptor.descriptors = RT_NONE;
            node.descriptor.flags = BCF_NONE;
            node.descriptor.memory_usage = MEM_USAGE_GPU_ONLY;
        }
        RenderGraph& graph;
        BufferNode& node;
    };
    using BufferSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::BufferBuilder&)>;
    inline BufferHandle create_buffer(const BufferSetupFunction& setup)
    {
        auto newTex = new BufferNode();
        resources.emplace_back(newTex);
        graph->insert(newTex);
        BufferBuilder builder(*this, *newTex);
        setup(*this, builder);
        return newTex->get_handle();
    }
    inline BufferHandle get_buffer(const char* name)
    {
        if (blackboard.named_buffers.find(name) != blackboard.named_buffers.end())
            return blackboard.named_buffers[name]->get_handle();
        return UINT64_MAX;
    }
    const ECGpuResourceState get_lastest_state(const BufferNode* buffer, const PassNode* pending_pass) const;

    class TextureBuilder
    {
    public:
        friend class RenderGraph;
        TextureBuilder& set_name(const char* name);
        TextureBuilder& import(CGpuTextureId texture, ECGpuResourceState init_state);
        TextureBuilder& extent(uint32_t width, uint32_t height, uint32_t depth = 1);
        TextureBuilder& format(ECGpuFormat format);
        TextureBuilder& array(uint32_t size);
        TextureBuilder& sample_count(ECGpuSampleCount count);
        TextureBuilder& allow_render_target();
        TextureBuilder& allow_depth_stencil();
        TextureBuilder& allow_readwrite();
        TextureBuilder& owns_memory();
        TextureBuilder& allow_lone();

    protected:
        TextureBuilder(RenderGraph& graph, TextureNode& node)
            : graph(graph)
            , node(node)
        {
            node.descriptor.descriptors = RT_TEXTURE;
        }
        RenderGraph& graph;
        TextureNode& node;
        CGpuTextureId imported = nullptr;
    };
    using TextureSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::TextureBuilder&)>;
    inline TextureHandle create_texture(const TextureSetupFunction& setup)
    {
        auto newTex = new TextureNode();
        resources.emplace_back(newTex);
        graph->insert(newTex);
        TextureBuilder builder(*this, *newTex);
        setup(*this, builder);
        return newTex->get_handle();
    }
    inline TextureHandle get_texture(const char* name)
    {
        if (blackboard.named_textures.find(name) != blackboard.named_textures.end())
            return blackboard.named_textures[name]->get_handle();
        return UINT64_MAX;
    }
    const ECGpuResourceState get_lastest_state(const TextureNode* texture, const PassNode* pending_pass) const;

    bool compile();
    virtual uint64_t execute();
    virtual CGpuDeviceId get_backend_device() { return nullptr; }
    virtual CGpuQueueId get_gfx_queue() { return nullptr; }
    virtual uint32_t collect_garbage(uint64_t critical_frame)
    {
        return collect_texture_garbage(critical_frame) +
               collect_buffer_garbage(critical_frame);
    }
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame) { return 0; }
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame) { return 0; }

    inline BufferNode* resolve(BufferHandle hdl) { return static_cast<BufferNode*>(graph->node_at(hdl)); }
    inline TextureNode* resolve(TextureHandle hdl) { return static_cast<TextureNode*>(graph->node_at(hdl)); }
    inline PassNode* resolve(PassHandle hdl) { return static_cast<PassNode*>(graph->node_at(hdl)); }

    uint32_t foreach_writer_passes(TextureHandle texture,
        eastl::function<void(PassNode* writer, TextureNode* tex, RenderGraphEdge* edge)>) const;
    uint32_t foreach_reader_passes(TextureHandle texture,
        eastl::function<void(PassNode* reader, TextureNode* tex, RenderGraphEdge* edge)>) const;
    uint32_t foreach_writer_passes(BufferHandle buffer,
        eastl::function<void(PassNode* writer, BufferNode* buf, RenderGraphEdge* edge)>) const;
    uint32_t foreach_reader_passes(BufferHandle buffer,
        eastl::function<void(PassNode* reader, BufferNode* buf, RenderGraphEdge* edge)>) const;

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
using BufferSetupFunction = RenderGraph::BufferSetupFunction;
using BufferBuilder = RenderGraph::BufferBuilder;

class RenderGraphViz
{
public:
    static void write_graphviz(RenderGraph& graph, const char* outf);
};
} // namespace render_graph
} // namespace sakura