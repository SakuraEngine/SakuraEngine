#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "platform/debug.h"
#include "platform/memory.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "utils/format.hpp"
#include <EASTL/set.h>

#include "tracy/Tracy.hpp"

namespace skr
{
namespace render_graph
{
// Render Graph Executor

void RenderGraphFrameExecutor::initialize(CGPUQueueId gfx_queue, CGPUDeviceId device)
{
    CGPUCommandPoolDescriptor pool_desc = {};
    gfx_cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    CGPUCommandBufferDescriptor cmd_desc = {};
    cmd_desc.is_secondary = false;
    gfx_cmd_buf = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
    exec_fence = cgpu_create_fence(device);

    CGPUMarkerBufferDescriptor marker_desc = {};
    marker_desc.marker_count = 1000;
    marker_buffer = cgpu_create_marker_buffer(device, &marker_desc);
}

void RenderGraphFrameExecutor::commit(CGPUQueueId gfx_queue, uint64_t frame_index)
{
    CGPUQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &gfx_cmd_buf;
    submit_desc.cmds_count = 1;
    submit_desc.signal_fence = exec_fence;
    cgpu_submit_queue(gfx_queue, &submit_desc);
    exec_frame = frame_index;
}

void RenderGraphFrameExecutor::reset_begin(TextureViewPool& texture_view_pool)
{
    {
        ZoneScopedN("ResetBindTables");
        for (auto bind_table_pool : bind_table_pools)
        {
            bind_table_pool.second->reset();
        }
    }

    {
        ZoneScopedN("ResetAliasingBinds");
        for (auto aliasing_texture : aliasing_textures)
        {
            texture_view_pool.erase(aliasing_texture);
            cgpu_free_texture(aliasing_texture);
        }
        aliasing_textures.clear();
    }

    {
        ZoneScopedN("ResetMarkerBuffer");
        marker_idx = 0;
        marker_messages.clear();
        valid_marker_val++;
    }

    {
        ZoneScopedN("ResetCommandPool");
        cgpu_reset_command_pool(gfx_cmd_pool);
    }

    cgpu_cmd_begin(gfx_cmd_buf);
    write_marker("Frame Begin");
}

void RenderGraphFrameExecutor::write_marker(const char* message)
{
    cgpu_marker_buffer_write(gfx_cmd_buf, marker_buffer, marker_idx++, valid_marker_val);
    marker_messages.push_back(message);
}

void RenderGraphFrameExecutor::print_error_trace(uint64_t frame_index)
{
    auto fill_data = (const uint32_t*)marker_buffer->cgpu_buffer->cpu_mapped_address;
    if (fill_data[0] == 0) return;// begin cmd is unlikely to fail on gpu
    SKR_LOG_FATAL("Device lost caused by GPU command buffer failure detected %d frames ago, command trace:", frame_index - exec_frame);
    for (uint32_t i = 0; i < marker_messages.size(); i++)
    {
        if (fill_data[i] != valid_marker_val)
        {
            SKR_LOG_ERROR("\tFailed Command %d: %s (marker %d)", i, marker_messages[i].c_str(), fill_data[i]);
        }
        else
        {
            SKR_LOG_INFO("\tCommand %d: %s (marker %d)", i, marker_messages[i].c_str(), fill_data[i]);
        }
    }
}

void RenderGraphFrameExecutor::finalize()
{
    if (gfx_cmd_buf) cgpu_free_command_buffer(gfx_cmd_buf);
    if (gfx_cmd_pool) cgpu_free_command_pool(gfx_cmd_pool);
    if (exec_fence) cgpu_free_fence(exec_fence);
    gfx_cmd_buf = nullptr;
    gfx_cmd_pool = nullptr;
    exec_fence = nullptr;
    for (auto [rs, pool] : bind_table_pools)
    {
        pool->destroy();
        SkrDelete(pool);
    }
    for (auto aliasing_tex : aliasing_textures)
    {
        cgpu_free_texture(aliasing_tex);
    }
    if (marker_buffer) cgpu_free_marker_buffer(marker_buffer);
}

// Render Graph Backend

RenderGraphBackend::RenderGraphBackend(const RenderGraphBuilder& builder)
    : RenderGraph(builder)
    , gfx_queue(builder.gfx_queue)
    , device(builder.device)
{
}

RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup) SKR_NOEXCEPT
{
    RenderGraphBuilder builder = {};
    RenderGraph* graph = nullptr;
    setup(builder);
    if (builder.no_backend)
        graph = SkrNew<RenderGraph>(builder);
    else
    {
        if (!builder.gfx_queue) assert(0 && "not supported!");
        graph = SkrNew<RenderGraphBackend>(builder);
    }
    graph->initialize();
    return graph;
}

void RenderGraph::destroy(RenderGraph* g) SKR_NOEXCEPT
{
    g->finalize();
    SkrDelete(g);
}

void RenderGraphBackend::initialize() SKR_NOEXCEPT
{
    RenderGraph::initialize();
    backend = device->adapter->instance->backend;
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].initialize(gfx_queue, device);
    }
    buffer_pool.initialize(device);
    texture_pool.initialize(device);
    texture_view_pool.initialize(device);
}

void RenderGraphBackend::finalize() SKR_NOEXCEPT
{
    RenderGraph::finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].finalize();
    }
    buffer_pool.finalize();
    texture_pool.finalize();
    texture_view_pool.finalize();
}

// memory aliasing:
// - lifespan不重叠的资源对象都可以考虑做aliasing处理
// - aliasing的memory-type有要求，需要检查两个对象之间的内存堆兼容性
// - 可以撮合两个对象，将它们提升为可alias的，把他们的size和alignment取最大处理即可。
//  - 首先寻找可以直接alias处理的, cgpu_try_create_aliasing_resource(ResourceId, ResourceDesc)
//  - 其次再考虑提升合并的行为（此行为在前端无法模拟）
CGPUTextureId RenderGraphBackend::try_aliasing_allocate(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT
{
    if (node.frame_aliasing_source)
    {
        ZoneScopedN("AllocateAliasingResource");
        // allocate & try bind
        auto aliasing_texture = cgpu_create_texture(device, &node.descriptor);
        CGPUTextureAliasingBindDescriptor aliasing_desc = {};
        aliasing_desc.aliased = resolve(executor, *node.frame_aliasing_source);
        aliasing_desc.aliasing = aliasing_texture;
        node.frame_aliasing = cgpu_try_bind_aliasing_texture(device, &aliasing_desc);
        if (!node.frame_aliasing)
        {
            cgpu_free_texture(aliasing_texture);
            ((TextureNode&)node).descriptor.is_aliasing = false;
            return nullptr;
        }
        executor.aliasing_textures.emplace_back(aliasing_texture);
        return aliasing_texture;
    }
    return nullptr;
}

uint64_t RenderGraphBackend::get_latest_finished_frame() SKR_NOEXCEPT
{
    uint64_t result = frame_index - RG_MAX_FRAME_IN_FLIGHT;
    for (auto&& executor : executors)
    {
        if (!executor.exec_fence) continue;
        if (cgpu_query_fence_status(executor.exec_fence) == CGPU_FENCE_STATUS_COMPLETE)
        {
            result = std::max(result, executor.exec_frame);
        }
    }
    return result;
}

CGPUTextureId RenderGraphBackend::resolve(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT
{
    ZoneScopedN("ResolveTexture");
    if (!node.frame_texture)
    {
        if (auto aliased = try_aliasing_allocate(executor, node); aliased)
        {
            node.frame_texture = aliased;
            node.init_state = CGPU_RESOURCE_STATE_UNDEFINED;
        }
        else
        {
            auto allocated = texture_pool.allocate(node.descriptor, { frame_index, node.tags });
            node.frame_texture = node.imported ? node.frame_texture : allocated.first;
            node.init_state = allocated.second;
        }
    }
    return node.frame_texture;
}

CGPUBufferId RenderGraphBackend::resolve(RenderGraphFrameExecutor& executor, const BufferNode& node) SKR_NOEXCEPT
{
    ZoneScopedN("ResolveBuffer");
    if (!node.frame_buffer)
    {
        uint64_t latest_frame = (node.tags & kRenderGraphDynamicResourceTag) ? get_latest_finished_frame() : UINT64_MAX;
        auto allocated = buffer_pool.allocate(node.descriptor, { frame_index, node.tags }, latest_frame);
        node.frame_buffer = node.imported ? node.frame_buffer : allocated.first;
        node.init_state = allocated.second;
    }
    return node.frame_buffer;
}

void RenderGraphBackend::calculate_barriers(RenderGraphFrameExecutor& executor, PassNode* pass,
    stack_vector<CGPUTextureBarrier>& tex_barriers, stack_vector<eastl::pair<TextureHandle, CGPUTextureId>>& resolved_textures,
    stack_vector<CGPUBufferBarrier>& buf_barriers, stack_vector<eastl::pair<BufferHandle, CGPUBufferId>>& resolved_buffers) SKR_NOEXCEPT
{
    ZoneScopedN("CalculateBarriers");
    tex_barriers.reserve(pass->textures_count());
    resolved_textures.reserve(pass->textures_count());
    buf_barriers.reserve(pass->buffers_count());
    resolved_buffers.reserve(pass->buffers_count());
    pass->foreach_textures(
        [&](TextureNode* texture, TextureEdge* edge) {
            auto tex_resolved = resolve(executor, *texture);
            resolved_textures.emplace_back(texture->get_handle(), tex_resolved);
            const auto current_state = get_lastest_state(texture, pass);
            const auto dst_state = edge->requested_state;
            if (current_state == dst_state) return;
            CGPUTextureBarrier barrier = {};
            barrier.src_state = current_state;
            barrier.dst_state = dst_state;
            barrier.texture = tex_resolved;
            tex_barriers.emplace_back(barrier);
        });
    pass->foreach_buffers(
        [&](BufferNode* buffer, BufferEdge* edge) {
            auto buf_resolved = resolve(executor, *buffer);
            resolved_buffers.emplace_back(buffer->get_handle(), buf_resolved);
            const auto current_state = get_lastest_state(buffer, pass);
            const auto dst_state = edge->requested_state;
            if (current_state == dst_state) return;
            CGPUBufferBarrier barrier = {};
            barrier.src_state = current_state;
            barrier.dst_state = dst_state;
            barrier.buffer = buf_resolved;
            buf_barriers.emplace_back(barrier);
        });
}

const CGPUShaderResource* find_shader_resource(uint64_t name_hash, CGPURootSignatureId root_sig, ECGPUResourceType* type = nullptr)
{
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        for (uint32_t j = 0; j < root_sig->tables[i].resources_count; j++)
        {
            const auto& resource = root_sig->tables[i].resources[j];
            if (resource.name_hash == name_hash)
            {
                if (type) *type = resource.type;
                return &root_sig->tables[i].resources[j];
            }
        }
    }
    return nullptr;
}

CGPUXBindTableId RenderGraphBackend::alloc_update_pass_bind_table(RenderGraphFrameExecutor& executor, PassNode* pass) SKR_NOEXCEPT
{
    ZoneScopedN("UpdateBindings");
    CGPURootSignatureId root_sig = nullptr;
    auto tex_read_edges = pass->tex_read_edges();
    auto tex_rw_edges = pass->tex_readwrite_edges();
    auto buf_read_edges = pass->buf_read_edges();
    auto buf_rw_edges = pass->buf_readwrite_edges(); (void)buf_rw_edges;
    // Get Root Signature
    if (pass->pass_type == EPassType::Render)
        root_sig = ((RenderPassNode*)pass)->root_signature;
    else if (pass->pass_type == EPassType::Compute)
        root_sig = ((ComputePassNode*)pass)->root_signature;
    if (!root_sig) return nullptr;
    // Allocate or get descriptor set heap
    auto&& table_pool_iter = executor.bind_table_pools.find(root_sig);
    if (table_pool_iter == executor.bind_table_pools.end())
        executor.bind_table_pools.emplace(root_sig, SkrNew<BindTablePool>(root_sig));
    eastl::string bind_table_keys = "";
    // Bind resources
    stack_vector<CGPUDescriptorData> desc_set_updates;
    stack_vector<const char*> bindTableValueNames = {};
    // CBV Buffers
    stack_vector<CGPUBufferId> cbvs(buf_read_edges.size());
    stack_vector<CGPUTextureViewId> srvs(tex_read_edges.size());
    stack_vector<CGPUTextureViewId> uavs(tex_rw_edges.size());
    {
        for (uint32_t e_idx = 0; e_idx < buf_read_edges.size(); e_idx++)
        {
            auto& read_edge = buf_read_edges[e_idx];
            SKR_ASSERT(!read_edge->name.empty());
            // TODO: refactor this
            const auto& resource = *find_shader_resource(read_edge->name_hash, root_sig);

            ECGPUResourceType resource_type = resource.type;
            {
                bind_table_keys += read_edge->name.empty() ? resource.name : read_edge->name;
                bind_table_keys += ";";
                bindTableValueNames.emplace_back(resource.name);

                auto buffer_readed = read_edge->get_buffer_node();
                CGPUDescriptorData update = {};
                update.count = 1;
                update.name = resource.name;
                update.binding_type = resource_type;
                update.binding = resource.binding;
                cbvs[e_idx] = resolve(executor, *buffer_readed);
                update.buffers = &cbvs[e_idx];
                desc_set_updates.emplace_back(update);
            }
        }
        // SRVs
        for (uint32_t e_idx = 0; e_idx < tex_read_edges.size(); e_idx++)
        {
            auto& read_edge = tex_read_edges[e_idx];
            SKR_ASSERT(!read_edge->name.empty());
            const auto& resource = *find_shader_resource(read_edge->name_hash, root_sig);

            {
                bind_table_keys += read_edge->name.empty() ? resource.name : read_edge->name;
                bind_table_keys += ";";
                bindTableValueNames.emplace_back(resource.name);

                auto texture_readed = read_edge->get_texture_node();
                CGPUDescriptorData update = {};
                update.count = 1;
                update.name = resource.name;
                update.binding_type = CGPU_RESOURCE_TYPE_TEXTURE;
                update.binding = resource.binding;
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = resolve(executor, *texture_readed);
                view_desc.base_array_layer = read_edge->get_array_base();
                view_desc.array_layer_count = read_edge->get_array_count();
                view_desc.base_mip_level = read_edge->get_mip_base();
                view_desc.mip_level_count = read_edge->get_mip_count();
                view_desc.format = (ECGPUFormat)view_desc.texture->format;
                const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(view_desc.format);
                const bool is_depth_only = FormatUtil_IsDepthStencilFormat(view_desc.format);
                view_desc.aspects =
                    is_depth_stencil ?
                    is_depth_only ? CGPU_TVA_DEPTH : CGPU_TVA_DEPTH | CGPU_TVA_STENCIL :
                    CGPU_TVA_COLOR;
                view_desc.usages = CGPU_TVU_SRV;
                view_desc.dims = read_edge->get_dimension();
                srvs[e_idx] = texture_view_pool.allocate(view_desc, frame_index);
                update.textures = &srvs[e_idx];
                desc_set_updates.emplace_back(update);
            }
        }
        // UAVs
        for (uint32_t e_idx = 0; e_idx < tex_rw_edges.size(); e_idx++)
        {
            auto& rw_edge = tex_rw_edges[e_idx];
            SKR_ASSERT(!rw_edge->name.empty());
            const auto& resource = *find_shader_resource(rw_edge->name_hash, root_sig);

            {
                bind_table_keys += rw_edge->name.empty() ? resource.name : rw_edge->name;
                bind_table_keys += ";";
                bindTableValueNames.emplace_back(resource.name);

                auto texture_readwrite = rw_edge->get_texture_node();
                CGPUDescriptorData update = {};
                update.count = 1;
                update.name = resource.name;
                update.binding_type = CGPU_RESOURCE_TYPE_RW_TEXTURE;
                update.binding = resource.binding;
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = resolve(executor, *texture_readwrite);
                view_desc.base_array_layer = 0;
                view_desc.array_layer_count = 1;
                view_desc.base_mip_level = 0;
                view_desc.mip_level_count = 1;
                view_desc.aspects = CGPU_TVA_COLOR;
                view_desc.format = (ECGPUFormat)view_desc.texture->format;
                view_desc.usages = CGPU_TVU_UAV;
                view_desc.dims = CGPU_TEX_DIMENSION_2D;
                uavs[e_idx] = texture_view_pool.allocate(view_desc, frame_index);
                update.textures = &uavs[e_idx];
                desc_set_updates.emplace_back(update);
            }
        }
    }
    auto bind_table = executor.bind_table_pools[root_sig]->pop(bind_table_keys.c_str(), bindTableValueNames.data(), (uint32_t)bindTableValueNames.size());
    cgpux_bind_table_update(bind_table, desc_set_updates.data(), (uint32_t)desc_set_updates.size());
    return bind_table;
}

void RenderGraphBackend::deallocate_resources(PassNode* pass) SKR_NOEXCEPT
{
    ZoneScopedN("VirtualDeallocate");
    pass->foreach_textures([this, pass](TextureNode* texture, TextureEdge* edge) {
        if (texture->imported) return;
        bool is_last_user = true;
        texture->foreach_neighbors([&](DependencyGraphNode* neig) {
            RenderGraphNode* rg_node = (RenderGraphNode*)neig;
            if (rg_node->type == EObjectType::Pass)
            {
                PassNode* other_pass = (PassNode*)rg_node;
                is_last_user = is_last_user && (pass->order >= other_pass->order);
            }
        });
        if (is_last_user)
        {
            if (!texture->frame_aliasing)
            {
                texture_pool.deallocate(texture->descriptor, texture->frame_texture,
                    edge->requested_state, { frame_index, texture->tags });
            }
        }
    });
    pass->foreach_buffers([this, pass](BufferNode* buffer, BufferEdge* edge) {
        if (buffer->imported) return;
        bool is_last_user = true;
        buffer->foreach_neighbors([&](DependencyGraphNode* neig) {
            RenderGraphNode* rg_node = (RenderGraphNode*)neig;
            if (rg_node->type == EObjectType::Pass)
            {
                PassNode* other_pass = (PassNode*)rg_node;
                is_last_user = is_last_user && (pass->order >= other_pass->order);
            }
        });
        if (is_last_user)
        {
            buffer_pool.deallocate(buffer->descriptor, buffer->frame_buffer,
                edge->requested_state, { frame_index, buffer->tags });
        }
    });
}

void RenderGraphBackend::execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass) SKR_NOEXCEPT
{
    ZoneScopedC(tracy::Color::LightBlue);
    ZoneName(pass->name.c_str(), pass->name.size());

    ComputePassContext pass_context = {};
    // resource de-virtualize
    stack_vector<CGPUTextureBarrier> tex_barriers = {};
    stack_vector<eastl::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
    stack_vector<CGPUBufferBarrier> buffer_barriers = {};
    stack_vector<eastl::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
    calculate_barriers(executor, pass,
        tex_barriers, resolved_textures,
        buffer_barriers, resolved_buffers);
    // allocate & update descriptor sets
    pass_context.bind_table = alloc_update_pass_bind_table(executor, pass);
    pass_context.resolved_buffers = resolved_buffers;
    pass_context.resolved_textures = resolved_textures;
    pass_context.executor = &executor;
    // call cgpu apis
    CGPUResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    CGPUEventInfo event = { pass->name.c_str(), { 1.f, 1.f, 0.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    // dispatch
    CGPUComputePassDescriptor pass_desc = {};
    pass_desc.name = pass->get_name();
    pass_context.cmd = executor.gfx_cmd_buf;
    pass_context.encoder = cgpu_cmd_begin_compute_pass(executor.gfx_cmd_buf, &pass_desc);
    if(pass->pipeline)
    {
        cgpu_compute_encoder_bind_pipeline(pass_context.encoder, pass->pipeline);
    }
    cgpux_compute_encoder_bind_bind_table(pass_context.encoder, pass_context.bind_table);
    {
        ZoneScopedN("PassExecutor");
        pass->executor(*this, pass_context);
    }
    cgpu_cmd_end_compute_pass(executor.gfx_cmd_buf, pass_context.encoder);
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass) SKR_NOEXCEPT
{
    ZoneScopedC(tracy::Color::LightPink);
    ZoneName(pass->name.c_str(), pass->name.size());

    RenderPassContext pass_context = {};
    // resource de-virtualize
    stack_vector<CGPUTextureBarrier> tex_barriers = {};
    stack_vector<eastl::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
    stack_vector<CGPUBufferBarrier> buffer_barriers = {};
    stack_vector<eastl::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
    calculate_barriers(executor, pass,
        tex_barriers, resolved_textures,
        buffer_barriers, resolved_buffers);
    // allocate & update descriptor sets
    pass_context.bind_table = alloc_update_pass_bind_table(executor, pass);
    pass_context.resolved_buffers = resolved_buffers;
    pass_context.resolved_textures = resolved_textures;
    pass_context.executor = &executor;
    // call cgpu apis
    CGPUResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    CGPUEventInfo event = { pass->name.c_str(), { 1.f, 0.5f, 0.5f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    {
        ZoneScopedN("WriteBarrierMarker");
        executor.write_marker(skr::format("Pass-{}-BeginBarrier", pass->get_name()).c_str());
    }
    // color attachments
    // TODO: MSAA
    stack_vector<CGPUColorAttachment> color_attachments = {};
    CGPUDepthStencilAttachment ds_attachment = {};
    auto write_edges = pass->tex_write_edges();
    for (auto& write_edge : write_edges)
    {
        // TODO: MSAA
        auto texture_target = write_edge->get_texture_node();
        const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(
            (ECGPUFormat)resolve(executor, *texture_target)->format);
        const bool is_depth_only = FormatUtil_IsDepthOnlyFormat(
            (ECGPUFormat)resolve(executor, *texture_target)->format);
        if (write_edge->requested_state == CGPU_RESOURCE_STATE_DEPTH_WRITE && is_depth_stencil)
        {
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = resolve(executor, *texture_target);
            view_desc.base_array_layer = write_edge->get_array_base();
            view_desc.array_layer_count = write_edge->get_array_count();
            view_desc.base_mip_level = write_edge->get_mip_level();
            view_desc.mip_level_count = 1;
            view_desc.aspects =
                is_depth_only ? CGPU_TVA_DEPTH : CGPU_TVA_DEPTH | CGPU_TVA_STENCIL;
            view_desc.format = (ECGPUFormat)view_desc.texture->format;
            view_desc.usages = CGPU_TVU_RTV_DSV;
            view_desc.dims = CGPU_TEX_DIMENSION_2D;
            ds_attachment.view = texture_view_pool.allocate(view_desc, frame_index);
            ds_attachment.depth_load_action = pass->depth_load_action;
            ds_attachment.depth_store_action = pass->depth_store_action;
            ds_attachment.stencil_load_action = pass->stencil_load_action;
            ds_attachment.stencil_store_action = pass->stencil_store_action;
            ds_attachment.clear_depth = 1.f; // TODO:Remove this
            ds_attachment.write_depth = true;
        }
        else
        {
            CGPUColorAttachment attachment = {};
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = resolve(executor, *texture_target);
            view_desc.base_array_layer = write_edge->get_array_base();
            view_desc.array_layer_count = write_edge->get_array_count();
            view_desc.base_mip_level = write_edge->get_mip_level();
            view_desc.mip_level_count = 1; // TODO: mip
            view_desc.format = (ECGPUFormat)view_desc.texture->format;
            view_desc.aspects = CGPU_TVA_COLOR;
            view_desc.usages = CGPU_TVU_RTV_DSV;
            view_desc.dims = CGPU_TEX_DIMENSION_2D;
            attachment.view = texture_view_pool.allocate(view_desc, frame_index);
            attachment.load_action = pass->load_actions[write_edge->mrt_index];
            attachment.store_action = pass->store_actions[write_edge->mrt_index];
            attachment.clear_color = write_edge->clear_value;
            color_attachments.emplace_back(attachment);
        }
    }
    CGPURenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count = (uint32_t)color_attachments.size();
    pass_desc.sample_count = CGPU_SAMPLE_COUNT_1;
    pass_desc.name = pass->get_name();
    pass_desc.color_attachments = color_attachments.data();
    pass_desc.depth_stencil = &ds_attachment;
    pass_context.cmd = executor.gfx_cmd_buf;
    {
        ZoneScopedN("WriteBeginPassMarker");
        executor.write_marker(skr::format("Pass-{}-BeginPass", pass->get_name()).c_str());
    }
    {
        ZoneScopedN("BeginRenderPass");
        pass_context.encoder = cgpu_cmd_begin_render_pass(executor.gfx_cmd_buf, &pass_desc);
    }
    if (pass->pipeline) 
    {
        cgpu_render_encoder_bind_pipeline(pass_context.encoder, pass->pipeline);
    }
    cgpux_render_encoder_bind_bind_table(pass_context.encoder, pass_context.bind_table);
    {
        ZoneScopedN("PassExecutor");
        pass->executor(*this, pass_context);
    }
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, pass_context.encoder);
    {
        ZoneScopedN("WriteEndPassMarker");
        executor.write_marker(skr::format("Pass-{}-EndRenderPass", pass->get_name()).c_str());
    }
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass) SKR_NOEXCEPT
{
    ZoneScopedC(tracy::Color::LightYellow);
    ZoneName(pass->name.c_str(), pass->name.size());
    // resource de-virtualize
    stack_vector<CGPUTextureBarrier> tex_barriers = {};
    stack_vector<eastl::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
    stack_vector<CGPUBufferBarrier> buffer_barriers = {};
    stack_vector<eastl::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
    calculate_barriers(executor, pass,
        tex_barriers, resolved_textures,
        buffer_barriers, resolved_buffers);
    // call cgpu apis
    CGPUResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    CGPUEventInfo event = { pass->name.c_str(), { 0.f, .5f, 1.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    {
        CopyPassContext stack = {};
        stack.cmd = executor.gfx_cmd_buf;
        stack.resolved_buffers = resolved_buffers;
        stack.resolved_textures = resolved_textures;
        pass->executor(*this, stack);
    }
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    for (uint32_t i = 0; i < pass->t2ts.size(); i++)
    {
        auto src_node = RenderGraph::resolve(pass->t2ts[i].first);
        auto dst_node = RenderGraph::resolve(pass->t2ts[i].second);
        CGPUTextureToTextureTransfer t2t = {};
        t2t.src = resolve(executor, *src_node);
        t2t.src_subresource.aspects = pass->t2ts[i].first.aspects;
        t2t.src_subresource.mip_level = pass->t2ts[i].first.mip_level;
        t2t.src_subresource.base_array_layer = pass->t2ts[i].first.array_base;
        t2t.src_subresource.layer_count = pass->t2ts[i].first.array_count;
        t2t.dst = resolve(executor, *dst_node);
        t2t.dst_subresource.aspects = pass->t2ts[i].second.aspects;
        t2t.dst_subresource.mip_level = pass->t2ts[i].second.mip_level;
        t2t.dst_subresource.base_array_layer = pass->t2ts[i].second.array_base;
        t2t.dst_subresource.layer_count = pass->t2ts[i].second.array_count;
        cgpu_cmd_transfer_texture_to_texture(executor.gfx_cmd_buf, &t2t);
    }
    for (uint32_t i = 0; i < pass->b2bs.size(); i++)
    {
        auto src_node = RenderGraph::resolve(pass->b2bs[i].first);
        auto dst_node = RenderGraph::resolve(pass->b2bs[i].second);
        CGPUBufferToBufferTransfer b2b = {};
        b2b.src = resolve(executor, *src_node);
        b2b.src_offset = pass->b2bs[i].first.from;
        b2b.dst = resolve(executor, *dst_node);
        b2b.dst_offset = pass->b2bs[i].second.from;
        b2b.size = pass->b2bs[i].first.to - b2b.src_offset;
        cgpu_cmd_transfer_buffer_to_buffer(executor.gfx_cmd_buf, &b2b);
    }
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass) SKR_NOEXCEPT
{
    auto read_edges = pass->tex_read_edges();
    auto&& read_edge = read_edges[0];
    auto texture_target = read_edge->get_texture_node();
    auto back_buffer = pass->descriptor.swapchain->back_buffers[pass->descriptor.index];
    CGPUTextureBarrier present_barrier = {};
    present_barrier.texture = back_buffer;
    present_barrier.src_state = get_lastest_state(texture_target, pass);
    present_barrier.dst_state = CGPU_RESOURCE_STATE_PRESENT;
    CGPUResourceBarrierDescriptor barriers = {};
    barriers.texture_barriers = &present_barrier;
    barriers.texture_barriers_count = 1;
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
}

uint64_t RenderGraphBackend::execute(RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    const auto executor_index = frame_index % RG_MAX_FRAME_IN_FLIGHT;
    RenderGraphFrameExecutor& executor = executors[executor_index];
    if (device->is_lost)
    {
        for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
        {
            executors[i].print_error_trace(frame_index);
        }
        SKR_BREAK();
    }
    {
        ZoneScopedN("AcquireExecutor");
        cgpu_wait_fences(&executor.exec_fence, 1);
        if (profiler) profiler->on_acquire_executor(*this, executor);
    }
    {
        ZoneScopedN("GraphExecutePasses");
        executor.reset_begin(texture_view_pool);
        if (profiler) profiler->on_cmd_begin(*this, executor);
        {
            ZoneScopedN("GraphExecutorBeginEvent");

            skr::string frameLabel = "Frame";
            frameLabel.append(skr::to_string(frame_index));
            CGPUEventInfo event = { frameLabel.c_str(), { 0.8f, 0.8f, 0.8f, 1.f } };
            cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
        }
        for (auto& pass : passes)
        {
            if (pass->pass_type == EPassType::Render)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_render_pass(executor, static_cast<RenderPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Present)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_present_pass(executor, static_cast<PresentPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Compute)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_compute_pass(executor, static_cast<ComputePassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Copy)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_copy_pass(executor, static_cast<CopyPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
        }
        {
            cgpu_cmd_end_event(executor.gfx_cmd_buf);
        }
        if (profiler) profiler->on_cmd_end(*this, executor);
        cgpu_cmd_end(executor.gfx_cmd_buf);
    }
    {
        // submit
        ZoneScopedN("GraphQueueSubmit");
        if (profiler) profiler->before_commit(*this, executor);
        {
            ZoneScopedN("CGPUGfxQueueSubmit");
            executor.commit(gfx_queue, frame_index);
        }
        if (profiler) profiler->after_commit(*this, executor);
    }
    {
        ZoneScopedN("GraphCleanup");
        // 1.dealloc culled resources
        for (auto culled_resource : culled_resources)
        {
            object_factory->Dealloc(culled_resource);
        }
        culled_resources.clear();
        // 2.dealloc culled passes 
        for (auto culled_pass : culled_passes)
        {
            object_factory->Dealloc(culled_pass);
        }
        culled_passes.clear();
        // 3.dealloc passes & connected edges 
        for (auto pass : passes)
        {
            pass->foreach_textures(
            [this](TextureNode* t, TextureEdge* e) {
                object_factory->Dealloc(e);
            });
            pass->foreach_buffers(
            [this](BufferNode* t, BufferEdge* e) {
                object_factory->Dealloc(e);
            });
            object_factory->Dealloc(pass);
        }
        passes.clear();
        // 4.dealloc resource nodes
        for (auto resource : resources)
        {
            object_factory->Dealloc(resource);
        }
        resources.clear();

        graph->clear();
        blackboard->clear();
    }
    return frame_index++;
}

CGPUDeviceId RenderGraphBackend::get_backend_device() SKR_NOEXCEPT { return device; }

uint32_t RenderGraphBackend::collect_garbage(uint64_t critical_frame,
    uint32_t tex_with_tags, uint32_t tex_without_tags,
    uint32_t buf_with_tags, uint32_t buf_without_tags) SKR_NOEXCEPT
{
    return collect_texture_garbage(critical_frame, tex_with_tags, tex_without_tags) 
        + collect_buffer_garbage(critical_frame, buf_with_tags, buf_without_tags);
}

uint32_t RenderGraphBackend::collect_texture_garbage(uint64_t critical_frame, uint32_t with_tags, uint32_t without_tags) SKR_NOEXCEPT
{
    if (critical_frame > get_latest_finished_frame())
    {
        SKR_LOG_ERROR("undone frame on GPU detected, collect texture garbage may cause GPU Crash!!"
                      "\n\tcurrent: %d, latest finished: %d", critical_frame, get_latest_finished_frame());
    }
    uint32_t total_count = 0;
    for (auto&& [key, queue] : texture_pool.textures)
    {
        for (auto&& pooled: queue)
        {
            if (pooled.mark.frame_index <= critical_frame 
                && (pooled.mark.tags & with_tags) && !(pooled.mark.tags & without_tags))
            {
                texture_view_pool.erase(pooled.texture);
                cgpu_free_texture(pooled.texture);
                pooled.texture = nullptr;
            }
        }
        uint32_t prev_count = (uint32_t)queue.size();
        queue.erase(
            eastl::remove_if(queue.begin(), queue.end(),
            [&](auto& element) {
                return element.texture == nullptr;
            }),
            queue.end());
        total_count += prev_count - (uint32_t)queue.size();
    }
    return total_count;
}

uint32_t RenderGraphBackend::collect_buffer_garbage(uint64_t critical_frame, uint32_t with_tags, uint32_t without_tags) SKR_NOEXCEPT
{
    if (critical_frame > get_latest_finished_frame())
    {
        SKR_LOG_ERROR("undone frame on GPU detected, collect buffer garbage may cause GPU Crash!!"
                      "\n\tcurrent: %d, latest finished: %d", critical_frame, get_latest_finished_frame());
    }
    uint32_t total_count = 0;
    for (auto&& [key, queue] : buffer_pool.buffers)
    {
        for (auto&& pooled : queue)
        {
            if (pooled.mark.frame_index <= critical_frame 
                && (pooled.mark.tags & with_tags) && !(pooled.mark.tags & without_tags))
            {
                cgpu_free_buffer(pooled.buffer);
                pooled.buffer = nullptr;
            }
        }
        uint32_t prev_count = (uint32_t)queue.size();
        queue.erase(
            eastl::remove_if(queue.begin(), queue.end(),
                [&](auto& element) {
                    return element.buffer == nullptr;
                }),
                queue.end());
        total_count += prev_count - (uint32_t)queue.size();
    }
    return total_count;
}
} // namespace render_graph
} // namespace skr