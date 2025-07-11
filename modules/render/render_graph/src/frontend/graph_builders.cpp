#include "SkrBase/misc/debug.h" 
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "SkrContainers/string.hpp"

namespace skr
{
namespace render_graph
{
// graph builder
RenderGraph::RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPassNode& pass) SKR_NOEXCEPT
    : graph(graph),
      node(pass)
{
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_device(CGPUDeviceId device_) SKR_NOEXCEPT
{
    device = device_;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::enable_memory_aliasing() SKR_NOEXCEPT
{
    memory_aliasing = true;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_gfx_queue(CGPUQueueId queue) SKR_NOEXCEPT
{
    gfx_queue = queue;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_cmpt_queues(const skr::Vector<CGPUQueueId>& queues) SKR_NOEXCEPT
{
    cmpt_queues = queues;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::with_cpy_queues(const skr::Vector<CGPUQueueId>& queues) SKR_NOEXCEPT
{
    cpy_queues = queues;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::backend_api(ECGPUBackend backend) SKR_NOEXCEPT
{
    api = backend;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::frontend_only() SKR_NOEXCEPT
{
    no_backend = true;
    return *this;
}

PassHandle RenderGraph::add_render_pass(const RenderPassSetupFunction& setup, const RenderPassExecuteFunction& executor) SKR_NOEXCEPT
{
    SkrZoneScopedN("CopyPassBuilder::add_render_pass");

    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<RenderPassNode>(passes_size);
    passes.add(newPass);
    graph->insert(newPass);
    // build up
    RenderPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    newPass->executor = executor;
    return newPass->get_handle();
}

// render pass builder
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    if (name)
    {
        node.set_name(name);
        graph.blackboard->add_pass(node.get_name(), &node);
    }
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, TextureSRVHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(name, handle);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(
    uint32_t mrt_index, TextureRTVHandle handle, ECGPULoadAction load_action, CGPUClearValue clear_color,
    ECGPUStoreAction store_action) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<TextureRenderEdge>(mrt_index, handle._this, clear_color);
    auto&& edge = node.out_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.load_actions[mrt_index] = load_action;
    node.store_actions[mrt_index] = store_action;
    return *this;
}

// 0 ... CGPU_MAX_MRT_COUNT-1 RTs
// CGPU_MAX_MRT_COUNT DS
// CGPU_MAX_MRT_COUNT + 1 .. 2 * CGPU_MAX_MRT_COUNT ResolveTargets
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::resolve_msaa(uint32_t mrt_index, TextureSubresourceHandle handle)
{
    auto allocated = graph.node_factory->Allocate<TextureRenderEdge>(
        CGPU_MAX_MRT_COUNT + 1 + mrt_index, handle._this, fastclear_0000, CGPU_RESOURCE_STATE_RESOLVE_DEST);
    auto&& edge = node.out_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_depth_stencil(TextureDSVHandle handle,
    ECGPULoadAction dload_action, ECGPUStoreAction dstore_action,
    ECGPULoadAction sload_action, ECGPUStoreAction sstore_action) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<TextureRenderEdge>(
        CGPU_MAX_MRT_COUNT, handle._this, fastclear_0000, CGPU_RESOURCE_STATE_DEPTH_WRITE);
    auto&& edge = node.out_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    node.depth_load_action = dload_action;
    node.depth_store_action = dstore_action;
    node.stencil_load_action = sload_action;
    node.stencil_store_action = sstore_action;
    node.clear_depth = handle.cleardepth;
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name, handle, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<PipelineBufferEdge>(buffer, requested_state);
    auto&& edge = node.ppl_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(buffer._this), &node, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_pipeline(CGPURenderPipelineId pipeline) SKR_NOEXCEPT
{
    node.pipeline = pipeline;
    node.root_signature = pipeline->root_signature;
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_root_signature(CGPURootSignatureId signature) SKR_NOEXCEPT
{
    node.root_signature = signature;
    return *this;
}

// compute pass builder
RenderGraph::ComputePassBuilder::ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass) SKR_NOEXCEPT
    : graph(graph), node(pass)
{
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    if (name)
    {
        graph.blackboard->add_pass(name, &node);
        node.set_name(name);
    }
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, TextureSRVHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(name, handle);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, TextureUAVHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<TextureReadWriteEdge>(name, handle);
    auto&& edge = node.inout_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name, handle, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, BufferRangeHandle handle) SKR_NOEXCEPT
{
    auto name = skr::format(u8"set{}_binding{}", set, binding);
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name.c_str(), handle, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name, handle.range(0, ~0), CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, BufferHandle handle) SKR_NOEXCEPT
{
    auto name = skr::format(u8"set{}_binding{}", set, binding);
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name.c_str(), handle.range(0, ~0), CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(uint32_t set, uint32_t binding, TextureSRVHandle handle) SKR_NOEXCEPT
{
    auto name = skr::format(u8"set{}_binding{}", set, binding);
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(name.c_str(), handle);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, TextureUAVHandle handle) SKR_NOEXCEPT
{
    auto name = skr::format(u8"set{}_binding{}", set, binding);
    auto allocated = graph.node_factory->Allocate<TextureReadWriteEdge>(name.c_str(), handle);
    auto&& edge = node.inout_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, BufferRangeHandle handle) SKR_NOEXCEPT
{
    auto name = skr::format(u8"set{}_binding{}", set, binding);
    auto allocated = graph.node_factory->Allocate<BufferReadWriteEdge>(handle, CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    auto&& edge = node.out_buffer_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<BufferReadWriteEdge>(handle.range(0, ~0), CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    auto&& edge = node.out_buffer_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(uint32_t set, uint32_t binding, BufferHandle handle) SKR_NOEXCEPT
{
    auto name = skr::format(u8"set{}_binding{}", set, binding);
    auto allocated = graph.node_factory->Allocate<BufferReadWriteEdge>(handle.range(0, ~0), CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    auto&& edge = node.out_buffer_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    auto allocated = graph.node_factory->Allocate<BufferReadWriteEdge>(handle, CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    auto&& edge = node.out_buffer_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_pipeline(CGPUComputePipelineId pipeline) SKR_NOEXCEPT
{
    node.pipeline = pipeline;
    node.root_signature = pipeline->root_signature;
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_root_signature(CGPURootSignatureId signature) SKR_NOEXCEPT
{
    node.root_signature = signature;
    return *this;
}

PassHandle RenderGraph::add_compute_pass(const ComputePassSetupFunction& setup, const ComputePassExecuteFunction& executor) SKR_NOEXCEPT
{
    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<ComputePassNode>(passes_size);
    passes.add(newPass);
    graph->insert(newPass);
    // build up
    ComputePassBuilder builder(*this, *newPass);
    setup(*this, builder);
    newPass->executor = executor;
    return newPass->get_handle();
}

// copy pass
RenderGraph::CopyPassBuilder::CopyPassBuilder(RenderGraph& graph, CopyPassNode& pass) SKR_NOEXCEPT
    : graph(graph),
      node(pass)
{
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    if (name)
    {
        graph.blackboard->add_pass(name, &node);
        node.set_name(name);
    }
    return *this;
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::can_be_lone() SKR_NOEXCEPT
{
    node.can_be_lone = true;
    return *this;
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::buffer_to_buffer(BufferRangeHandle src, BufferRangeHandle dst, ECGPUResourceState out_state) SKR_NOEXCEPT
{
    SkrZoneScopedN("CopyPassBuilder::buffer_to_buffer");

    auto allocated_in = graph.node_factory->Allocate<BufferReadEdge>(u8"CopySrc", src, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto allocated_out = graph.node_factory->Allocate<BufferReadWriteEdge>(dst, CGPU_RESOURCE_STATE_COPY_DEST);
    auto&& in_edge = node.in_buffer_edges.emplace(allocated_in).ref();
    auto&& out_edge = node.out_buffer_edges.emplace(allocated_out).ref();
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this), out_edge);
    node.b2bs.emplace(src, dst);
    if (out_state != CGPU_RESOURCE_STATE_COPY_DEST)
    {
        node.bbarriers.emplace(dst, out_state);
    }
    return *this;
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::buffer_to_texture(BufferRangeHandle src, TextureSubresourceHandle dst, ECGPUResourceState out_state) SKR_NOEXCEPT
{
    SkrZoneScopedN("CopyPassBuilder::buffer_to_texture");

    auto allocated_in = graph.node_factory->Allocate<BufferReadEdge>(u8"CopySrc", src, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto allocated_out = graph.node_factory->Allocate<TextureRenderEdge>(0u, dst._this, fastclear_0000, CGPU_RESOURCE_STATE_COPY_DEST);
    auto&& in_edge = node.in_buffer_edges.emplace(allocated_in).ref();
    auto&& out_edge = node.out_texture_edges.emplace(allocated_out).ref();
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this), out_edge);
    node.b2ts.emplace(src, dst);
    if (out_state != CGPU_RESOURCE_STATE_COPY_DEST)
    {
        node.tbarriers.emplace(dst, out_state);
    }
    return *this;
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::texture_to_texture(TextureSubresourceHandle src, TextureSubresourceHandle dst, ECGPUResourceState out_state) SKR_NOEXCEPT
{
    SkrZoneScopedN("CopyPassBuilder::texture_to_texture");

    auto allocated_in = graph.node_factory->Allocate<TextureReadEdge>(u8"CopySrc", src._this, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto allocated_out = graph.node_factory->Allocate<TextureRenderEdge>(0u, dst._this, fastclear_0000, CGPU_RESOURCE_STATE_COPY_DEST);
    auto&& in_edge = node.in_texture_edges.emplace(allocated_in).ref();
    auto&& out_edge = node.out_texture_edges.emplace(allocated_out).ref();
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this), out_edge);
    node.t2ts.emplace(src, dst);
    if (out_state != CGPU_RESOURCE_STATE_COPY_DEST)
    {
        node.tbarriers.emplace(dst, out_state);
    }
    return *this;
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::from_buffer(BufferRangeHandle src) SKR_NOEXCEPT
{
    SkrZoneScopedN("CopyPassBuilder::from_buffer");

    auto allocated_in = graph.node_factory->Allocate<BufferReadEdge>(u8"CopySrc", src, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto&& in_edge = node.in_buffer_edges.emplace(allocated_in).ref();
    graph.graph->link(graph.graph->access_node(src._this), &node, in_edge);
    return *this;
}

PassHandle RenderGraph::add_copy_pass(const CopyPassSetupFunction& setup, const CopyPassExecuteFunction& executor) SKR_NOEXCEPT
{
    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<CopyPassNode>(passes_size);
    passes.add(newPass);
    graph->insert(newPass);
    // build up
    CopyPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    newPass->executor = executor;
    return newPass->get_handle();
}

// present pass builder
RenderGraph::PresentPassBuilder::PresentPassBuilder(RenderGraph& graph, PresentPassNode& present) SKR_NOEXCEPT
    : graph(graph),
      node(present)
{
}

RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    if (name)
    {
        graph.blackboard->add_pass(name, &node);
        node.set_name(name);
    }
    return *this;
}

RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::swapchain(CGPUSwapChainId chain, uint32_t index) SKR_NOEXCEPT
{
    node.descriptor.swapchain = chain;
    node.descriptor.index = index;
    return *this;
}

RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::texture(TextureHandle handle, bool is_backbuffer) SKR_NOEXCEPT
{
    assert(is_backbuffer && "blit to screen mode not supported!");
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(u8"PresentSrc", handle, CGPU_RESOURCE_STATE_PRESENT);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle), &node, edge);
    return *this;
}

PassHandle RenderGraph::add_present_pass(const PresentPassSetupFunction& setup) SKR_NOEXCEPT
{
    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<PresentPassNode>(passes_size);
    passes.add(newPass);
    graph->insert(newPass);
    // build up
    PresentPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    return newPass->get_handle();
}

// buffer builder
RenderGraph::BufferBuilder::BufferBuilder(RenderGraph& graph, BufferNode& node) SKR_NOEXCEPT
    : graph(graph),
      node(node)
{
    node.descriptor.descriptors = CGPU_RESOURCE_TYPE_NONE;
    node.descriptor.flags = CGPU_BCF_NONE;
    node.descriptor.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    // blackboard
    node.set_name(name);
    node.descriptor.name = node.get_name();
    graph.blackboard->add_buffer(node.descriptor.name, &node);
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::with_tags(uint32_t tags) SKR_NOEXCEPT
{
    node.tags |= tags;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::import(CGPUBufferId buffer, ECGPUResourceState init_state) SKR_NOEXCEPT
{
    node.imported = buffer;
    node.frame_buffer = buffer;
    node.init_state = init_state;
    node.descriptor.descriptors = buffer->info->descriptors;
    node.descriptor.size = buffer->info->size;
    node.descriptor.memory_usage = (ECGPUMemoryUsage)buffer->info->memory_usage;
    graph.imported_buffers[buffer] = node.get_handle();
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::allocate_dedicated() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_BCF_DEDICATED_BIT;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride) SKR_NOEXCEPT
{
    node.descriptor.first_element = first_element;
    node.descriptor.element_count = element_count;
    node.descriptor.element_stride = element_stride;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::size(uint64_t size) SKR_NOEXCEPT
{
    node.descriptor.size = size;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::with_flags(CGPUBufferCreationFlags flags) SKR_NOEXCEPT
{
    node.descriptor.flags |= flags;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::memory_usage(ECGPUMemoryUsage mem_usage) SKR_NOEXCEPT
{
    node.descriptor.memory_usage = mem_usage;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::allow_shader_readwrite() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_RW_BUFFER;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::allow_shader_read() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_BUFFER;
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_upload_buffer() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_BCF_PERSISTENT_MAP_BIT;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_SOURCE;
    node.descriptor.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    node.tags |= kRenderGraphDynamicResourceTag;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_vertex_buffer() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_index_buffer() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_uniform_buffer() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::prefer_on_device() SKR_NOEXCEPT
{
    node.descriptor.prefer_on_device = true;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::prefer_on_host() SKR_NOEXCEPT
{
    node.descriptor.prefer_on_device = true;
    return *this;
}

BufferHandle RenderGraph::create_buffer(const BufferSetupFunction& setup) SKR_NOEXCEPT
{
    SkrZoneScopedN("RenderGraph::create_buffer(handle)");

    auto newBuf = node_factory->Allocate<BufferNode>();
    resources.add(newBuf);
    graph->insert(newBuf);
    BufferBuilder builder(*this, *newBuf);
    setup(*this, builder);
    // set default gc tag
    if (newBuf->tags == kRenderGraphInvalidResourceTag) newBuf->tags |= kRenderGraphDefaultResourceTag;
    return newBuf->get_handle();
}

BufferHandle RenderGraph::get_buffer(const char8_t* name) SKR_NOEXCEPT
{
    if (auto buffer = blackboard->buffer(name))
    {
        return buffer->get_handle();
    }
    return UINT64_MAX;
}

BufferHandle RenderGraph::get_imported(CGPUBufferId buffer) SKR_NOEXCEPT
{
    if (auto it = imported_buffers.find(buffer); it != imported_buffers.end())
    {
        return it->second;
    }
    return UINT64_MAX;
}

// texture builder
RenderGraph::TextureBuilder::TextureBuilder(RenderGraph& graph, TextureNode& node) SKR_NOEXCEPT
    : graph(graph),
      node(node)
{
    node.descriptor.sample_count = CGPU_SAMPLE_COUNT_1;
    node.descriptor.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
    node.descriptor.is_restrict_dedicated = false;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    // blackboard
    node.set_name(name);
    node.descriptor.name = node.get_name();
    graph.blackboard->add_texture(node.descriptor.name, &node);
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::with_flags(CGPUTextureCreationFlags flags) SKR_NOEXCEPT
{
    node.descriptor.flags |= flags;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::with_tags(uint32_t tags) SKR_NOEXCEPT
{
    node.tags |= tags;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::import(CGPUTextureId texture, ECGPUResourceState init_state) SKR_NOEXCEPT
{
    const auto texInfo = texture->info;
    node.imported = texture;
    node.frame_texture = texture;
    node.init_state = init_state;
    node.descriptor.width = texInfo->width;
    node.descriptor.height = texInfo->height;
    node.descriptor.depth = texInfo->depth;
    node.descriptor.format = (ECGPUFormat)texInfo->format;
    node.descriptor.array_size = texInfo->array_size_minus_one + 1;
    node.descriptor.sample_count = texInfo->sample_count;
    graph.imported_textures[texture] = node.get_handle();
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::extent(uint64_t width, uint64_t height, uint64_t depth) SKR_NOEXCEPT
{
    node.descriptor.width = width;
    node.descriptor.height = height;
    node.descriptor.depth = depth;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::format(ECGPUFormat format) SKR_NOEXCEPT
{
    node.descriptor.format = format;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::array(uint32_t size) SKR_NOEXCEPT
{
    node.descriptor.array_size = size;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::sample_count(ECGPUSampleCount count) SKR_NOEXCEPT
{
    node.descriptor.sample_count = count;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_readwrite() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_RW_TEXTURE;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_render_target() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_RENDER_TARGET;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_depth_stencil() SKR_NOEXCEPT
{
    node.descriptor.descriptors |= CGPU_RESOURCE_TYPE_DEPTH_STENCIL;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allocate_dedicated() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_TCF_DEDICATED_BIT;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_lone() SKR_NOEXCEPT
{
    node.canbe_lone = true;
    return *this;
}

TextureHandle RenderGraph::create_texture(const TextureSetupFunction& setup) SKR_NOEXCEPT
{
    SkrZoneScopedN("RenderGraph::create_texture(handle)");

    auto newTex = node_factory->Allocate<TextureNode>();
    resources.add(newTex);
    graph->insert(newTex);
    TextureBuilder builder(*this, *newTex);
    setup(*this, builder);
    // set default gc tag
    if (newTex->tags == kRenderGraphInvalidResourceTag) newTex->tags |= kRenderGraphDefaultResourceTag;
    return newTex->get_handle();
}

TextureHandle RenderGraph::get_texture(const char8_t* name) SKR_NOEXCEPT
{    
    if (auto texture = blackboard->texture(name))
    {
        return texture->get_handle();
    }
    return UINT64_MAX;
}

TextureHandle RenderGraph::get_imported(CGPUTextureId texture) SKR_NOEXCEPT
{
    if (auto it = imported_textures.find(texture); it != imported_textures.end())
    {
        return it->second;
    }
    return UINT64_MAX;
}

} // namespace render_graph
} // namespace skr