#pragma once
#include <EASTL/vector.h>
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "SkrRenderGraph/frontend/blackboard.hpp"

#ifndef RG_MAX_FRAME_IN_FLIGHT
#define RG_MAX_FRAME_IN_FLIGHT 3
#endif

namespace skr
{
namespace render_graph
{
class SKR_RENDER_GRAPH_API RenderGraphProfiler
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

class SKR_RENDER_GRAPH_API RenderGraph
{
public:
    friend class RenderGraphViz;
    class SKR_RENDER_GRAPH_API RenderGraphBuilder
    {
    public:
        friend class RenderGraph;
        friend class RenderGraphBackend;
        RenderGraphBuilder& frontend_only() SKR_NOEXCEPT;
        RenderGraphBuilder& backend_api(ECGPUBackend backend) SKR_NOEXCEPT;
        RenderGraphBuilder& with_device(CGPUDeviceId device) SKR_NOEXCEPT;
        RenderGraphBuilder& with_gfx_queue(CGPUQueueId queue) SKR_NOEXCEPT;
        RenderGraphBuilder& enable_memory_aliasing() SKR_NOEXCEPT;

    protected:
        bool memory_aliasing = false;
        bool no_backend;
        ECGPUBackend api;
        CGPUDeviceId device;
        CGPUQueueId gfx_queue;
    };
    using RenderGraphSetupFunction = eastl::function<void(class RenderGraph::RenderGraphBuilder&)>;
    static RenderGraph* create(const RenderGraphSetupFunction& setup) SKR_NOEXCEPT;
    static void destroy(RenderGraph* g) SKR_NOEXCEPT;
    
    class SKR_RENDER_GRAPH_API RenderPassBuilder
    {
    public:
        friend class RenderGraph;
        RenderPassBuilder& set_name(const char* name) SKR_NOEXCEPT;
        // textures
        RenderPassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle) SKR_NOEXCEPT;
        RenderPassBuilder& read(const char8_t* name, TextureSRVHandle handle) SKR_NOEXCEPT;
        RenderPassBuilder& write(uint32_t mrt_index, TextureRTVHandle handle,
            ECGPULoadAction load_action = CGPU_LOAD_ACTION_CLEAR,
            CGPUClearValue clear_color = fastclear_0000,
            ECGPUStoreAction store_action = CGPU_STORE_ACTION_STORE) SKR_NOEXCEPT;
        RenderPassBuilder& resolve_msaa(uint32_t mrt_index, TextureSubresourceHandle handle);
        
        RenderPassBuilder& set_depth_stencil(TextureDSVHandle handle,
            ECGPULoadAction dload_action = CGPU_LOAD_ACTION_CLEAR,
            ECGPUStoreAction dstore_action = CGPU_STORE_ACTION_STORE,
            ECGPULoadAction sload_action = CGPU_LOAD_ACTION_CLEAR,
            ECGPUStoreAction sstore_action = CGPU_STORE_ACTION_STORE) SKR_NOEXCEPT;

        // buffers
        RenderPassBuilder& read(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT;
        RenderPassBuilder& read(uint32_t set, uint32_t binding, BufferRangeHandle handle) SKR_NOEXCEPT;
        RenderPassBuilder& write(uint32_t set, uint32_t binding, BufferHandle handle) SKR_NOEXCEPT;
        RenderPassBuilder& write(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT;
        RenderPassBuilder& use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state) SKR_NOEXCEPT;

        RenderPassBuilder& set_pipeline(CGPURenderPipelineId pipeline) SKR_NOEXCEPT;
        RenderPassBuilder& set_root_signature(CGPURootSignatureId signature) SKR_NOEXCEPT;
    protected:
        RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass) SKR_NOEXCEPT;
        RenderGraph& graph;
        RenderPassNode& node;
    };
    using RenderPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::RenderPassBuilder&)>;
    PassHandle add_render_pass(const RenderPassSetupFunction& setup, const RenderPassExecuteFunction& executor) SKR_NOEXCEPT;

    class SKR_RENDER_GRAPH_API ComputePassBuilder
    {
    public:
        friend class RenderGraph;
        ComputePassBuilder& set_name(const char* name) SKR_NOEXCEPT;
        ComputePassBuilder& read(uint32_t set, uint32_t binding, TextureSRVHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& read(const char8_t* name, TextureSRVHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& readwrite(uint32_t set, uint32_t binding, TextureUAVHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& readwrite(const char8_t* name, TextureUAVHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& read(uint32_t set, uint32_t binding, BufferHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& read(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& readwrite(uint32_t set, uint32_t binding, BufferHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& readwrite(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT;
        ComputePassBuilder& set_pipeline(CGPUComputePipelineId pipeline) SKR_NOEXCEPT;
        ComputePassBuilder& set_root_signature(CGPURootSignatureId signature) SKR_NOEXCEPT;

    protected:
        ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass) SKR_NOEXCEPT;
        RenderGraph& graph;
        ComputePassNode& node;
    };
    using ComputePassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::ComputePassBuilder&)>;
    PassHandle add_compute_pass(const ComputePassSetupFunction& setup, const ComputePassExecuteFunction& executor) SKR_NOEXCEPT;

    class SKR_RENDER_GRAPH_API CopyPassBuilder
    {
    public:
        friend class RenderGraph;
        CopyPassBuilder& set_name(const char* name) SKR_NOEXCEPT;
        CopyPassBuilder& can_be_lone() SKR_NOEXCEPT;
        CopyPassBuilder& texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst, ECGPUResourceState out_state = CGPU_RESOURCE_STATE_COPY_DEST) SKR_NOEXCEPT;
        CopyPassBuilder& buffer_to_buffer(BufferRangeHandle src, BufferRangeHandle dst, ECGPUResourceState out_state = CGPU_RESOURCE_STATE_COPY_DEST) SKR_NOEXCEPT;
        CopyPassBuilder& buffer_to_texture(BufferRangeHandle src, TextureSubresourceHandle dst, ECGPUResourceState out_state = CGPU_RESOURCE_STATE_COPY_DEST) SKR_NOEXCEPT;
        CopyPassBuilder& from_buffer(BufferRangeHandle src) SKR_NOEXCEPT;

    protected:
        CopyPassBuilder(RenderGraph& graph, CopyPassNode& pass) noexcept;
        RenderGraph& graph;
        CopyPassNode& node;
    };
    using CopyPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::CopyPassBuilder&)>;
    PassHandle add_copy_pass(const CopyPassSetupFunction& setup, const CopyPassExecuteFunction& executor) SKR_NOEXCEPT;

    class SKR_RENDER_GRAPH_API PresentPassBuilder
    {
    public:
        friend class RenderGraph;

        PresentPassBuilder& set_name(const char* name) SKR_NOEXCEPT;
        PresentPassBuilder& swapchain(CGPUSwapChainId chain, uint32_t index) SKR_NOEXCEPT;
        PresentPassBuilder& texture(TextureHandle texture, bool is_backbuffer = true) SKR_NOEXCEPT;

    protected:
        PresentPassBuilder(RenderGraph& graph, PresentPassNode& present) noexcept;
        RenderGraph& graph;
        PresentPassNode& node;
    };
    using PresentPassSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::PresentPassBuilder&)>;
    PassHandle add_present_pass(const PresentPassSetupFunction& setup) SKR_NOEXCEPT;

    class SKR_RENDER_GRAPH_API BufferBuilder
    {
    public:
        friend class RenderGraph;
        BufferBuilder& set_name(const char* name) SKR_NOEXCEPT;
        BufferBuilder& with_tags(uint32_t tags) SKR_NOEXCEPT;
        BufferBuilder& import(CGPUBufferId buffer, ECGPUResourceState init_state) SKR_NOEXCEPT;
        BufferBuilder& owns_memory() SKR_NOEXCEPT;
        BufferBuilder& structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride) SKR_NOEXCEPT;
        BufferBuilder& size(uint64_t size) SKR_NOEXCEPT;
        BufferBuilder& with_flags(CGPUBufferCreationFlags flags) SKR_NOEXCEPT;
        BufferBuilder& memory_usage(ECGPUMemoryUsage mem_usage) SKR_NOEXCEPT;
        BufferBuilder& allow_shader_readwrite() SKR_NOEXCEPT;
        BufferBuilder& allow_shader_read() SKR_NOEXCEPT;
        BufferBuilder& as_upload_buffer() SKR_NOEXCEPT;
        BufferBuilder& as_vertex_buffer() SKR_NOEXCEPT;
        BufferBuilder& as_index_buffer() SKR_NOEXCEPT;
        BufferBuilder& as_uniform_buffer() SKR_NOEXCEPT;
        BufferBuilder& prefer_on_device() SKR_NOEXCEPT;
        BufferBuilder& prefer_on_host() SKR_NOEXCEPT;

    protected:
        BufferBuilder(RenderGraph& graph, BufferNode& node) SKR_NOEXCEPT;
        RenderGraph& graph;
        BufferNode& node;
    };
    using BufferSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::BufferBuilder&)>;
    BufferHandle create_buffer(const BufferSetupFunction& setup) SKR_NOEXCEPT;
    inline BufferHandle get_buffer(const char* name) SKR_NOEXCEPT;
    const ECGPUResourceState get_lastest_state(const BufferNode* buffer, const PassNode* pending_pass) const SKR_NOEXCEPT;

    class SKR_RENDER_GRAPH_API TextureBuilder
    {
    public:
        friend class RenderGraph;
        TextureBuilder& set_name(const char* name) SKR_NOEXCEPT;
        TextureBuilder& with_flags(CGPUTextureCreationFlags tags) SKR_NOEXCEPT;
        TextureBuilder& with_tags(uint32_t tags) SKR_NOEXCEPT;
        TextureBuilder& import(CGPUTextureId texture, ECGPUResourceState init_state) SKR_NOEXCEPT;
        TextureBuilder& extent(uint32_t width, uint32_t height, uint32_t depth = 1) SKR_NOEXCEPT;
        TextureBuilder& format(ECGPUFormat format) SKR_NOEXCEPT;
        TextureBuilder& array(uint32_t size) SKR_NOEXCEPT;
        TextureBuilder& sample_count(ECGPUSampleCount count) SKR_NOEXCEPT;
        TextureBuilder& allow_render_target() SKR_NOEXCEPT;
        TextureBuilder& allow_depth_stencil() SKR_NOEXCEPT;
        TextureBuilder& allow_readwrite() SKR_NOEXCEPT;
        TextureBuilder& owns_memory() SKR_NOEXCEPT;
        TextureBuilder& allow_lone() SKR_NOEXCEPT;

    protected:
        TextureBuilder(RenderGraph& graph, TextureNode& node) SKR_NOEXCEPT;
        RenderGraph& graph;
        TextureNode& node;
        CGPUTextureId imported = nullptr;
    };
    using TextureSetupFunction = eastl::function<void(RenderGraph&, class RenderGraph::TextureBuilder&)>;
    TextureHandle create_texture(const TextureSetupFunction& setup) SKR_NOEXCEPT;
    TextureHandle get_texture(const char* name) SKR_NOEXCEPT;
    const ECGPUResourceState get_lastest_state(const TextureNode* texture, const PassNode* pending_pass) const SKR_NOEXCEPT;

    bool compile() SKR_NOEXCEPT;    
    BufferNode* resolve(BufferHandle hdl) SKR_NOEXCEPT; 
    TextureNode* resolve(TextureHandle hdl) SKR_NOEXCEPT;
    PassNode* resolve(PassHandle hdl) SKR_NOEXCEPT;
    const CGPUBufferDescriptor* resolve_descriptor(BufferHandle hdl) SKR_NOEXCEPT;
    const CGPUTextureDescriptor* resolve_descriptor(TextureHandle hdl) SKR_NOEXCEPT;
    
    inline Blackboard& get_blackboard() SKR_NOEXCEPT
    {
        return *blackboard;
    }
    virtual uint64_t execute(RenderGraphProfiler* profiler = nullptr) SKR_NOEXCEPT;
    virtual CGPUDeviceId get_backend_device() SKR_NOEXCEPT { return nullptr; }
    virtual CGPUQueueId get_gfx_queue() SKR_NOEXCEPT { return nullptr; }
    virtual uint32_t collect_garbage(uint64_t critical_frame,
        uint32_t tex_with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t tex_without_flags = 0,
        uint32_t buf_with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t buf_without_flags = 0) SKR_NOEXCEPT
    {
        return collect_texture_garbage(critical_frame, tex_with_tags,tex_without_flags)
            + collect_buffer_garbage(critical_frame, buf_with_tags, buf_without_flags);
    }
    virtual uint32_t collect_texture_garbage(uint64_t critical_frame,
        uint32_t with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t without_flags = 0) SKR_NOEXCEPT { return 0; }
    virtual uint32_t collect_buffer_garbage(uint64_t critical_frame,
        uint32_t with_tags = kRenderGraphDefaultResourceTag | kRenderGraphDynamicResourceTag, uint32_t without_flags = 0) SKR_NOEXCEPT { return 0; }


    inline uint64_t get_frame_index() const SKR_NOEXCEPT { return frame_index; }

    inline bool enable_memory_aliasing(bool enabled) SKR_NOEXCEPT
    {
        aliasing_enabled = enabled;
        return aliasing_enabled;
    }

    RenderGraph(const RenderGraphBuilder& builder) SKR_NOEXCEPT;
    virtual ~RenderGraph() SKR_NOEXCEPT = default;
protected:
    uint32_t foreach_textures(eastl::function<void(TextureNode*)> texture) SKR_NOEXCEPT;
    uint32_t foreach_writer_passes(TextureHandle texture,
        eastl::function<void(PassNode* writer, TextureNode* tex, RenderGraphEdge* edge)>) const SKR_NOEXCEPT;
    uint32_t foreach_reader_passes(TextureHandle texture,
        eastl::function<void(PassNode* reader, TextureNode* tex, RenderGraphEdge* edge)>) const SKR_NOEXCEPT;
    uint32_t foreach_writer_passes(BufferHandle buffer,
        eastl::function<void(PassNode* writer, BufferNode* buf, RenderGraphEdge* edge)>) const SKR_NOEXCEPT;
    uint32_t foreach_reader_passes(BufferHandle buffer,
        eastl::function<void(PassNode* reader, BufferNode* buf, RenderGraphEdge* edge)>) const SKR_NOEXCEPT;

    virtual void initialize() SKR_NOEXCEPT;
    virtual void finalize() SKR_NOEXCEPT;

    bool aliasing_enabled;
    uint64_t frame_index = 0;

    struct NodeAndEdgeFactory* object_factory = nullptr;
    Blackboard* blackboard = nullptr;
    DependencyGraph* graph = nullptr;

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

class SKR_RENDER_GRAPH_API RenderGraphViz
{
public:
    static void write_graphviz(RenderGraph& graph, const char* outf) SKR_NOEXCEPT;
};
} // namespace render_graph
} // namespace skr