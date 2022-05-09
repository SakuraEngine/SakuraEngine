#include "render_graph/frontend/render_graph.hpp"
#include "../cgpu/common/common_utils.h"

namespace skr
{
namespace render_graph
{
// graph builder
RenderGraph::RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass) noexcept
    : graph(graph)
    , node(pass)
{
}
RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_device(CGPUDeviceId device_)
{
    device = device_;
    return *this;
}
RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::enable_memory_aliasing()
{
    memory_aliasing = true;
    return *this;
}
RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_gfx_queue(CGPUQueueId queue)
{
    gfx_queue = queue;
    return *this;
}
RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::backend_api(ECGPUBackend backend)
{
    api = backend;
    return *this;
}
RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::frontend_only()
{
    no_backend = true;
    return *this;
}
PassHandle RenderGraph::add_render_pass(const RenderPassSetupFunction& setup, const RenderPassExecuteFunction& executor)
{
    auto newPass = new RenderPassNode((uint32_t)passes.size());
    passes.emplace_back(newPass);
    graph->insert(newPass);
    // build up
    RenderPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    newPass->executor = executor;
    return newPass->get_handle();
}

// render pass builder
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, TextureSRVHandle handle)
{
    auto&& edge = node.in_texture_edges.emplace_back(
    new TextureReadEdge(set, binding, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, TextureSRVHandle handle)
{
    auto&& edge = node.in_texture_edges.emplace_back(
    new TextureReadEdge(name, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(
uint32_t mrt_index, TextureRTVHandle handle, ECGPULoadAction load_action,
ECGPUStoreAction store_action)
{
    auto&& edge = node.out_texture_edges.emplace_back(
    new TextureRenderEdge(mrt_index, handle._this));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.load_actions[mrt_index] = load_action;
    node.store_actions[mrt_index] = store_action;
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_depth_stencil(TextureDSVHandle handle,
ECGPULoadAction dload_action, ECGPUStoreAction dstore_action,
ECGPULoadAction sload_action, ECGPUStoreAction sstore_action)
{
    auto&& edge = node.out_texture_edges.emplace_back(
    new TextureRenderEdge(
    MAX_MRT_COUNT, handle._this,
    CGPU_RESOURCE_STATE_DEPTH_WRITE));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.depth_load_action = dload_action;
    node.depth_store_action = dstore_action;
    node.stencil_load_action = sload_action;
    node.stencil_store_action = sstore_action;
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, BufferHandle handle)
{
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(const char8_t* name, BufferHandle handle)
{
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state)
{
    auto&& edge = node.ppl_buffer_edges.emplace_back(
    new PipelineBufferEdge(buffer, requested_state));
    graph.graph->link(graph.graph->access_node(buffer._this), &node, edge);
    return *this;
}
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_pipeline(CGPURenderPipelineId pipeline)
{
    node.pipeline = pipeline;
    return *this;
}

// compute pass builder
RenderGraph::ComputePassBuilder::ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass) noexcept
    : graph(graph)
    , node(pass)
{
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, TextureSRVHandle handle)
{
    auto&& edge = node.in_texture_edges.emplace_back(
    new TextureReadEdge(set, binding, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, TextureSRVHandle handle)
{
    auto&& edge = node.in_texture_edges.emplace_back(
    new TextureReadEdge(name, handle));
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, TextureUAVHandle handle)
{
    auto&& edge = node.inout_texture_edges.emplace_back(
    new TextureReadWriteEdge(set, binding, handle));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, TextureUAVHandle handle)
{
    auto&& edge = node.inout_texture_edges.emplace_back(
    new TextureReadWriteEdge(name, handle));
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, BufferHandle handle)
{
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, BufferHandle handle)
{
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, BufferHandle handle)
{
    return *this;
}
RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_pipeline(CGPUComputePipelineId pipeline)
{
    node.pipeline = pipeline;
    return *this;
}
PassHandle RenderGraph::add_compute_pass(const ComputePassSetupFunction& setup, const ComputePassExecuteFunction& executor)
{
    auto newPass = new ComputePassNode((uint32_t)passes.size());
    passes.emplace_back(newPass);
    graph->insert(newPass);
    // build up
    ComputePassBuilder builder(*this, *newPass);
    setup(*this, builder);
    newPass->executor = executor;
    return newPass->get_handle();
}

// copy pass
RenderGraph::CopyPassBuilder::CopyPassBuilder(RenderGraph& graph, CopyPassNode& pass) noexcept
    : graph(graph)
    , node(pass)
{
}
RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::buffer_to_buffer(BufferRangeHandle src, BufferRangeHandle dst)
{
    auto&& in_edge = node.in_buffer_edges.emplace_back(
    new BufferReadEdge(src, CGPU_RESOURCE_STATE_COPY_SOURCE));
    auto&& out_edge = node.out_buffer_edges.emplace_back(
    new BufferReadWriteEdge(dst, CGPU_RESOURCE_STATE_COPY_DEST));
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this), out_edge);
    node.b2bs.emplace_back(src, dst);
    return *this;
}
RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst)
{
    auto&& in_edge = node.in_texture_edges.emplace_back(
    new TextureReadEdge(0, 0, src._this,
    CGPU_RESOURCE_STATE_COPY_SOURCE));
    auto&& out_edge = node.out_texture_edges.emplace_back(
    new TextureRenderEdge(0, dst._this,
    CGPU_RESOURCE_STATE_COPY_DEST));
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this), out_edge);
    node.t2ts.emplace_back(src, dst);
    return *this;
}
PassHandle RenderGraph::add_copy_pass(const CopyPassSetupFunction& setup)
{
    auto newPass = new CopyPassNode((uint32_t)passes.size());
    passes.emplace_back(newPass);
    graph->insert(newPass);
    // build up
    CopyPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    return newPass->get_handle();
}

// present pass builder
RenderGraph::PresentPassBuilder::PresentPassBuilder(RenderGraph& graph, PresentPassNode& present) noexcept
    : graph(graph)
    , node(present)
{
}
RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::set_name(const char* name)
{
    if (name)
    {
        graph.blackboard.named_passes[name] = &node;
        node.set_name(name);
    }
    return *this;
}
RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::swapchain(CGPUSwapChainId chain, uint32_t index)
{
    node.descriptor.swapchain = chain;
    node.descriptor.index = index;
    return *this;
}
RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::texture(TextureHandle handle, bool is_backbuffer)
{
    assert(is_backbuffer && "blit to screen mode not supported!");
    auto&& edge = node.in_texture_edges.emplace_back(
    new TextureReadEdge(0, 0, handle, CGPU_RESOURCE_STATE_PRESENT));
    graph.graph->link(graph.graph->access_node(handle), &node, edge);
    return *this;
}
PassHandle RenderGraph::add_present_pass(const PresentPassSetupFunction& setup)
{
    auto newPass = new PresentPassNode((uint32_t)passes.size());
    passes.emplace_back(newPass);
    graph->insert(newPass);
    // build up
    PresentPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    return newPass->get_handle();
}

// buffer builder
RenderGraph::BufferBuilder::BufferBuilder(RenderGraph& graph, BufferNode& node) noexcept
    : graph(graph)
    , node(node)
{
    node.descriptor.descriptors = CGPU_RT_NONE;
    node.descriptor.flags = CGPU_BCF_NONE;
    node.descriptor.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::set_name(const char* name)
{
    node.descriptor.name = name;
    // blackboard
    graph.blackboard.named_buffers[name] = &node;
    node.set_name(name);
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::import(CGPUBufferId buffer, ECGPUResourceState init_state)
{
    node.imported = buffer;
    node.frame_buffer = buffer;
    node.init_state = init_state;
    node.descriptor.descriptors = buffer->descriptors;
    node.descriptor.size = buffer->size;
    node.descriptor.memory_usage = (ECGPUMemoryUsage)buffer->memory_usage;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::owns_memory()
{
    node.descriptor.flags |= CGPU_BCF_OWN_MEMORY_BIT;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride)
{
    node.descriptor.first_element = first_element;
    node.descriptor.elemet_count = element_count;
    node.descriptor.element_stride = element_stride;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::size(uint64_t size)
{
    node.descriptor.size = size;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::with_flags(CGPUBufferCreationFlags flags)
{
    node.descriptor.flags |= flags;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::memory_usage(ECGPUMemoryUsage mem_usage)
{
    node.descriptor.memory_usage = mem_usage;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::allow_shader_readwrite()
{
    node.descriptor.descriptors |= CGPU_RT_RW_BUFFER;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::allow_shader_read()
{
    node.descriptor.descriptors |= CGPU_RT_BUFFER;
    node.descriptor.descriptors |= CGPU_RT_UNIFORM_BUFFER;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_upload_buffer()
{
    node.descriptor.flags |= CGPU_BCF_PERSISTENT_MAP_BIT;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_SOURCE;
    node.descriptor.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_vertex_buffer()
{
    node.descriptor.descriptors |= CGPU_RT_VERTEX_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_index_buffer()
{
    node.descriptor.descriptors |= CGPU_RT_INDEX_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::prefer_on_device()
{
    node.descriptor.prefer_on_device = true;
    return *this;
}
RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::prefer_on_host()
{
    node.descriptor.prefer_on_device = true;
    return *this;
}
BufferHandle RenderGraph::create_buffer(const BufferSetupFunction& setup)
{
    auto newTex = new BufferNode();
    resources.emplace_back(newTex);
    graph->insert(newTex);
    BufferBuilder builder(*this, *newTex);
    setup(*this, builder);
    return newTex->get_handle();
}
BufferHandle RenderGraph::get_buffer(const char* name)
{
    if (blackboard.named_buffers.find(name) != blackboard.named_buffers.end())
        return blackboard.named_buffers[name]->get_handle();
    return UINT64_MAX;
}

// texture builder
RenderGraph::TextureBuilder::TextureBuilder(RenderGraph& graph, TextureNode& node) noexcept
    : graph(graph)
    , node(node)
{
    node.descriptor.descriptors = CGPU_RT_TEXTURE;
    node.descriptor.is_dedicated = false;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::set_name(const char* name)
{
    node.descriptor.name = name;
    // blackboard
    graph.blackboard.named_textures[name] = &node;
    node.set_name(name);
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::import(CGPUTextureId texture, ECGPUResourceState init_state)
{
    node.imported = texture;
    node.frame_texture = texture;
    node.init_state = init_state;
    node.descriptor.width = texture->width;
    node.descriptor.height = texture->height;
    node.descriptor.depth = texture->depth;
    node.descriptor.format = (ECGPUFormat)texture->format;
    node.descriptor.array_size = texture->array_size_minus_one + 1;
    node.descriptor.sample_count = texture->sample_count;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::extent(
uint32_t width, uint32_t height, uint32_t depth)
{
    node.descriptor.width = width;
    node.descriptor.height = height;
    node.descriptor.depth = depth;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::format(
ECGPUFormat format)
{
    node.descriptor.format = format;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::array(uint32_t size)
{
    node.descriptor.array_size = size;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::sample_count(
ECGPUSampleCount count)
{
    node.descriptor.sample_count = count;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_readwrite()
{
    node.descriptor.descriptors |= CGPU_RT_RW_TEXTURE;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_render_target()
{
    node.descriptor.descriptors |= CGPU_RT_RENDER_TARGET;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_depth_stencil()
{
    node.descriptor.descriptors |= CGPU_RT_DEPTH_STENCIL;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::owns_memory()
{
    node.descriptor.flags |= CGPU_TCF_OWN_MEMORY_BIT;
    return *this;
}
RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_lone()
{
    node.canbe_lone = true;
    return *this;
}
TextureHandle RenderGraph::create_texture(const TextureSetupFunction& setup)
{
    auto newTex = new TextureNode();
    resources.emplace_back(newTex);
    graph->insert(newTex);
    TextureBuilder builder(*this, *newTex);
    setup(*this, builder);
    return newTex->get_handle();
}
TextureHandle RenderGraph::get_texture(const char* name)
{
    if (blackboard.named_textures.find(name) != blackboard.named_textures.end())
        return blackboard.named_textures[name]->get_handle();
    return UINT64_MAX;
}
} // namespace render_graph
} // namespace skr