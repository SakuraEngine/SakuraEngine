#include "render_graph/backend/graph_backend.hpp"
#include "../cgpu/common/common_utils.h"

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

const ResourceNode::LifeSpan ResourceNode::lifespan() const
{
    uint32_t from = UINT32_MAX, to = 0;
    foreach_neighbors([&](const DependencyGraphNode* node) {
        auto rg_node = static_cast<const RenderGraphNode*>(node);
        if (rg_node->type == EObjectType::Pass)
        {
            auto pass_node = static_cast<const RenderPassNode*>(node);
            from = (from <= pass_node->order) ? from : pass_node->order;
            to = (to >= pass_node->order) ? to : pass_node->order;
        }
    });
    foreach_inv_neighbors([&](const DependencyGraphNode* node) {
        auto rg_node = static_cast<const RenderGraphNode*>(node);
        if (rg_node->type == EObjectType::Pass)
        {
            auto pass_node = static_cast<const RenderPassNode*>(node);
            from = (from <= pass_node->order) ? from : pass_node->order;
            to = (to >= pass_node->order) ? to : pass_node->order;
        }
    });
    return { from, to };
}

bool RenderGraph::compile()
{
    // TODO: cull
    // 1.init resources states
    for (auto& resource : resources)
    {
        switch (resource->type)
        {
            case EObjectType::Texture: {
                auto texture = static_cast<TextureNode*>(resource);
                // texture->init_state = RESOURCE_STATE_COMMON;
                (void)texture;
            }
            break;
            default:
                break;
        }
    }
    // 2.calc lifespan
    for (auto& pass : passes)
    {
        graph->foreach_incoming_edges(pass,
            [=](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* edge) {
                auto rg_from = static_cast<RenderGraphNode*>(from);
                // auto rg_to = static_cast<RenderGraphNode*>(to);
                if (rg_from->type == EObjectType::Texture)
                {
                    // auto texture_from = static_cast<TextureNode*>(rg_from);
                }
            });
    }
    return true;
}

uint32_t RenderGraph::foreach_writer_passes(TextureHandle texture,
    eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_incoming_edges(
        texture,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(from);
            TextureNode* texture = static_cast<TextureNode*>(to);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
}

uint32_t RenderGraph::foreach_reader_passes(TextureHandle texture,
    eastl::function<void(PassNode*, TextureNode*, RenderGraphEdge*)> f) const
{
    return graph->foreach_outgoing_edges(
        texture,
        [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
            PassNode* pass = static_cast<PassNode*>(to);
            TextureNode* texture = static_cast<TextureNode*>(from);
            RenderGraphEdge* edge = static_cast<RenderGraphEdge*>(e);
            f(pass, texture, edge);
        });
}

const ECGpuResourceState RenderGraph::get_lastest_state(
    const TextureNode* texture, const PassNode* pending_pass) const
{
    if (passes[0] == pending_pass)
        return texture->init_state;
    PassNode* pass_iter = nullptr;
    auto result = texture->init_state;
    foreach_writer_passes(texture->get_handle(),
        [&](PassNode* pass, TextureNode* texture, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::TextureWrite)
            {
                auto write_edge = static_cast<TextureRenderEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = write_edge->requested_state;
                }
            }
            else if (edge->type == ERelationshipType::TextureReadWrite)
            {
                auto rw_edge = static_cast<TextureReadWriteEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = rw_edge->requested_state;
                }
            }
        });
    foreach_reader_passes(texture->get_handle(),
        [&](PassNode* pass, TextureNode* texture, RenderGraphEdge* edge) {
            if (edge->type == ERelationshipType::TextureRead)
            {
                auto read_edge = static_cast<TextureRenderEdge*>(edge);
                if (pass->after(pass_iter) && pass->before(pending_pass))
                {
                    pass_iter = pass;
                    result = read_edge->requested_state;
                }
            }
        });
    return result;
}

uint64_t RenderGraph::execute()
{
    graph->clear();
    return frame_index++;
}

void RenderGraph::initialize()
{
}

void RenderGraph::finalize()
{
}

void RenderGraphBackend::initialize()
{
    backend = device->adapter->instance->backend;
    for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].initialize(gfx_queue, device);
    }
    texture_pool.initialize(device);
    texture_view_pool.initialize(device);
}

void RenderGraphBackend::finalize()
{
    for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].finalize();
    }
    texture_pool.finalize();
    texture_view_pool.finalize();
    for (auto desc_set_heap : desc_set_pool)
    {
        desc_set_heap.second->destroy();
    }
}

CGpuTextureId RenderGraphBackend::resolve(const TextureNode& node)
{
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

void RenderGraphBackend::calculate_barriers(PassNode* pass, eastl::vector<CGpuTextureBarrier>& tex_barriers)
{
    auto read_edges = pass->read_edges();
    auto write_edges = pass->write_edges();
    auto rw_edges = pass->readwrite_edges();
    for (auto& read_edge : read_edges)
    {
        auto texture_readed = read_edge->get_texture_node();
        auto tex_resolved = resolve(*texture_readed);
        const auto current_state = get_lastest_state(texture_readed, pass);
        const auto dst_state = read_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = tex_resolved;
        tex_barriers.emplace_back(barrier);
    }
    for (auto& write_edge : write_edges)
    {
        auto texture_target = write_edge->get_texture_node();
        auto tex_resolved = resolve(*texture_target);
        const auto current_state = get_lastest_state(texture_target, pass);
        const auto dst_state = write_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = tex_resolved;
        tex_barriers.emplace_back(barrier);
    }
    for (auto& rw_edge : rw_edges)
    {
        auto texture_target = rw_edge->get_texture_node();
        auto tex_resolved = resolve(*texture_target);
        const auto current_state = get_lastest_state(texture_target, pass);
        const auto dst_state = rw_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = tex_resolved;
        tex_barriers.emplace_back(barrier);
    }
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

gsl::span<CGpuDescriptorSetId> RenderGraphBackend::alloc_update_pass_descsets(PassNode* pass)
{
    CGpuRootSignatureId root_sig = nullptr;
    auto read_edges = pass->read_edges();
    auto rw_edges = pass->readwrite_edges();
    if (pass->pass_type == EPassType::Render)
        root_sig = ((RenderPassNode*)pass)->pipeline->root_signature;
    else if (pass->pass_type == EPassType::Compute)
        root_sig = ((ComputePassNode*)pass)->pipeline->root_signature;
    auto&& desc_set_heap = desc_set_pool.find(root_sig);
    if (desc_set_heap == desc_set_pool.end())
        desc_set_pool.insert({ root_sig, new DescSetHeap(root_sig) });
    auto desc_sets = desc_set_pool[root_sig]->pop();
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
                auto texture_readed = rw_edge->get_texture_node();
                CGpuDescriptorData update = {};
                update.count = 1;
                update.binding = rw_set_binding.second;
                update.binding_type = RT_RW_TEXTURE;
                CGpuTextureViewDescriptor view_desc = {};
                view_desc.texture = resolve(*texture_readed);
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
    auto read_edges = pass->read_edges();
    auto write_edges = pass->write_edges();
    auto rw_edges = pass->readwrite_edges();
    for (auto& read_edge : read_edges)
    {
        auto texture_readed = read_edge->get_texture_node();
        if (texture_readed->imported)
            continue;
        bool is_last_user = true;
        texture_readed->foreach_neighbors(
            [&](DependencyGraphNode* neig) {
                RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                if (rg_node->type == EObjectType::Pass)
                {
                    PassNode* other_pass = (PassNode*)rg_node;
                    is_last_user = is_last_user && (pass->order >= other_pass->order);
                }
            });
        if (is_last_user)
            texture_pool.deallocate(texture_readed->descriptor,
                texture_readed->frame_texture,
                read_edge->requested_state,
                frame_index);
    }
    for (auto& write_edge : write_edges)
    {
        auto texture_target = write_edge->get_texture_node();
        if (texture_target->imported)
            continue;
        bool is_last_user = true;
        texture_target->foreach_neighbors(
            [&](DependencyGraphNode* neig) {
                RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                if (rg_node->type == EObjectType::Pass)
                {
                    PassNode* other_pass = (PassNode*)rg_node;
                    is_last_user = is_last_user && (pass->order >= other_pass->order);
                }
            });
        if (is_last_user)
            texture_pool.deallocate(texture_target->descriptor,
                texture_target->frame_texture,
                write_edge->requested_state,
                frame_index);
    }
    for (auto& rw_edge : rw_edges)
    {
        auto texture_target = rw_edge->get_texture_node();
        if (texture_target->imported)
            continue;
        bool is_last_user = true;
        texture_target->foreach_neighbors(
            [&](DependencyGraphNode* neig) {
                RenderGraphNode* rg_node = (RenderGraphNode*)neig;
                if (rg_node->type == EObjectType::Pass)
                {
                    PassNode* other_pass = (PassNode*)rg_node;
                    is_last_user = is_last_user && (pass->order >= other_pass->order);
                }
            });
        if (is_last_user)
            texture_pool.deallocate(texture_target->descriptor,
                texture_target->frame_texture,
                rw_edge->requested_state,
                frame_index);
    }
}

void RenderGraphBackend::execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass)
{
    ComputePassStack stack = {};
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    calculate_barriers(pass, tex_barriers);
    // allocate & update descriptor sets
    stack.desc_sets = alloc_update_pass_descsets(pass);
    // call cgpu apis
    if (!tex_barriers.empty())
    {
        CGpuResourceBarrierDescriptor barriers = {};
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
        cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    }
    // dispatch
    CGpuComputePassDescriptor pass_desc = {};
    pass_desc.name = pass->get_name();
    stack.encoder = cgpu_cmd_begin_compute_pass(executor.gfx_cmd_buf, &pass_desc);
    cgpu_compute_encoder_bind_pipeline(stack.encoder, pass->pipeline);
    for (auto desc_set : stack.desc_sets)
    {
        cgpu_compute_encoder_bind_descriptor_set(stack.encoder, desc_set);
    }
    pass->executor(*this, stack);
    cgpu_cmd_end_compute_pass(executor.gfx_cmd_buf, stack.encoder);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass)
{
    RenderPassStack stack = {};
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    calculate_barriers(pass, tex_barriers);
    // allocate & update descriptor sets
    stack.desc_sets = alloc_update_pass_descsets(pass);
    // call cgpu apis
    if (!tex_barriers.empty())
    {
        CGpuResourceBarrierDescriptor barriers = {};
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
        cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    }
    // color attachments
    // TODO: MSAA
    eastl::vector<CGpuColorAttachment> color_attachments = {};
    CGpuDepthStencilAttachment ds_attachment = {};
    auto write_edges = pass->write_edges();
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
    pass->executor(*this, stack);
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, stack.encoder);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass)
{
    // resource de-virtualize
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    calculate_barriers(pass, tex_barriers);
    // call cgpu apis
    if (!tex_barriers.empty())
    {
        CGpuResourceBarrierDescriptor barriers = {};
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
        cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    }
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
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass)
{
    auto read_edges = pass->read_edges();
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
    RenderGraphFrameExecutor& executor = executors[0];
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
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &executor.gfx_cmd_buf;
    submit_desc.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &submit_desc);
    graph->clear();
    blackboard.clear();
    for (auto pass : passes)
    {
        delete pass;
    }
    passes.clear();
    for (auto resource : resources)
    {
        delete resource;
    }
    resources.clear();
    // cleanup internal resources
    for (auto desc_heap : desc_set_pool)
    {
        desc_heap.second->reset();
    }
    return frame_index++;
}

CGpuDeviceId RenderGraphBackend::get_backend_device()
{
    return device;
}

void RenderGraphBackend::collect_grabage(uint64_t critical_frame)
{
    collect_texture_grabage(critical_frame);
    collect_buffer_grabage(critical_frame);
}

void RenderGraphBackend::collect_texture_grabage(uint64_t critical_frame)
{
    for (auto&& iter : texture_pool.textures)
    {
        auto&& queue = iter.second;
        for (auto&& element : queue)
        {
            if (element.second <= critical_frame)
            {
                cgpu_free_texture(element.first.first);
                element.first.first = nullptr;
            }
        }
        using ElementType = decltype(queue.front());
        queue.erase(
            eastl::remove_if(queue.begin(), queue.end(),
                [](ElementType& element) {
                    return element.first.first == nullptr;
                }),
            queue.end());
    }
}

void RenderGraphBackend::collect_buffer_grabage(uint64_t critical_frame)
{
}
} // namespace render_graph
} // namespace sakura