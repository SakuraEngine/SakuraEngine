#include "SkrBase/misc/debug.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"

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
    auto newPass = node_factory->Allocate<RenderPassNode>(passes_size, frame_index);
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
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: texture is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(name, handle, CGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this.id()), &node, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(
    uint32_t mrt_index, TextureRTVHandle handle, ECGPULoadAction load_action, CGPUClearValue clear_color, ECGPUStoreAction store_action) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: render target is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureRenderEdge>(mrt_index, handle._this, clear_color);
    auto&& edge = node.out_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this.id()), edge);
    node.load_actions[mrt_index] = load_action;
    node.store_actions[mrt_index] = store_action;
    return *this;
}

// 0 ... CGPU_MAX_MRT_COUNT-1 RTs
// CGPU_MAX_MRT_COUNT DS
// CGPU_MAX_MRT_COUNT + 1 .. 2 * CGPU_MAX_MRT_COUNT ResolveTargets
RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::resolve_msaa(uint32_t mrt_index, TextureSubresourceHandle handle)
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: resolve target is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureRenderEdge>(
        CGPU_MAX_MRT_COUNT + 1 + mrt_index, handle._this, fastclear_0000, CGPU_RESOURCE_STATE_RESOLVE_DEST);
    auto&& edge = node.out_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this.id()), edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::set_depth_stencil(TextureDSVHandle handle,
    ECGPULoadAction dload_action,
    ECGPUStoreAction dstore_action,
    ECGPULoadAction sload_action,
    ECGPUStoreAction sstore_action) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: depth-stencil is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureRenderEdge>(
        CGPU_MAX_MRT_COUNT, handle._this, fastclear_0000, CGPU_RESOURCE_STATE_DEPTH_WRITE);
    auto&& edge = node.out_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this.id()), edge);
    node.depth_load_action = dload_action;
    node.depth_store_action = dstore_action;
    node.stencil_load_action = sload_action;
    node.stencil_store_action = sstore_action;
    node.clear_depth = handle.cleardepth;
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::read(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: buffer is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name, handle, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this.id()), &node, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::write(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::use_buffer(PipelineBufferHandle buffer, ECGPUResourceState requested_state) SKR_NOEXCEPT
{
    if (buffer._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: pipeline buffer is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<PipelineBufferEdge>(buffer, requested_state);
    auto&& edge = node.ppl_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(buffer._this.id()), &node, edge);
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

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::with_flags(EPassFlags flags) SKR_NOEXCEPT
{
    node.add_flags(flags);
    return *this;
}

// compute pass builder
RenderGraph::ComputePassBuilder::ComputePassBuilder(RenderGraph& graph, ComputePassNode& pass) SKR_NOEXCEPT
    : graph(graph),
      node(pass)
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
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: texture is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(name, handle, CGPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this.id()), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, TextureUAVHandle handle) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: texture is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureReadWriteEdge>(name, handle);
    auto&& edge = node.inout_texture_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this.id()), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: buffer is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name, handle, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this.id()), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT
{
    if (((HandleStorage)handle).frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: buffer is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<BufferReadEdge>(name, handle.range(0, ~0), CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    auto&& edge = node.in_buffer_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(((HandleStorage)handle).id()), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, BufferHandle handle) SKR_NOEXCEPT
{
    if (((HandleStorage)handle).frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: buffer is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<BufferReadWriteEdge>(name, handle.range(0, ~0), CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    auto&& edge = node.out_buffer_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(((HandleStorage)handle).id()), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::readwrite(const char8_t* name, BufferRangeHandle handle) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: buffer is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<BufferReadWriteEdge>(name, handle, CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    auto&& edge = node.out_buffer_edges.emplace(allocated).ref();
    graph.graph->link(&node, graph.graph->access_node(handle._this.id()), edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::read(const char8_t* name, AccelerationStructureSRVHandle handle) SKR_NOEXCEPT
{
    if (handle._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: acceleration structure is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<AccelerationStructureReadEdge>(name, handle);
    auto&& edge = node.in_acceleration_structure_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(handle._this.id()), &node, edge);
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_pipeline(CGPUComputePipelineId pipeline) SKR_NOEXCEPT
{
    SKR_ASSERT(node.ray_pipeline == nullptr && "Can't set both compute & ray pipeline");
    node.pipeline = pipeline;
    node.root_signature = pipeline->root_signature;
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_ray_pipeline(CGPURayPipelineId pipeline) SKR_NOEXCEPT
{
    SKR_ASSERT(node.pipeline == nullptr && "Can't set both compute & ray pipeline");
    node.ray_pipeline = pipeline;
    node.root_signature = pipeline->root_signature;
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::set_root_signature(CGPURootSignatureId signature) SKR_NOEXCEPT
{
    node.root_signature = signature;
    return *this;
}

RenderGraph::ComputePassBuilder& RenderGraph::ComputePassBuilder::with_flags(EPassFlags flags) SKR_NOEXCEPT
{
    node.add_flags(flags);
    return *this;
}

PassHandle RenderGraph::add_compute_pass(const ComputePassSetupFunction& setup, const ComputePassExecuteFunction& executor) SKR_NOEXCEPT
{
    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<ComputePassNode>(passes_size, frame_index);
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

    if (src._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: src buffer is not alive in the current frame");
    if (dst._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: dst buffer is not alive in the current frame");

    auto allocated_in = graph.node_factory->Allocate<BufferReadEdge>(u8"CopySrc", src, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto allocated_out = graph.node_factory->Allocate<BufferReadWriteEdge>(u8"CopyDst", dst, CGPU_RESOURCE_STATE_COPY_DEST);
    auto&& in_edge = node.in_buffer_edges.emplace(allocated_in).ref();
    auto&& out_edge = node.out_buffer_edges.emplace(allocated_out).ref();
    graph.graph->link(graph.graph->access_node(src._this.id()), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this.id()), out_edge);
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

    if (src._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: src buffer is not alive in the current frame");
    if (dst._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: dst texture is not alive in the current frame");

    auto allocated_in = graph.node_factory->Allocate<BufferReadEdge>(u8"CopySrc", src, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto allocated_out = graph.node_factory->Allocate<TextureRenderEdge>(0u, dst._this, fastclear_0000, CGPU_RESOURCE_STATE_COPY_DEST);
    auto&& in_edge = node.in_buffer_edges.emplace(allocated_in).ref();
    auto&& out_edge = node.out_texture_edges.emplace(allocated_out).ref();
    graph.graph->link(graph.graph->access_node(src._this.id()), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this.id()), out_edge);
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

    if (src._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: src texture is not alive in the current frame");
    if (dst._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: dst texture is not alive in the current frame");

    auto allocated_in = graph.node_factory->Allocate<TextureReadEdge>(u8"CopySrc", src._this, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto allocated_out = graph.node_factory->Allocate<TextureRenderEdge>(0u, dst._this, fastclear_0000, CGPU_RESOURCE_STATE_COPY_DEST);
    auto&& in_edge = node.in_texture_edges.emplace(allocated_in).ref();
    auto&& out_edge = node.out_texture_edges.emplace(allocated_out).ref();
    graph.graph->link(graph.graph->access_node(src._this.id()), &node, in_edge);
    graph.graph->link(&node, graph.graph->access_node(dst._this.id()), out_edge);
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

    if (src._this.frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: src buffer is not alive in the current frame");

    auto allocated_in = graph.node_factory->Allocate<BufferReadEdge>(u8"CopySrc", src, CGPU_RESOURCE_STATE_COPY_SOURCE);
    auto&& in_edge = node.in_buffer_edges.emplace(allocated_in).ref();
    graph.graph->link(graph.graph->access_node(src._this.id()), &node, in_edge);
    return *this;
}

RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::with_flags(EPassFlags flags) SKR_NOEXCEPT
{
    node.add_flags(flags);
    return *this;
}

PassHandle RenderGraph::add_copy_pass(const CopyPassSetupFunction& setup, const CopyPassExecuteFunction& executor) SKR_NOEXCEPT
{
    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<CopyPassNode>(passes_size, frame_index);
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
    if (((HandleStorage)handle).frame_index() != graph.frame_index)
        SKR_LOG_FATAL(u8"lifetime leak detected: present texture is not alive in the current frame");
    auto allocated = graph.node_factory->Allocate<TextureReadEdge>(u8"PresentSrc", handle, CGPU_RESOURCE_STATE_PRESENT);
    auto&& edge = node.in_texture_edges.emplace(allocated).ref();
    graph.graph->link(graph.graph->access_node(((HandleStorage)handle).id()), &node, edge);
    return *this;
}

PassHandle RenderGraph::add_present_pass(const PresentPassSetupFunction& setup) SKR_NOEXCEPT
{
    const uint32_t passes_size = static_cast<uint32_t>(passes.size());
    auto newPass = node_factory->Allocate<PresentPassNode>(passes_size, frame_index);
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
    node.descriptor.usages = CGPU_BUFFER_USAGE_NONE;
    node.descriptor.flags = CGPU_BUFFER_FLAG_NONE;
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
    node.imported = true;
    node.imported_buffer = buffer;
    node.init_state = init_state;
    node.descriptor.usages = buffer->info->usages;
    node.descriptor.size = buffer->info->size;
    node.descriptor.memory_usage = (ECGPUMemoryUsage)buffer->info->memory_usage;
    graph.imported_buffers.emplace(buffer, node.get_handle());
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::heap_dedicated() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_BUFFER_FLAG_DEDICATED_BIT;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride) SKR_NOEXCEPT
{
    node.view_desc.offset = first_element * element_stride;
    node.view_desc.size = element_count * element_stride;
    node.view_desc.structure.element_stride = element_stride;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::size(uint64_t size) SKR_NOEXCEPT
{
    node.descriptor.size = size;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::with_flags(CGPUBufferFlags flags) SKR_NOEXCEPT
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
    node.descriptor.usages |= CGPU_BUFFER_USAGE_SHADER_READWRITE;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::allow_shader_read() SKR_NOEXCEPT
{
    node.descriptor.usages |= CGPU_BUFFER_USAGE_SHADER_READ;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_upload_buffer() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT;
    node.descriptor.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
    node.tags |= kRenderGraphDynamicResourceTag;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_vertex_buffer() SKR_NOEXCEPT
{
    node.descriptor.usages |= CGPU_BUFFER_USAGE_VERTEX_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_index_buffer() SKR_NOEXCEPT
{
    node.descriptor.usages |= CGPU_BUFFER_USAGE_INDEX_BUFFER;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return *this;
}

RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::as_constant_buffer() SKR_NOEXCEPT
{
    node.descriptor.usages |= CGPU_BUFFER_USAGE_CONSTANT_BUFFER;
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

    auto newBuf = node_factory->Allocate<BufferNode>(frame_index);
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
    return BufferHandle();
}

BufferHandle RenderGraph::get_imported(CGPUBufferId buffer) SKR_NOEXCEPT
{
    if (auto it = imported_buffers.find(buffer); it != imported_buffers.end())
    {
        return it->second;
    }
    return BufferHandle();
}

// texture builder
RenderGraph::TextureBuilder::TextureBuilder(RenderGraph& graph, TextureNode& node) SKR_NOEXCEPT
    : graph(graph),
      node(node)
{
    node.descriptor.sample_count = CGPU_SAMPLE_COUNT_1;
    node.descriptor.usages = CGPU_TEXTURE_USAGE_SHADER_READ;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    // blackboard
    node.set_name(name);
    node.descriptor.name = node.get_name();
    graph.blackboard->add_texture(node.descriptor.name, &node);
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::with_flags(CGPUTextureFlags flags) SKR_NOEXCEPT
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
    if (texture)
    {
        const auto texInfo = texture->info;
        node.descriptor.width = texInfo->width;
        node.descriptor.height = texInfo->height;
        node.descriptor.depth = texInfo->depth;
        node.descriptor.format = (ECGPUFormat)texInfo->format;
        node.descriptor.array_size = texInfo->array_size;
        node.descriptor.sample_count = texInfo->sample_count;
    }
    node.init_state = init_state;
    node.imported = true;
    node.imported_texture = texture;
    graph.imported_textures.emplace(texture, node.get_handle());
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

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::mip_count(uint32_t size) SKR_NOEXCEPT
{
    node.descriptor.mip_levels = size;
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
    node.descriptor.usages |= CGPU_TEXTURE_USAGE_SHADER_READWRITE;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_render_target() SKR_NOEXCEPT
{
    node.descriptor.usages |= CGPU_TEXTURE_USAGE_RENDER_TARGET;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::allow_depth_stencil() SKR_NOEXCEPT
{
    node.descriptor.usages |= CGPU_TEXTURE_USAGE_DEPTH_STENCIL;
    node.descriptor.start_state = CGPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::heap_dedicated() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_TEXTURE_FLAG_HEAP_DEDICATED_BIT;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::driver_dedicated() SKR_NOEXCEPT
{
    node.descriptor.flags |= CGPU_TEXTURE_FLAG_DRIVER_DEDICATED_BIT;
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

    auto newTex = node_factory->Allocate<TextureNode>(frame_index);
    resources.add(newTex);
    graph->insert(newTex);
    TextureBuilder builder(*this, *newTex);
    builder.mip_count(1);
    builder.array(1);
    setup(*this, builder);
    // set default gc tag
    if (newTex->tags == kRenderGraphInvalidResourceTag) 
        newTex->tags |= kRenderGraphDefaultResourceTag;
    return newTex->get_handle();
}

TextureHandle RenderGraph::get_texture(const char8_t* name) SKR_NOEXCEPT
{
    if (auto texture = blackboard->texture(name))
    {
        return texture->get_handle();
    }
    return TextureHandle();
}

TextureHandle RenderGraph::get_imported(CGPUTextureId texture) SKR_NOEXCEPT
{
    if (auto it = imported_textures.find(texture); it != imported_textures.end())
    {
        return it->second;
    }
    return TextureHandle();
}

// acceleration structure builder
RenderGraph::AccelerationStructureBuilder::AccelerationStructureBuilder(RenderGraph& graph, AccelerationStructureNode& node) SKR_NOEXCEPT
    : graph(graph),
      node(node)
{
}

RenderGraph::AccelerationStructureBuilder& RenderGraph::AccelerationStructureBuilder::set_name(const char8_t* name) SKR_NOEXCEPT
{
    // blackboard
    node.set_name(name);
    // node.descriptor.name = node.get_name();
    graph.blackboard->add_acceleration_structure(name, &node);
    return *this;
}

RenderGraph::AccelerationStructureBuilder& RenderGraph::AccelerationStructureBuilder::import(CGPUAccelerationStructureId acceleration_structure) SKR_NOEXCEPT
{
    node.imported = true;
    node.imported_as = acceleration_structure;
    node.init_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_READ;
    graph.imported_acceleration_structures.emplace(acceleration_structure, node.get_handle());
    return *this;
}

AccelerationStructureHandle RenderGraph::create_acceleration_structure(const AccelerationStructureSetupFunction& setup) SKR_NOEXCEPT
{
    SkrZoneScopedN("RenderGraph::create_acceleration_structure(handle)");

    auto newAS = node_factory->Allocate<AccelerationStructureNode>(frame_index);
    resources.add(newAS);
    graph->insert(newAS);
    AccelerationStructureBuilder builder(*this, *newAS);
    setup(*this, builder);
    // set default gc tag
    if (newAS->tags == kRenderGraphInvalidResourceTag) newAS->tags |= kRenderGraphDefaultResourceTag;
    return newAS->get_handle();
}

AccelerationStructureHandle RenderGraph::get_acceleration_structure(const char8_t* name) SKR_NOEXCEPT
{
    if (auto acceleration_structure = blackboard->acceleration_structure(name))
    {
        return acceleration_structure->get_handle();
    }
    return AccelerationStructureHandle();
}

AccelerationStructureHandle RenderGraph::get_imported(CGPUAccelerationStructureId acceleration_structure) SKR_NOEXCEPT
{
    if (auto it = imported_acceleration_structures.find(acceleration_structure); it != imported_acceleration_structures.end())
    {
        return it->second;
    }
    return AccelerationStructureHandle();
}

} // namespace render_graph
} // namespace skr