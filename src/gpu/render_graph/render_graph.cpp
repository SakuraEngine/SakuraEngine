#include "render_graph/backend/graph_backend.hpp"

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
                auto rg_to = static_cast<RenderGraphNode*>(to);
                if (rg_from->type == EObjectType::Texture)
                {
                    auto texture_from = static_cast<TextureNode*>(rg_from);
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
}

CGpuTextureId RenderGraphBackend::resolve(const TextureNode& node)
{
    return node.imported ? node.frame_texture : nullptr;
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass)
{
    auto read_edges = pass->read_edges();
    auto write_edges = pass->write_edges();
    // resource de-virtualize
    // init barriers
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    for (auto& read_edge : read_edges)
    {
        auto texture_readed = read_edge->get_texture_node();
        const auto current_state = get_lastest_state(texture_readed, pass);
        const auto dst_state = read_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = resolve(*texture_readed);
        tex_barriers.emplace_back(barrier);
    }
    for (auto& write_edge : write_edges)
    {
        auto texture_target = write_edge->get_texture_node();
        const auto current_state = get_lastest_state(texture_target, pass);
        const auto dst_state = write_edge->requested_state;
        if (current_state == dst_state) continue;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture = resolve(*texture_target);
        tex_barriers.emplace_back(barrier);
    }
    // color attachments
    eastl::vector<CGpuColorAttachment> color_attachments = {};
    for (auto& write_edge : write_edges)
    {
        CGpuColorAttachment attachment = {};
        auto texture_target = write_edge->get_texture_node();
        // TODO: MSAA
        CGpuTextureViewDescriptor view_desc = {};
        view_desc.texture = resolve(*texture_target);
        // TODO: add view_desc on resource edges
        view_desc.base_array_layer = 0;
        view_desc.array_layer_count = 1;
        view_desc.base_mip_level = 0;
        view_desc.mip_level_count = 1;
        view_desc.aspects = TVA_COLOR;
        view_desc.format = (ECGpuFormat)view_desc.texture->format;
        view_desc.usages = TVU_RTV_DSV;
        view_desc.dims = TEX_DIMENSION_2D;
        attachment.view = texture_view_pool.allocate(view_desc, frame_index);
        attachment.load_action = pass->load_actions[write_edge->mrt_index];
        attachment.store_action = pass->store_actions[write_edge->mrt_index];
        color_attachments.emplace_back(attachment);
    }
    // call cgpu apis
    if (!tex_barriers.empty())
    {
        CGpuResourceBarrierDescriptor barriers = {};
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = tex_barriers.size();
        cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    }
    // TODO: MSAA
    CGpuRenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count = write_edges.size();
    pass_desc.sample_count = SAMPLE_COUNT_1;
    pass_desc.name = pass->get_name();
    pass_desc.color_attachments = color_attachments.data();
    pass_desc.depth_stencil = nullptr;
    auto encoder = cgpu_cmd_begin_render_pass(executor.gfx_cmd_buf, &pass_desc);
    pass->executor(*this, encoder);
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, encoder);
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass)
{
    auto read_edges = pass->read_edges();
    auto&& read_edge = read_edges[0];
    auto texture_target = read_edge->get_texture_node();
    CGpuTextureBarrier present_barrier = {};
    present_barrier.texture = pass->descriptor.swapchain->back_buffers[pass->descriptor.index];
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
    return frame_index++;
}
} // namespace render_graph
} // namespace sakura