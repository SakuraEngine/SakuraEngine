#pragma once
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include "render_graph/frontend/blackboard.hpp"
#include "render_graph/frontend/resource_node.hpp"
#include "render_graph/frontend/resource_edge.hpp"
#include "render_graph/frontend/pass_node.hpp"

#define RG_MAX_FRAME_IN_FLIGHT 3

namespace sakura
{
namespace render_graph
{
class RenderGraphProfiler
{
public:
    virtual ~RenderGraphProfiler() = default;
    virtual void on_acquire_executor(class RenderGraph&, class RenderGraphFrameExecutor&) {}
    virtual void on_cmd_begin(class RenderGraph&, class RenderGraphFrameExecutor&) {}
    virtual void on_cmd_end(class RenderGraph&, class RenderGraphFrameExecutor&) {}
    virtual void on_pass_begin(class RenderGraph&, class RenderGraphFrameExecutor&, class PassNode& pass) {}
    virtual void on_pass_end(class RenderGraph&, class RenderGraphFrameExecutor&, class PassNode& pass) {}
    virtual void before_commit(class RenderGraph&, class RenderGraphFrameExecutor&) {}
    virtual void after_commit(class RenderGraph&, class RenderGraphFrameExecutor&) {}
};

class RenderGraph
{
public:
    friend class RenderGraphViz;
    class RenderGraphBuilder
    {
    public:
        friend class RenderGraph;
        friend class RenderGraphBackend;
        RenderGraphBuilder& frontend_only();
        RenderGraphBuilder& backend_api(ECGPUBackend backend);
        RenderGraphBuilder& with_device(CGPUDeviceId device);
        RenderGraphBuilder& with_gfx_queue(CGPUQueueId queue);
        RenderGraphBuilder& enable_memory_aliasing();

    protected:
        bool memory_aliasing = false;
        bool no_backend;
        ECGPUBackend api;
        CGPUDeviceId device;
        CGPUQueueId gfx_queue;
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
        ECGPULoadAction load_action = CGPU_LOAD_ACTION_CLEAR,
        ECGPUStoreAction store_action = CGPU_STORE_ACTION_STORE);
        RenderPassBuilder& set_depth_stencil(TextureDSVHandle handle,
        ECGPULoadAction dload_action = CGPU_LOAD_ACTION_CLEAR,
        ECGPUStoreAction dstore_action = CGPU_STORE_ACTION_STORE,
        ECGPULoadAction sload_action = CGPU_LOAD_ACTION_CLEAR,
        ECGPUStoreAction sstore_action = CGPU_STORE_ACTION_STORE);
        // buffers
        RenderPassBuilder& read(const char8_t* name, BufferHandle handle);
        RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle);
        RenderPassBuilder& write(const char8_t* name, BufferHandle handle);
        RenderPassBuilder& use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state);

        RenderPassBuilder& set_pipeline(CGPURenderPipelineId pipeline);

    protected:
        RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass) noexcept;
        RenderGraph& graph;
        RenderPassNode& node;
    };
    using RenderPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::RenderPassBuilder&)>;
    PassHandle add_render_pass(const RenderPassSetupFunction& setup, const RenderPassExecuteFunction& executor);

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
        ComputePassBuilder& set_pipeline(CGPUComputePipelineId pipeline);

    protected:
        ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass) noexcept;
        RenderGraph& graph;
        ComputePassNode& node;
    };
    using ComputePassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::ComputePassBuilder&)>;
    PassHandle add_compute_pass(const ComputePassSetupFunction& setup, const ComputePassExecuteFunction& executor);

    class CopyPassBuilder
    {
    public:
        friend class RenderGraph;
        CopyPassBuilder& set_name(const char* name);
        CopyPassBuilder& texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst);
        CopyPassBuilder& buffer_to_buffer(BufferRangeHandle src, BufferRangeHandle dst);

    protected:
        CopyPassBuilder(RenderGraph& graph, CopyPassNode& pass) noexcept;
        RenderGraph& graph;
        CopyPassNode& node;
    };
    using CopyPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::CopyPassBuilder&)>;
    PassHandle add_copy_pass(const CopyPassSetupFunction& setup);

    class PresentPassBuilder
    {
    public:
        friend class RenderGraph;

        PresentPassBuilder& set_name(const char* name);
        PresentPassBuilder& swapchain(CGPUSwapChainId chain, uint32_t idnex);
        PresentPassBuilder& texture(TextureHandle texture, bool is_backbuffer = true);

    protected:
        PresentPassBuilder(RenderGraph& graph, PresentPassNode& present) noexcept;
        RenderGraph& graph;
        PresentPassNode& node;
    };
    using PresentPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::PresentPassBuilder&)>;
    PassHandle add_present_pass(const PresentPassSetupFunction& setup);

    class BufferBuilder
    {
    public:
        friend class RenderGraph;
        BufferBuilder& set_name(const char* name);
        BufferBuilder& import(CGPUBufferId buffer, ECGPUResourceState init_state);
        BufferBuilder& owns_memory();
        BufferBuilder& structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride);
        BufferBuilder& size(uint64_t size);
        BufferBuilder& memory_usage(ECGPUMemoryUsage mem_usage);
        BufferBuilder& allow_shader_readwrite();
        BufferBuilder& allow_shader_read();
        BufferBuilder& as_upload_buffer();
        BufferBuilder& as_vertex_buffer();
        BufferBuilder& as_index_buffer();

    protected:
        BufferBuilder(RenderGraph& graph, BufferNode& node) noexcept;
        RenderGraph& graph;
        BufferNode& node;
    };
    using BufferSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::BufferBuilder&)>;
    BufferHandle create_buffer(const BufferSetupFunction& setup);
    inline BufferHandle get_buffer(const char* name);
    const ECGPUResourceState get_lastest_state(const BufferNode* buffer, const PassNode* pending_pass) const;

    class TextureBuilder
    {
    public:
        friend class RenderGraph;
        TextureBuilder& set_name(const char* name);
        TextureBuilder& import(CGPUTextureId texture, ECGPUResourceState init_state);
        TextureBuilder& extent(uint32_t width, uint32_t height, uint32_t depth = 1);
        TextureBuilder& format(ECGPUFormat format);
        TextureBuilder& array(uint32_t size);
        TextureBuilder& sample_count(ECGPUSampleCount count);
        TextureBuilder& allow_render_target();
        TextureBuilder& allow_depth_stencil();
        TextureBuilder& allow_readwrite();
        TextureBuilder& owns_memory();
        TextureBuilder& allow_lone();

    protected:
        TextureBuilder(RenderGraph& graph, TextureNode& node) noexcept;
        RenderGraph& graph;
        TextureNode& node;
        CGPUTextureId imported = nullptr;
    };
    using TextureSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::TextureBuilder&)>;
    TextureHandle create_texture(const TextureSetupFunction& setup);
    TextureHandle get_texture(const char* name);
    const ECGPUResourceState get_lastest_state(const TextureNode* texture, const PassNode* pending_pass) const;

    bool compile();
    virtual uint64_t execute(RenderGraphProfiler* profiler = nullptr);
    virtual CGPUDeviceId get_backend_device() { return nullptr; }
    virtual CGPUQueueId get_gfx_queue() { return nullptr; }
    virtual uint32_t collect_garbage(uint64_t critical_frame)
    {
        return collect_texture_garbage(critical_frame) + collect_buffer_garbage(critical_frame);
    }
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame) { return 0; }
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame) { return 0; }

    inline BufferNode* resolve(BufferHandle hdl) { return static_cast<BufferNode*>(graph->node_at(hdl)); }
    inline TextureNode* resolve(TextureHandle hdl) { return static_cast<TextureNode*>(graph->node_at(hdl)); }
    inline PassNode* resolve(PassHandle hdl) { return static_cast<PassNode*>(graph->node_at(hdl)); }
    inline uint64_t get_frame_index() const { return frame_index; }

    inline bool enable_memory_aliasing(bool enabled)
    {
        aliasing_enabled = enabled;
        return aliasing_enabled;
    }

protected:
    uint32_t foreach_textures(eastl::function<void(TextureNode*)> texture);
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

    RenderGraph(const RenderGraphBuilder& builder);
    virtual ~RenderGraph() = default;

    bool aliasing_enabled;
    uint64_t frame_index = 0;
    Blackboard blackboard;
    eastl::unique_ptr<DependencyGraph> graph =
    eastl::unique_ptr<DependencyGraph>(DependencyGraph::Create());
    eastl::vector<PassNode*> passes;
    eastl::vector<ResourceNode*> resources;
    eastl::vector<PassNode*> culled_passes;
    eastl::vector<ResourceNode*> culled_resources;
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