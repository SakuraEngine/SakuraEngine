#include "render_graph/backend/graph_backend.hpp"
#include "../cgpu/common/common_utils.h"
#include "tracy/Tracy.hpp"

namespace sakura
{
namespace render_graph
{
RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup)
{
    RenderGraphBuilder builder = {};
    RenderGraph* graph = nullptr;
    setup(builder);
    if (builder.no_backend)
        graph = new RenderGraph();
    else
    {
        if (!builder.gfx_queue) assert(0 && "not supported!");
        graph = new RenderGraphBackend(builder.gfx_queue, builder.device);
    }
    graph->initialize();
    return graph;
}

void RenderGraph::destroy(RenderGraph* g)
{
    g->finalize();
    delete g;
}

void RenderGraphBackend::initialize()
{
    backend = device->adapter->instance->backend;
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].initialize(gfx_queue, device);
    }
    buffer_pool.initialize(device);
    texture_pool.initialize(device);
    texture_view_pool.initialize(device);
}

void RenderGraphBackend::finalize()
{
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].finalize();
    }
    buffer_pool.finalize();
    texture_pool.finalize();
    texture_view_pool.finalize();
}

CGpuTextureId RenderGraphBackend::resolve(const TextureNode& node)
{
    ZoneScopedN("ResolveTexture");
    if (!node.frame_texture)
    {
        auto& wnode = const_cast<TextureNode&>(node);
        auto allocated = texture_pool.allocate(node.descriptor, frame_index);
        wnode.frame_texture = node.imported ?
                                  node.frame_texture :
                                  allocated.first;
        wnode.init_state = allocated.second;
    }
    return node.frame_texture;
}

CGpuBufferId RenderGraphBackend::resolve(const BufferNode& node)
{
    ZoneScopedN("ResolveBuffer");
    if (!node.frame_buffer)
    {
        auto& wnode = const_cast<BufferNode&>(node);
        auto allocated = buffer_pool.allocate(node.descriptor, frame_index);
        wnode.frame_buffer = node.imported ?
                                 node.frame_buffer :
                                 allocated.first;
        wnode.init_state = allocated.second;
    }
    return node.frame_buffer;
}

void RenderGraphBackend::calculate_barriers(PassNode* pass,
    eastl::vector<CGpuTextureBarrier>& tex_barriers, eastl::vector<eastl::pair<TextureHandle, CGpuTextureId>>& resolved_textures,
    eastl::vector<CGpuBufferBarrier>& buf_barriers, eastl::vector<eastl::pair<BufferHandle, CGpuBufferId>>& resolved_buffers)
{
    ZoneScopedN("CalculateBarriers");
    tex_barriers.reserve(pass->textures_count());
    resolved_textures.reserve(pass->textures_count());
    buf_barriers.reserve(pass->buffers_count());
    resolved_buffers.reserve(pass->buffers_count());
    pass->foreach_textures(
        [&](TextureNode* texture, TextureEdge* edge) {
            auto tex_resolved = resolve(*texture);
            resolved_textures.emplace_back(texture->get_handle(), tex_resolved);
            const auto current_state = get_lastest_state(texture, pass);
            const auto dst_state = edge->requested_state;
            if (current_state == dst_state) return;
            CGpuTextureBarrier barrier = {};
            barrier.src_state = current_state;
            barrier.dst_state = dst_state;
            barrier.texture = tex_resolved;
            tex_barriers.emplace_back(barrier);
        });
    pass->foreach_buffers(
        [&](BufferNode* buffer, BufferEdge* edge) {
            auto buf_resolved = resolve(*buffer);
            resolved_buffers.emplace_back(buffer->get_handle(), buf_resolved);
            const auto current_state = get_lastest_state(buffer, pass);
            const auto dst_state = edge->requested_state;
            if (current_state == dst_state) return;
            CGpuBufferBarrier barrier = {};
            barrier.src_state = current_state;
            barrier.dst_state = dst_state;
            barrier.buffer = buf_resolved;
            buf_barriers.emplace_back(barrier);
        });
}

eastl::pair<uint32_t, uint32_t> calculate_bind_set(const char8_t* name, CGpuRootSignatureId root_sig)
{
    uint32_t set = 0, binding = 0;
    auto name_hash = cgpu_hash(name, strlen(name), (size_t)root_sig->device);
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        for (uint32_t j = 0; j < root_sig->tables[i].resources_count; j++)
        {
            if (root_sig->tables[i].resources[j].name_hash == name_hash)
            {
                set = root_sig->tables[i].resources[j].set;
                binding = root_sig->tables[i].resources[j].binding;
                return { set, binding };
            }
        }
    }
    return { UINT32_MAX, UINT32_MAX };
}

gsl::span<CGpuDescriptorSetId> RenderGraphBackend::alloc_update_pass_descsets(RenderGraphFrameExecutor& executor, PassNode* pass)
{
    ZoneScopedN("UpdateBindings");
    CGpuRootSignatureId root_sig = nullptr;
    auto read_edges = pass->tex_read_edges();
    auto rw_edges = pass->tex_readwrite_edges();
    if (pass->pass_type == EPassType::Render)
        root_sig = ((RenderPassNode*)pass)->pipeline->root_signature;
    else if (pass->pass_type == EPassType::Compute)
        root_sig = ((ComputePassNode*)pass)->pipeline->root_signature;
    auto&& desc_set_heap = executor.desc_set_pool.find(root_sig);
    if (desc_set_heap == executor.desc_set_pool.end())
        executor.desc_set_pool.insert({ root_sig, new DescSetHeap(root_sig) });
    auto desc_sets = executor.desc_set_pool[root_sig]->pop();
    for (uint32_t set_idx = 0; set_idx < desc_sets.size(); set_idx++)
    {
        eastl::vector<CGpuDescriptorData> desc_set_updates;
        // SRVs
        eastl::vector<CGpuTextureViewId> srvs(read_edges.size());
        for (uint32_t e_idx = 0; e_idx < read_edges.size(); e_idx++)
        {
            auto& read_edge = read_edges[e_idx];
            auto read_set_binding =
                read_edge->name.empty() ?
                    eastl::pair<uint32_t, uint32_t>(read_edge->set, read_edge->binding) :
                    calculate_bind_set(read_edge->name.c_str(), root_sig);
            if (read_set_binding.first == set_idx)
            {
                auto texture_readed = read_edge->get_texture_node();
                CGpuDescriptorData update = {};
                update.count = 1;
                update.binding = read_set_binding.second;
                update.binding_type = RT_TEXTURE;
                CGpuTextureViewDescriptor view_desc = {};
                view_desc.texture = resolve(*texture_readed);
                view_desc.base_array_layer = read_edge->get_array_base();
                view_desc.array_layer_count = read_edge->get_array_count();
                view_desc.base_mip_level = read_edge->get_mip_base();
                view_desc.mip_level_count = read_edge->get_mip_count();
                view_desc.format = (ECGpuFormat)view_desc.texture->format;
                const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(view_desc.format);
                const bool is_depth_only = FormatUtil_IsDepthStencilFormat(view_desc.format);
                view_desc.aspects =
                    is_depth_stencil ?
                        is_depth_only ? TVA_DEPTH : TVA_DEPTH | TVA_STENCIL :
                        TVA_COLOR;
                view_desc.usages = TVU_SRV;
                view_desc.dims = read_edge->get_dimension();
                srvs[e_idx] = texture_view_pool.allocate(view_desc, frame_index);
                update.textures = &srvs[e_idx];
                desc_set_updates.emplace_back(update);
            }
        }
        // UAVs
        eastl::vector<CGpuTextureViewId> uavs(rw_edges.size());
        for (uint32_t e_idx = 0; e_idx < rw_edges.size(); e_idx++)
        {
            auto& rw_edge = rw_edges[e_idx];
            auto rw_set_binding =
                rw_edge->name.empty() ?
                    eastl::pair<uint32_t, uint32_t>(rw_edge->set, rw_edge->binding) :
                    calculate_bind_set(rw_edge->name.c_str(), root_sig);
            if (rw_set_binding.first == set_idx)
            {
                auto texture_readwrite = rw_edge->get_texture_node();
                CGpuDescriptorData update = {};
                update.count = 1;
                update.binding = rw_set_binding.second;
                update.binding_type = RT_RW_TEXTURE;
                CGpuTextureViewDescriptor view_desc = {};
                view_desc.texture = resolve(*texture_readwrite);
                view_desc.base_array_layer = 0;
                view_desc.array_layer_count = 1;
                view_desc.base_mip_level = 0;
                view_desc.mip_level_count = 1;
                view_desc.aspects = TVA_COLOR;
                view_desc.format = (ECGpuFormat)view_desc.texture->format;
                view_desc.usages = TVU_UAV;
                view_desc.dims = TEX_DIMENSION_2D;
                uavs[e_idx] = texture_view_pool.allocate(view_desc, frame_index);
                update.textures = &uavs[e_idx];
                desc_set_updates.emplace_back(update);
            }
        }
        auto update_count = desc_set_updates.size();
        if (update_count)
        {
            cgpu_update_descriptor_set(desc_sets[set_idx],
                desc_set_updates.data(), desc_set_updates.size());
        }
    }
    return desc_sets;
}

void RenderGraphBackend::deallocate_resources(PassNode* pass)
{
    ZoneScopedN("VirtualDeallocate");
    pass->foreach_textures(
        [this, pass](TextureNode* texture, TextureEdge* edge) {
            if (texture->imported) return;
            bool is_last_user = true;
            texture->foreach_neighbors(
                [&](DependencyGraphNode* neig) {
                    RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                    if (rg_node->type == EObjectType::Pass)
                    {
                        PassNode* other_pass = (PassNode*)rg_node;
                        is_last_user = is_last_user && (pass->order >= other_pass->order);
                    }
                });
            if (is_last_user)
                texture_pool.deallocate(texture->descriptor,
                    texture->frame_texture,
                    edge->requested_state,
                    frame_index);
        });
    pass->foreach_buffers(
        [this, pass](BufferNode* buffer, BufferEdge* edge) {
            if (buffer->imported) return;
            bool is_last_user = true;
            buffer->foreach_neighbors(
                [&](DependencyGraphNode* neig) {
                    RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                    if (rg_node->type == EObjectType::Pass)
                    {
                        PassNode* other_pass = (PassNode*)rg_node;
                        is_last_user = is_last_user && (pass->order >= other_pass->order);
                    }
                });
            if (is_last_user)
                buffer_pool.deallocate(buffer->descriptor,
                    buffer->frame_buffer,
                    edge->requested_state,
                    frame_index);
        });
}

void RenderGraphBackend::execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass)
{
    ZoneScopedC(tracy::Color::LightBlue);
    ZoneName(pass->name.c_str(), pass->name.size());
    ComputePassContext stack = {};
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    eastl::vector<eastl::pair<TextureHandle, CGpuTextureId>> resolved_textures = {};
    eastl::vector<CGpuBufferBarrier> buffer_barriers = {};
    eastl::vector<eastl::pair<BufferHandle, CGpuBufferId>> resolved_buffers = {};
    calculate_barriers(pass, tex_barriers, resolved_textures, buffer_barriers, resolved_buffers);
    // allocate & update descriptor sets
    stack.desc_sets = alloc_update_pass_descsets(executor, pass);
    stack.resolved_buffers = resolved_buffers;
    stack.resolved_textures = resolved_textures;
    // call cgpu apis
    CGpuResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = buffer_barriers.size();
    }
    CGpuEventInfo event = { pass->name.c_str(), { 1.f, 1.f, 1.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    CGpuMarkerInfo barrier_marker = { "barriers finish", { 1.f, 1.f, 1.f, 1.f } };
    cgpu_cmd_set_marker(executor.gfx_cmd_buf, &barrier_marker);
    // dispatch
    CGpuComputePassDescriptor pass_desc = {};
    pass_desc.name = pass->get_name();
    stack.encoder = cgpu_cmd_begin_compute_pass(executor.gfx_cmd_buf, &pass_desc);
    cgpu_compute_encoder_bind_pipeline(stack.encoder, pass->pipeline);
    for (auto desc_set : stack.desc_sets)
    {
        cgpu_compute_encoder_bind_descriptor_set(stack.encoder, desc_set);
    }
    {
        ZoneScopedN("PassExecutor");
        pass->executor(*this, stack);
    }
    cgpu_cmd_end_compute_pass(executor.gfx_cmd_buf, stack.encoder);
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass)
{
    ZoneScopedC(tracy::Color::LightPink);
    ZoneName(pass->name.c_str(), pass->name.size());
    RenderPassContext stack = {};
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    eastl::vector<eastl::pair<TextureHandle, CGpuTextureId>> resolved_textures = {};
    eastl::vector<CGpuBufferBarrier> buffer_barriers = {};
    eastl::vector<eastl::pair<BufferHandle, CGpuBufferId>> resolved_buffers = {};
    calculate_barriers(pass, tex_barriers, resolved_textures, buffer_barriers, resolved_buffers);
    // allocate & update descriptor sets
    stack.desc_sets = alloc_update_pass_descsets(executor, pass);
    stack.resolved_buffers = resolved_buffers;
    stack.resolved_textures = resolved_textures;
    // call cgpu apis
    CGpuResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = buffer_barriers.size();
    }
    CGpuEventInfo event = { pass->name.c_str(), { 1.f, 1.f, 1.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    CGpuMarkerInfo barrier_marker = { "barriers finish", { 1.f, 1.f, 1.f, 1.f } };
    // color attachments
    // TODO: MSAA
    eastl::vector<CGpuColorAttachment> color_attachments = {};
    CGpuDepthStencilAttachment ds_attachment = {};
    auto write_edges = pass->tex_write_edges();
    for (auto& write_edge : write_edges)
    {
        // TODO: MSAA
        auto texture_target = write_edge->get_texture_node();
        const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(
            (ECGpuFormat)resolve(*texture_target)->format);
        const bool is_depth_only = FormatUtil_IsDepthStencilFormat(
            (ECGpuFormat)resolve(*texture_target)->format);
        if (write_edge->requested_state == RESOURCE_STATE_DEPTH_WRITE && is_depth_stencil)
        {
            CGpuTextureViewDescriptor view_desc = {};
            view_desc.texture = resolve(*texture_target);
            view_desc.base_array_layer = write_edge->get_array_base();
            view_desc.array_layer_count = write_edge->get_array_count();
            view_desc.base_mip_level = write_edge->get_mip_level();
            view_desc.mip_level_count = 1;
            view_desc.aspects =
                is_depth_only ? TVA_DEPTH : TVA_DEPTH | TVA_STENCIL;
            view_desc.format = (ECGpuFormat)view_desc.texture->format;
            view_desc.usages = TVU_RTV_DSV;
            view_desc.dims = TEX_DIMENSION_2D;
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
            CGpuColorAttachment attachment = {};
            CGpuTextureViewDescriptor view_desc = {};
            view_desc.texture = resolve(*texture_target);
            view_desc.base_array_layer = write_edge->get_array_base();
            view_desc.array_layer_count = write_edge->get_array_count();
            view_desc.base_mip_level = write_edge->get_mip_level();
            view_desc.mip_level_count = 1;
            view_desc.format = (ECGpuFormat)view_desc.texture->format;
            view_desc.aspects = TVA_COLOR;
            view_desc.usages = TVU_RTV_DSV;
            view_desc.dims = TEX_DIMENSION_2D;
            attachment.view = texture_view_pool.allocate(view_desc, frame_index);
            attachment.load_action = pass->load_actions[write_edge->mrt_index];
            attachment.store_action = pass->store_actions[write_edge->mrt_index];
            color_attachments.emplace_back(attachment);
        }
    }
    CGpuRenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count = color_attachments.size();
    pass_desc.sample_count = SAMPLE_COUNT_1;
    pass_desc.name = pass->get_name();
    pass_desc.color_attachments = color_attachments.data();
    pass_desc.depth_stencil = &ds_attachment;
    stack.encoder = cgpu_cmd_begin_render_pass(executor.gfx_cmd_buf, &pass_desc);
    cgpu_render_encoder_bind_pipeline(stack.encoder, pass->pipeline);
    for (auto desc_set : stack.desc_sets)
    {
        cgpu_render_encoder_bind_descriptor_set(stack.encoder, desc_set);
    }
    {
        ZoneScopedN("PassExecutor");
        pass->executor(*this, stack);
    }
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, stack.encoder);
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass)
{
    ZoneScopedC(tracy::Color::LightYellow);
    ZoneName(pass->name.c_str(), pass->name.size());
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    eastl::vector<eastl::pair<TextureHandle, CGpuTextureId>> resolved_textures = {};
    eastl::vector<CGpuBufferBarrier> buffer_barriers = {};
    eastl::vector<eastl::pair<BufferHandle, CGpuBufferId>> resolved_buffers = {};
    calculate_barriers(pass, tex_barriers, resolved_textures, buffer_barriers, resolved_buffers);
    // call cgpu apis
    CGpuResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = buffer_barriers.size();
    }
    CGpuEventInfo event = { pass->name.c_str(), { 1.f, 1.f, 1.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    for (uint32_t i = 0; i < pass->t2ts.size(); i++)
    {
        auto src_node = RenderGraph::resolve(pass->t2ts[i].first);
        auto dst_node = RenderGraph::resolve(pass->t2ts[i].second);
        CGpuTextureToTextureTransfer t2t = {};
        t2t.src = resolve(*src_node);
        t2t.src_subresource.aspects = pass->t2ts[i].first.aspects;
        t2t.src_subresource.mip_level = pass->t2ts[i].first.mip_level;
        t2t.src_subresource.base_array_layer = pass->t2ts[i].first.array_base;
        t2t.src_subresource.layer_count = pass->t2ts[i].first.array_count;
        t2t.dst = resolve(*dst_node);
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
        CGpuBufferToBufferTransfer b2b = {};
        b2b.src = resolve(*src_node);
        b2b.src_offset = pass->b2bs[i].first.from;
        b2b.dst = resolve(*dst_node);
        b2b.dst_offset = pass->b2bs[i].second.from;
        b2b.size = pass->b2bs[i].first.to - b2b.src_offset;
        cgpu_cmd_transfer_buffer_to_buffer(executor.gfx_cmd_buf, &b2b);
    }
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass)
{
    auto read_edges = pass->tex_read_edges();
    auto&& read_edge = read_edges[0];
    auto texture_target = read_edge->get_texture_node();
    auto back_buffer = pass->descriptor.swapchain->back_buffers[pass->descriptor.index];
    CGpuTextureBarrier present_barrier = {};
    present_barrier.texture = back_buffer;
    present_barrier.src_state = get_lastest_state(texture_target, pass);
    present_barrier.dst_state = RESOURCE_STATE_PRESENT;
    CGpuResourceBarrierDescriptor barriers = {};
    barriers.texture_barriers = &present_barrier;
    barriers.texture_barriers_count = 1;
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
}

uint64_t RenderGraphBackend::execute()
{
    const auto executor_index = frame_index % RG_MAX_FRAME_IN_FLIGHT;
    RenderGraphFrameExecutor& executor = executors[executor_index];
    {
        ZoneScopedN("AcquireExecutor");
        cgpu_wait_fences(&executor.exec_fence, 1);
        for (auto desc_heap : executor.desc_set_pool)
        {
            desc_heap.second->reset();
        }
    }
    {
        ZoneScopedN("GraphExecutePasses");
        cgpu_reset_command_pool(executor.gfx_cmd_pool);
        cgpu_cmd_begin(executor.gfx_cmd_buf);
        for (auto& pass : passes)
        {
            if (pass->pass_type == EPassType::Render)
            {
                execute_render_pass(executor, static_cast<RenderPassNode*>(pass));
            }
            else if (pass->pass_type == EPassType::Present)
            {
                execute_present_pass(executor, static_cast<PresentPassNode*>(pass));
            }
            else if (pass->pass_type == EPassType::Compute)
            {
                execute_compute_pass(executor, static_cast<ComputePassNode*>(pass));
            }
            else if (pass->pass_type == EPassType::Copy)
            {
                execute_copy_pass(executor, static_cast<CopyPassNode*>(pass));
            }
        }
        cgpu_cmd_end(executor.gfx_cmd_buf);
    }
    {
        // submit
        ZoneScopedN("GraphQueueSubmit");
        CGpuQueueSubmitDescriptor submit_desc = {};
        submit_desc.cmds = &executor.gfx_cmd_buf;
        submit_desc.cmds_count = 1;
        submit_desc.signal_fence = executor.exec_fence;
        cgpu_submit_queue(gfx_queue, &submit_desc);
    }
    {
        ZoneScopedN("GraphCleanup");
        for (auto culled_resource : culled_resources)
        {
            delete culled_resource;
        }
        culled_resources.clear();
        for (auto culled_pass : culled_passes)
        {
            delete culled_pass;
        }
        culled_passes.clear();
        for (auto pass : passes)
        {
            pass->foreach_textures(
                +[](TextureNode* t, TextureEdge* e) {
                    delete e;
                });
            pass->foreach_buffers(
                +[](BufferNode* t, BufferEdge* e) {
                    delete e;
                });
            delete pass;
        }
        passes.clear();
        for (auto resource : resources)
        {
            delete resource;
        }
        resources.clear();
        graph->clear();
        blackboard.clear();
    }
    return frame_index++;
}

CGpuDeviceId RenderGraphBackend::get_backend_device() { return device; }

uint32_t RenderGraphBackend::collect_garbage(uint64_t critical_frame)
{
    return collect_texture_garbage(critical_frame) +
           collect_buffer_garbage(critical_frame);
}

uint32_t RenderGraphBackend::collect_texture_garbage(uint64_t critical_frame)
{
    uint32_t total_count = 0;
    for (auto&& iter : texture_pool.textures)
    {
        auto&& queue = iter.second;
        for (auto&& element : queue)
        {
            if (element.second <= critical_frame)
            {
                texture_view_pool.erase(element.first.first);
                cgpu_free_texture(element.first.first);
                element.first.first = nullptr;
            }
        }
        using ElementType = decltype(queue.front());
        uint32_t prev_count = queue.size();
        queue.erase(
            eastl::remove_if(queue.begin(), queue.end(),
                [&](ElementType& element) {
                    return element.first.first == nullptr;
                }),
            queue.end());
        total_count += prev_count - queue.size();
    }
    return total_count;
}

uint32_t RenderGraphBackend::collect_buffer_garbage(uint64_t critical_frame)
{
    uint32_t total_count = 0;
    for (auto&& iter : buffer_pool.buffers)
    {
        auto&& queue = iter.second;
        for (auto&& element : queue)
        {
            if (element.second <= critical_frame)
            {
                cgpu_free_buffer(element.first.first);
                element.first.first = nullptr;
            }
        }
        using ElementType = decltype(queue.front());
        uint32_t prev_count = queue.size();
        queue.erase(
            eastl::remove_if(queue.begin(), queue.end(),
                [&](ElementType& element) {
                    return element.first.first == nullptr;
                }),
            queue.end());
        total_count += prev_count - queue.size();
    }
    return total_count;
}
} // namespace render_graph
} // namespace sakura